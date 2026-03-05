/**
 * @file Stm32HalI2cAdapter.hpp
 * @brief STM32 HAL I2C адаптер для библиотеки OLED SSD1315
 *
 * Использование:
 * @code
 * I2C_HandleTypeDef hi2c1;
 * Stm32HalI2cAdapter adapter;
 * adapter.init(&hi2c1);
 * @endcode
 */

#ifndef OLED_STM32HAL_I2C_ADAPTER_HPP
#define OLED_STM32HAL_I2C_ADAPTER_HPP

#include "../OledConfig.hpp"
#include "../ports/II2c.hpp"

#if OLED_USE_STM32HAL

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

namespace oled_lib
{

/**
 * @brief Адаптер STM32 HAL I2C для интерфейса II2c
 */
class Stm32HalI2cAdapter : public II2c
{
  public:
    /**
     * @brief Конструктор по умолчанию
     */
    Stm32HalI2cAdapter() : hi2c_(nullptr), timeout_(100) {}

    /**
     * @brief Инициализировать адаптер
     * @param hi2c Указатель на I2C_HandleTypeDef
     * @param timeout Таймаут I2C операций в мс (по умолчанию 100)
     */
    void init(I2C_HandleTypeDef* hi2c, uint32_t timeout = 100)
    {
        hi2c_ = hi2c;
        timeout_ = timeout;
    }

    /**
     * @brief Проверить инициализацию
     */
    bool isInitialized() const
    {
        return hi2c_ != nullptr;
    }

    /**
     * @brief Отправить данные по I2C
     * @param addr7 7-битный адрес устройства
     * @param data Указатель на данные
     * @param len Длина данных
     * @return true при успехе
     */
    bool write(uint8_t addr7, const uint8_t* data, size_t len) override
    {
        if (!hi2c_ || !data || len == 0)
        {
            return false;
        }

        // HAL требует 8-битный адрес (7-бит << 1)
        uint16_t addr8 = static_cast<uint16_t>(addr7) << 1;

        HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
            hi2c_, addr8, const_cast<uint8_t*>(data), static_cast<uint16_t>(len), timeout_);

        return (status == HAL_OK);
    }

    /**
     * @brief Проверить наличие устройства на шине
     * @param addr7 7-битный адрес устройства
     * @return true если устройство отвечает (ACK)
     */
    bool probe(uint8_t addr7) override
    {
        if (!hi2c_)
            return false;

        uint16_t addr8 = static_cast<uint16_t>(addr7) << 1;

        // HAL_I2C_IsDeviceReady проверяет наличие устройства
        HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(hi2c_, addr8,
                                                         1,  // Trials
                                                         timeout_);

        return (status == HAL_OK);
    }

  private:
    I2C_HandleTypeDef* hi2c_;
    uint32_t timeout_;
};

}  // namespace oled_lib

#endif  // OLED_USE_STM32HAL

#endif  // OLED_STM32HAL_I2C_ADAPTER_HPP
