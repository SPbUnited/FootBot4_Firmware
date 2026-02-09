#pragma once

#include "stm32f4xx_hal.h"

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}