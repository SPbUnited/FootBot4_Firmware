/**
 * @file II2c.hpp
 * @brief Абстракция I2C транспорта
 *
 * Позволяет использовать библиотеку с любым I2C драйвером
 */

#ifndef OLED_II2C_HPP
#define OLED_II2C_HPP

#include <cstddef>
#include <cstdint>

namespace oled_lib
{

/**
 * @brief Интерфейс I2C транспорта
 *
 * Реализуйте этот интерфейс для своего I2C драйвера
 * или используйте готовый WireI2cAdapter для Arduino Wire
 */
struct II2c
{
    /**
     * @brief Записать данные по I2C
     * @param addr7 7-битный адрес устройства
     * @param data Указатель на данные
     * @param len Длина данных
     * @return true если успешно
     */
    virtual bool write(uint8_t addr7, const uint8_t* data, size_t len) = 0;

    /**
     * @brief Проверить наличие устройства на шине (ping)
     * @param addr7 7-битный адрес устройства
     * @return true если устройство отвечает (ACK)
     */
    virtual bool probe(uint8_t addr7) = 0;

    virtual ~II2c() = default;
};

}  // namespace oled_lib

#endif  // OLED_II2C_HPP
