#pragma once

#include "stm32f4xx_hal.h"

namespace drivers::gpio
{

struct GPIODescriptor
{
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
    uint32_t Alternate = -1;
};

struct ADCDescriptor
{
  ADC_TypeDef *instance;
  uint32_t resolution;
  uint32_t dataAlign;
  uint32_t scanConvMode;
  FunctionalState continuousConvMode;
  uint32_t externalTrigConv;
  uint32_t externalTrigConvEdge;

  uint32_t channel;
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

class GPIOAnalogInputDriver : public GPIODescriptor, public ADCDescriptor
{
  private:
  public:
    ADC_HandleTypeDef handle;
    GPIOAnalogInputDriver() {}

    void init(GPIODescriptor pin, ADCDescriptor adc);

    uint32_t read();
};

}  // namespace drivers::gpio
