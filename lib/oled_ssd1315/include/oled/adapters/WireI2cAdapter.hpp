/**
 * @file WireI2cAdapter.hpp
 * @brief Адаптер Arduino Wire для II2c интерфейса
 */

#ifndef OLED_WIRE_I2C_ADAPTER_HPP
#define OLED_WIRE_I2C_ADAPTER_HPP

#include "../OledConfig.hpp"
#include "../ports/II2c.hpp"

#if OLED_ENABLED && OLED_USE_ARDUINO

#include <Wire.h>

namespace oled
{

/**
 * @brief Адаптер Arduino Wire библиотеки
 *
 * Использует TwoWire для отправки данных по I2C.
 * Поддерживает пакетную передачу с учётом ограничения буфера Wire.
 */
class WireI2cAdapter : public II2c
{
  public:
    // Стандартный размер буфера Wire (может отличаться на разных платформах)
    static constexpr size_t WIRE_BUFFER_SIZE = 32;

    /**
     * @brief Конструктор по умолчанию (для статического размещения)
     */
    WireI2cAdapter() : wire_(nullptr) {}

    /**
     * @brief Инициализация с Wire
     * @param wire Ссылка на TwoWire (обычно Wire)
     */
    void init(TwoWire& wire)
    {
        wire_ = &wire;
    }

    /**
     * @brief Проверка инициализации
     */
    bool isInitialized() const
    {
        return wire_ != nullptr;
    }

    /**
     * @brief Записать данные по I2C
     *
     * Автоматически разбивает большие пакеты на части
     * с учётом ограничения буфера Wire.
     *
     * @param addr7 7-битный адрес устройства
     * @param data Указатель на данные
     * @param len Длина данных
     * @return true если успешно
     */
    bool write(uint8_t addr7, const uint8_t* data, size_t len) override;

    /**
     * @brief Проверить наличие устройства на шине
     * @param addr7 7-битный адрес устройства
     * @return true если устройство отвечает (ACK)
     */
    bool probe(uint8_t addr7) override;

  private:
    TwoWire* wire_;
};

}  // namespace oled

#else  // !OLED_ENABLED || !OLED_USE_ARDUINO

namespace oled_lib
{

// Заглушка когда библиотека отключена
class WireI2cAdapter : public II2c
{
  public:
    WireI2cAdapter() {}
    template <typename T>
    void init(T&)
    {
    }
    bool isInitialized() const
    {
        return false;
    }
    bool write(uint8_t, const uint8_t*, size_t) override
    {
        return false;
    }
    bool probe(uint8_t) override
    {
        return false;
    }
};

}  // namespace oled_lib

#endif  // OLED_ENABLED && OLED_USE_ARDUINO

#endif  // OLED_WIRE_I2C_ADAPTER_HPP
