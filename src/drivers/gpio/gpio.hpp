#pragma once

#include "stm32f4xx_hal.h"

namespace drivers::gpio
{

struct GPIODescriptor
{
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
};

enum GPIO_OUT
{
    LED_STM32,
    LED_DRV1,
    LED_DRV2,
    LED_DRV3,
    LED_DRV4,
    LED_DRV5,
    LED_DATA_TRANSFER_STATUS_1,
    LED_DATA_TRANSFER_STATUS_2,
};

enum GPIO_INPUT
{
    BUTTON_ADDR_UP,
    BUTTON_ADDR_DOWN,
    BUTTON_SELECT,
};

class GPIOOutputDriver
{
  private:
  public:
    GPIOOutputDriver() {}

    void init();

    void write(GPIO_OUT pin, bool state);

    void toggle(GPIO_OUT pin);
};

class GPIOInputDriver
{
  private:
  public:
    GPIOInputDriver() {}

    void init();

    bool read(GPIO_INPUT pin);
};

}  // namespace drivers::gpio
