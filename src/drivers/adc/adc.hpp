#pragma once

#include <inttypes.h>

#include "stm32f4xx_hal.h"

namespace drivers::adc
{

struct ADCConfig
{
    ADC_TypeDef *instance;
    //
    void (*clk_enable)();
    //
    uint32_t channel;
    GPIO_TypeDef *port;
    uint32_t pin;
    //
    uint32_t resolution;
    uint32_t dataAlign;
    uint32_t scanConvMode;
    FunctionalState continuousConvMode;
    uint32_t externalTrigConv;
    uint32_t externalTrigConvEdge;
};

class ADCDriver : public ADCConfig
{
  private:
  public:
    ADC_HandleTypeDef handle;

    ADCDriver(ADCConfig config);

    void init();
    uint32_t read();
    void readMulti(uint32_t *data, uint8_t len);
};

}  // namespace drivers::adc
