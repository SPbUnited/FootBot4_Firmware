#include "can.hpp"

#include "drivers/driver_manager.hpp"
#include "kernel/kernel.hpp"

namespace drivers::can
{

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

    HAL_StatusTypeDef status = HAL_CAN_Init(&handle);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_CAN_GetError(&handle);
        kerror("CAN init error: %d, error code: %lu\n", int(status), error);
    }

    // Start the CAN peripheral
    status = HAL_CAN_Start(&handle);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_CAN_GetError(&handle);
        kerror("CAN start error: %d, error code: %lu\n", int(status), error);
    }

    // Configure and activate a filter to allow all messages (standard ID)
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
}

void CanDriver::write(uint32_t id, uint8_t *data, uint8_t len)
{
    CAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[4];
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
    }
}

void CanDriver::read(uint32_t id, uint8_t *data, uint8_t len)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t RxData[8];

    HAL_StatusTypeDef status = HAL_CAN_GetRxMessage(&handle, CAN_RX_FIFO0, &RxHeader, RxData);
    if (status == HAL_OK)
    {
        id = RxHeader.StdId;
        len = RxHeader.DLC;
        for (uint8_t i = 0; i < len; i++)
        {
            data[i] = RxData[i];
        }
    }
    else
    {
        kerror("CAN read error: %d\n", int(status));
    }
}

}  // namespace drivers::can
