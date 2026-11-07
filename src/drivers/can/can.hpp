#pragma once

#include <inttypes.h>
#include <stddef.h>

#include "stm32f4xx_hal.h"

namespace drivers::can
{

struct CanConfig
{
    CAN_TypeDef *instance;
    //
    void (*clk_enable)();
    //
    uint32_t txPin;
    GPIO_TypeDef *txPort;
    uint32_t txAlternate;
    uint32_t rxPin;
    GPIO_TypeDef *rxPort;
    uint32_t rxAlternate;
    //
    uint32_t prescaler;
    uint32_t mode;
    uint32_t sjw;
    uint32_t timeSeg1;
    uint32_t timeSeg2;
    FunctionalState timeTriggeredMode;
    FunctionalState autoBusOff;
    FunctionalState autoWakeUp;
    FunctionalState autoRetrans;
    FunctionalState receiveFifoLocked;
    FunctionalState transmitFifoPriority;
};

class CanDriver : public CanConfig
{
  private:
    struct RxFrame
    {
        uint32_t id;
        uint8_t data[8];
        uint8_t len;
    };

    static constexpr uint8_t RX_QUEUE_SIZE = 32;
    static constexpr uint8_t RX_QUEUE_MASK = RX_QUEUE_SIZE - 1;
    static constexpr uint8_t RX_FIFO0_DEPTH = 3;

    RxFrame rx_queue_[RX_QUEUE_SIZE] = {};
    volatile uint8_t rx_head_ = 0;
    volatile uint8_t rx_tail_ = 0;
    volatile uint32_t rx_queue_drops_ = 0;
    volatile uint32_t rx_fifo_overruns_ = 0;

    friend void handleCan1Rx0Pending(CAN_HandleTypeDef *handle);
    friend void handleCanError(CAN_HandleTypeDef *handle);

    void receivePending();
    bool enqueueFromIsr(const CAN_RxHeaderTypeDef &header, const uint8_t *data);

  public:
    CAN_HandleTypeDef handle;

    CanDriver(CanConfig config);

    void init();
    bool write(uint32_t id, const uint8_t *data, uint8_t len);
    /** Read one frame copied from the RX interrupt queue. */
    bool read(uint32_t *id, uint8_t *data, uint8_t *len);
    /** Return frames dropped because the software queue was full. */
    uint32_t rxQueueDrops() const;
    /** Return hardware RX FIFO overrun events. */
    uint32_t rxFifoOverruns() const;
};

void handleCan1Rx0Interrupt();
void handleCan1Rx0Pending(CAN_HandleTypeDef *handle);
void handleCanError(CAN_HandleTypeDef *handle);

}  // namespace drivers::can
