#pragma once

#include "stm32f4xx_hal.h"

namespace drivers::gpio
{

struct GPIODescriptor
{
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
};

class GPIOOutputDriver : public GPIODescriptor
{
  private:
  public:
    GPIOOutputDriver() {}

    void init(GPIODescriptor pin);

    void write(bool state);

    void toggle();
};

class GPIOInputDriver : public GPIODescriptor
{
  private:
  public:
    GPIOInputDriver() {}

    void init(GPIODescriptor pin);

    bool read();
};

}  // namespace drivers::gpio
