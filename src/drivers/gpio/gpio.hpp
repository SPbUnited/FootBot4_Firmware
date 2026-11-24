#pragma once

#include "stm32f4xx_hal.h"

namespace drivers::gpio
{

struct GPIODescriptor
{
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
    uint32_t Alternate = -1;
};

struct ADCDescriptor
{
    ADC_TypeDef* instance;
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
    uint8_t pwm_counter = 0;

  public:
    GPIOOutputDriver() {}

    void init(GPIODescriptor pin);

    void write(bool state);

    void toggle();

    // Advance one of the 100 PWM steps. Call at regular intervals.
    void soft_pwm(uint8_t threshold, bool reverse);
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
    ADC_HandleTypeDef handle;
    DMA_HandleTypeDef hdma_adc;
    uint32_t* adcDataBuffer;
    uint32_t bufferSize;

  public:
    GPIOAnalogInputDriver() {}

    /** Configures ADC, channel, and DMA; returns false on HAL setup failure. */
    bool init(GPIODescriptor pin, ADCDescriptor adc, uint32_t* buffer = nullptr,
              uint32_t size = 1);

    uint32_t read();

    uint32_t* getBuffer() const
    {
        return adcDataBuffer;
    }

    uint32_t getBufferSize() const
    {
        return bufferSize;
    }

    void setBuffer(uint32_t* buffer)
    {
        adcDataBuffer = buffer;
    }

    void setBufferSize(uint32_t size)
    {
        bufferSize = size;
    }

    void startDMA();

    void stopDMA();
};

}  // namespace drivers::gpio
