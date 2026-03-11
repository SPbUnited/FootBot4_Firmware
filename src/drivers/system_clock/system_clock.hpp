#pragma once

#ifndef NATIVE

#include "kernel/kernel.hpp"
#include "stm32f4xx_hal.h"

namespace drivers::system_clock
{

class SystemClock
{
  private:
  public:
    SystemClock() {}

    void init()
    {
        RCC_OscInitTypeDef RCC_OscInitStruct = {0};
        RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

        // Включение внешнего кварца HSE (предполагаем 8 МГц)
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
        RCC_OscInitStruct.HSEState = RCC_HSE_ON;
        RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
        RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
        RCC_OscInitStruct.PLL.PLLM = 8;              // Делитель HSE (8 МГц / 8 = 1 МГц)
        RCC_OscInitStruct.PLL.PLLN = 360;            // Множитель VCO (1 МГц * 360 = 360 МГц)
        RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;  // Делитель PLLP (360 / 2 = 180 МГц)
        RCC_OscInitStruct.PLL.PLLQ = 7;              // Для USB/SDIO, не обязательно
        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
            kernel::error("SystemClock init error: HAL_RCC_OscConfig");

        // Настройка шин AHB, APB
        RCC_ClkInitStruct.ClockType =
            RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
        RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;  // SYSCLK = PLL (180 МГц)
        RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;         // HCLK = 180 МГц
        RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;          // APB1 = 45 МГц
        RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;          // APB2 = 90 МГц

        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
            kernel::error("SystemClock init error: HAL_RCC_ClockConfig");

        // Включение тактирования необходимых периферийных модулей
        __HAL_RCC_GPIOF_CLK_ENABLE();  // для PF0, PF1
        __HAL_RCC_I2C2_CLK_ENABLE();   // для I2C2
    }
};

}  // namespace drivers::system_clock

#else

namespace drivers::system_clock
{

class SystemClock
{
  private:
  public:
    SystemClock() {}

    void init() {}
};

}  // namespace drivers::system_clock

#endif
