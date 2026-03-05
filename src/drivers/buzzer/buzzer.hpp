#pragma once

#include "stm32f4xx_hal.h"

namespace drivers::buzzer
{

class Buzzer
{
  private:
  public:
    Buzzer() {}

    void init()
    {
        __HAL_RCC_GPIOF_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_13;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    }

    void buzz()
    {
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
        HAL_Delay(1000);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
    }
};

}  // namespace drivers::buzzer
