#include "can.hpp"

#include "drivers/system_clock/system_clock.hpp"

extern "C"
{
    volatile uint32_t can_monitor_rx_frames = 0;
    volatile uint32_t can_monitor_tx_frames = 0;
    volatile uint32_t can_monitor_rx_bits_estimate = 0;
    volatile uint32_t can_monitor_rx_rate_millihz = 0;
    volatile uint32_t can_monitor_tx_rate_millihz = 0;
    volatile uint32_t can_monitor_bus_load_rx_permille = 0;
    volatile uint32_t can_monitor_rx_interval_us = 0;
    volatile uint32_t can_monitor_rx_interval_max_us = 0;
    volatile uint32_t can_monitor_rx_queue_drops = 0;
    volatile uint32_t can_monitor_rx_fifo_overruns = 0;
    volatile uint32_t can_monitor_last_rx_id = 0;
    volatile uint32_t can_monitor_last_rx_dlc = 0;
    volatile uint32_t can_monitor_motor_register_frames[16] = {};
    volatile uint32_t can_monitor_motor_register_interval_us[16] = {};
    volatile uint32_t can_monitor_motor_register_ids[16] = {
        0x0E6, 0x0E7, 0x0E8, 0x0E9, 0x166, 0x167, 0x168, 0x169,
        0x1E6, 0x1E7, 0x1E8, 0x1E9, 0x266, 0x267, 0x268, 0x269};
}
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
    ++tx_frames_;
    can_monitor_tx_frames = tx_frames_;
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
        can_monitor_rx_queue_drops = rx_queue_drops_;
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

void CanDriver::recordRxFrame(const CAN_RxHeaderTypeDef &header)
{
    const uint32_t now = drivers::system_clock::micros();
    const uint32_t frame_bits = 47U + 8U * header.DLC;
    ++rx_frames_;
    rx_bits_estimate_ += frame_bits;
    can_monitor_rx_frames = rx_frames_;
    can_monitor_rx_bits_estimate = rx_bits_estimate_;
    can_monitor_last_rx_id = header.StdId;
    can_monitor_last_rx_dlc = header.DLC;
    if (last_rx_time_us_ != 0)
    {
        last_rx_interval_us_ = now - last_rx_time_us_;
        if (last_rx_interval_us_ > max_rx_interval_us_)
        {
            max_rx_interval_us_ = last_rx_interval_us_;
        }
    }
    last_rx_time_us_ = now;
    can_monitor_rx_interval_us = last_rx_interval_us_;
    can_monitor_rx_interval_max_us = max_rx_interval_us_;

    const uint8_t motor_addr = static_cast<uint8_t>((header.StdId >> 7) & 0x0FU);
    const uint8_t register_addr = static_cast<uint8_t>(header.StdId & 0x7FU);
    if (motor_addr >= 1 && motor_addr <= 4 && register_addr >= 102 && register_addr <= 105)
    {
        const uint8_t index = (motor_addr - 1U) * 4U + register_addr - 102U;
        ++monitored_frame_counts_[index];
        if (monitored_frame_last_times_us_[index] != 0)
        {
            monitored_frame_intervals_us_[index] = now - monitored_frame_last_times_us_[index];
        }
        monitored_frame_last_times_us_[index] = now;
        can_monitor_motor_register_frames[index] = monitored_frame_counts_[index];
        can_monitor_motor_register_interval_us[index] = monitored_frame_intervals_us_[index];
    }
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
        recordRxFrame(header);
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

uint32_t CanDriver::rxFrames() const
{
    return rx_frames_;
}
uint32_t CanDriver::txFrames() const
{
    return tx_frames_;
}
uint32_t CanDriver::rxBitsEstimate() const
{
    return rx_bits_estimate_;
}
uint32_t CanDriver::lastRxIntervalUs() const
{
    return last_rx_interval_us_;
}
uint32_t CanDriver::maxRxIntervalUs() const
{
    return max_rx_interval_us_;
}
void CanDriver::updateMonitorWindow()
{
    static uint32_t window_start_us = 0;
    static uint32_t window_rx_frames = 0;
    static uint32_t window_tx_frames = 0;
    static uint32_t window_rx_bits = 0;
    const uint32_t now = drivers::system_clock::micros();
    if (window_start_us == 0)
    {
        window_start_us = now;
        window_rx_frames = rx_frames_;
        window_tx_frames = tx_frames_;
        window_rx_bits = rx_bits_estimate_;
        return;
    }
    const uint32_t elapsed_us = now - window_start_us;
    if (elapsed_us < 1000000U)
    {
        return;
    }
    can_monitor_rx_queue_drops = rx_queue_drops_;
    can_monitor_rx_fifo_overruns = rx_fifo_overruns_;
    can_monitor_rx_rate_millihz =
        static_cast<uint32_t>((rx_frames_ - window_rx_frames) * 1000000000ULL / elapsed_us);
    can_monitor_tx_rate_millihz =
        static_cast<uint32_t>((tx_frames_ - window_tx_frames) * 1000000000ULL / elapsed_us);
    can_monitor_bus_load_rx_permille =
        static_cast<uint32_t>((rx_bits_estimate_ - window_rx_bits) * 1000ULL / elapsed_us);
    window_start_us = now;
    window_rx_frames = rx_frames_;
    window_tx_frames = tx_frames_;
    window_rx_bits = rx_bits_estimate_;
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
        can_monitor_rx_fifo_overruns = can1_driver->rx_fifo_overruns_;
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
