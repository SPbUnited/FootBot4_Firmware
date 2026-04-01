#pragma once

#include <inttypes.h>

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
  public:
    CAN_HandleTypeDef handle;

    CanDriver(CanConfig config);

    void init();
    void write(uint32_t id, uint8_t *data, uint8_t len);
    void read(uint32_t id, uint8_t *data, uint8_t len);
};

}  // namespace drivers::can
