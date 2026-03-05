/**
 * @file PlatformDelay.hpp
 * @brief Platform-agnostic delay и GPIO функции
 *
 * Абстрагирует delay(), pinMode(), digitalWrite() для разных платформ.
 */

#ifndef OLED_PLATFORM_DELAY_HPP
#define OLED_PLATFORM_DELAY_HPP

#include <cstdint>

#include "../OledConfig.hpp"
#include "../OledTypes.hpp"

#if OLED_USE_ARDUINO
#include <Arduino.h>
#elif OLED_USE_STM32HAL
#if defined(STM32H7)
#include "stm32h7xx_hal.h"
#elif defined(STM32F4)
#include "stm32f4xx_hal.h"
#elif defined(STM32F1)
#include "stm32f1xx_hal.h"
#elif defined(STM32F0)
#include "stm32f0xx_hal.h"
#elif defined(STM32F3)
#include "stm32f3xx_hal.h"
#elif defined(STM32L4)
#include "stm32l4xx_hal.h"
#elif defined(STM32G0)
#include "stm32g0xx_hal.h"
#elif defined(STM32G4)
#include "stm32g4xx_hal.h"
#else
#include "stm32_hal_legacy.h"
#endif
#elif OLED_USE_ESPIDF
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

namespace oled_lib
{

/**
 * @brief Platform-agnostic delay в миллисекундах
 * @param ms Задержка в мс
 */
inline void platformDelay(uint32_t ms)
{
#if OLED_USE_ARDUINO
    delay(ms);
#elif OLED_USE_STM32HAL
    HAL_Delay(ms);
#elif OLED_USE_ESPIDF
    vTaskDelay(pdMS_TO_TICKS(ms));
#else
    // Busy-wait fallback (очень приблизительно)
    volatile uint32_t count = ms * 10000;
    while (count--)
    {
        __asm__ volatile("nop");
    }
#endif
}

/**
 * @brief Выполнить hardware reset через callback
 * @param callback Функция управления GPIO
 *
 * Если callback == nullptr, просто ждёт 20мс для стабилизации.
 */
inline void hardwareResetSequence(ResetGpioCallback callback)
{
    if (callback)
    {
        // Reset sequence: HIGH -> LOW -> HIGH
        callback(true);
        platformDelay(1);
        callback(false);
        platformDelay(10);
        callback(true);
        platformDelay(10);
    }
    else
    {
        // Без reset - задержка для стабилизации
        platformDelay(20);
    }
}

}  // namespace oled_lib

#endif  // OLED_PLATFORM_DELAY_HPP
