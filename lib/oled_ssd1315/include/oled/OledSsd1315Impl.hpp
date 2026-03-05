/**
 * @file OledSsd1315Impl.hpp
 * @brief Детали реализации OledSsd1315 (не включать в пользовательский код)
 *
 * Этот заголовок содержит платформенно-зависимые детали реализации.
 * Пользователи библиотеки должны включать только OledSsd1315.hpp
 */

#ifndef OLED_SSD1315_IMPL_HPP
#define OLED_SSD1315_IMPL_HPP

#include "OledConfig.hpp"
#include "OledTypes.hpp"
#include "domain/Gfx.hpp"
#include "domain/Ssd1315Driver.hpp"

#if OLED_ENABLED
#if OLED_USE_ARDUINO
#include <Wire.h>

#include "adapters/WireI2cAdapter.hpp"
#elif OLED_USE_STM32HAL
#include "adapters/Stm32HalI2cAdapter.hpp"
#endif
#endif

namespace oled_lib
{
namespace detail
{

/**
 * @brief Внутренняя реализация OledSsd1315
 *
 * Содержит все платформенно-зависимые поля.
 * Скрыта от публичного API через pImpl idiom.
 */
struct OledSsd1315Impl
{
#if OLED_ENABLED
#if OLED_USE_ARDUINO
    TwoWire* wire = nullptr;
    WireI2cAdapter adapter;
#elif OLED_USE_STM32HAL
    I2C_HandleTypeDef* hi2c = nullptr;
    Stm32HalI2cAdapter adapter;
#endif

    Ssd1315Driver driver;
    Gfx gfx;
    uint8_t buffer[OLED_MAX_BUFFER_SIZE] = {0};
    bool initialized = false;
    OledResult lastResult = OledResult::Ok;
    const char* lastErrorMsg = nullptr;

#if OLED_USE_STM32HAL
    volatile bool dmaInProgress = false;
#endif
#endif
};

}  // namespace detail
}  // namespace oled_lib

#endif  // OLED_SSD1315_IMPL_HPP
