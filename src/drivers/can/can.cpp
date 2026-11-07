#include "can.hpp"

#include "kernel/kernel.hpp"

namespace drivers::can
{

namespace
{
CanDriver *can1_driver = nullptr;
}

CanDriver::CanDriver(CanConfig config) : CanConfig(config) {}

void CanDriver::init()
{
    clk_enable();

    GPIO_InitTypeDef GPIO_InitStruct;

    // TX pin configuration
    GPIO_InitStruct.Pin = txPin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = txAlternate;

    HAL_GPIO_Init(txPort, &GPIO_InitStruct);

    // RX pin configuration
    GPIO_InitStruct.Pin = rxPin;
    GPIO_InitStruct.Alternate = rxAlternate;

    HAL_GPIO_Init(rxPort, &GPIO_InitStruct);

    handle.Instance = instance;

    handle.Init.Prescaler = prescaler;
    handle.Init.Mode = mode;
    handle.Init.SyncJumpWidth = sjw;
    handle.Init.TimeSeg1 = timeSeg1;
    handle.Init.TimeSeg2 = timeSeg2;
    handle.Init.TimeTriggeredMode = timeTriggeredMode;
    handle.Init.AutoBusOff = autoBusOff;
    handle.Init.AutoWakeUp = autoWakeUp;
    handle.Init.AutoRetransmission = autoRetrans;
    handle.Init.ReceiveFifoLocked = receiveFifoLocked;
    handle.Init.TransmitFifoPriority = transmitFifoPriority;

    can1_driver = this;

    HAL_StatusTypeDef status = HAL_CAN_Init(&handle);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_CAN_GetError(&handle);
        kerror("CAN init error: %d, error code: %lu\n", int(status), error);
    }

    // Configure a filter to allow all messages (standard ID)
    CAN_FilterTypeDef sFilterConfig;
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    status = HAL_CAN_ConfigFilter(&handle, &sFilterConfig);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_CAN_GetError(&handle);
        kerror("CAN filter config error: %d, error code: %lu\n", int(status), error);
    }

    status = HAL_CAN_Start(&handle);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_CAN_GetError(&handle);
        kerror("CAN start error: %d, error code: %lu\n", int(status), error);
        return;
    }

    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);

    status = HAL_CAN_ActivateNotification(&handle,
                                          CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_RX_FIFO0_OVERRUN);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_CAN_GetError(&handle);
        kerror("CAN RX notification error: %d, error code: %lu\n", int(status), error);
    }
}

bool CanDriver::write(uint32_t id, const uint8_t *data, uint8_t len)
{
    if (data == nullptr || len > 8 || id > 0x7FF)
    {
        return false;
    }

    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8];
    uint32_t TxMailbox;

    TxHeader.StdId = id;
    TxHeader.ExtId = 0;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = len;
    TxHeader.TransmitGlobalTime = DISABLE;

    for (uint8_t i = 0; i < len; i++)
    {
        TxData[i] = data[i];
    }

    // Try to add message to available mailbox
    uint32_t timeout = 10000;
    HAL_StatusTypeDef status = HAL_BUSY;

    while (status == HAL_BUSY && timeout-- > 0)
    {
        status = HAL_CAN_AddTxMessage(&handle, &TxHeader, TxData, &TxMailbox);
        HAL_Delay(1);
    }

    if (status != HAL_OK)
    {
        uint32_t error = HAL_CAN_GetError(&handle);
        kerror("CAN write error: %d, error code: %lu\n", int(status), error);
        return false;
    }
    return true;
}

bool CanDriver::read(uint32_t *id, uint8_t *data, uint8_t *len)
{
    if (id == nullptr || data == nullptr || len == nullptr)
    {
        return false;
    }

    const uint8_t tail = rx_tail_;
    if (tail == rx_head_)
    {
        return false;
    }

    __DMB();
    const RxFrame &frame = rx_queue_[tail];
    *id = frame.id;
    *len = frame.len;
    for (uint8_t i = 0; i < frame.len; ++i)
    {
        data[i] = frame.data[i];
    }

    __DMB();
    rx_tail_ = (tail + 1) & RX_QUEUE_MASK;
    return true;
}

bool CanDriver::enqueueFromIsr(const CAN_RxHeaderTypeDef &header, const uint8_t *data)
{
    if (header.IDE != CAN_ID_STD || header.RTR != CAN_RTR_DATA || header.DLC > 8)
    {
        return false;
    }

    const uint8_t head = rx_head_;
    const uint8_t next = (head + 1) & RX_QUEUE_MASK;
    if (next == rx_tail_)
    {
        ++rx_queue_drops_;
        return false;
    }

    RxFrame &frame = rx_queue_[head];
    frame.id = header.StdId;
    frame.len = static_cast<uint8_t>(header.DLC);
    for (uint8_t i = 0; i < frame.len; ++i)
    {
        frame.data[i] = data[i];
    }

    __DMB();
    rx_head_ = next;
    return true;
}

void CanDriver::receivePending()
{
    for (uint8_t frames_read = 0;
         frames_read < RX_FIFO0_DEPTH && HAL_CAN_GetRxFifoFillLevel(&handle, CAN_RX_FIFO0) > 0;
         ++frames_read)
    {
        CAN_RxHeaderTypeDef header;
        uint8_t data[8];
        if (HAL_CAN_GetRxMessage(&handle, CAN_RX_FIFO0, &header, data) != HAL_OK)
        {
            break;
        }
        enqueueFromIsr(header, data);
    }
}

uint32_t CanDriver::rxQueueDrops() const
{
    return rx_queue_drops_;
}

uint32_t CanDriver::rxFifoOverruns() const
{
    return rx_fifo_overruns_;
}

void handleCan1Rx0Interrupt()
{
    if (can1_driver != nullptr)
    {
        HAL_CAN_IRQHandler(&can1_driver->handle);
    }
}

void handleCan1Rx0Pending(CAN_HandleTypeDef *handle)
{
    if (can1_driver != nullptr && handle == &can1_driver->handle)
    {
        can1_driver->receivePending();
    }
}

void handleCanError(CAN_HandleTypeDef *handle)
{
    if (can1_driver != nullptr && handle == &can1_driver->handle &&
        (handle->ErrorCode & HAL_CAN_ERROR_RX_FOV0) != 0)
    {
        ++can1_driver->rx_fifo_overruns_;
        handle->ErrorCode &= ~HAL_CAN_ERROR_RX_FOV0;
    }
}

}  // namespace drivers::can

extern "C" void CAN1_RX0_IRQHandler(void)
{
    drivers::can::handleCan1Rx0Interrupt();
}

extern "C" void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *handle)
{
    drivers::can::handleCan1Rx0Pending(handle);
}

extern "C" void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *handle)
{
    drivers::can::handleCanError(handle);
}
