/**
 * @file Ssd1315Driver.hpp
 * @brief Низкоуровневый драйвер контроллера SSD1315
 */

#ifndef OLED_SSD1315_DRIVER_HPP
#define OLED_SSD1315_DRIVER_HPP

#include "../OledConfig.hpp"
#include "../OledTypes.hpp"
#include "../ports/II2c.hpp"
#include "Ssd1315Commands.hpp"

#if OLED_ENABLED

namespace oled_lib
{

/**
 * @brief Драйвер SSD1315
 *
 * Управляет инициализацией, командами и передачей данных в GDDRAM.
 * Использует Horizontal Addressing Mode для линейной заливки буфера.
 */
class Ssd1315Driver
{
  public:
    // Максимальный размер команды (control byte + данные)
    static constexpr size_t MAX_CMD_SIZE = 8;

    /**
     * @brief Конструктор по умолчанию (для статического размещения)
     */
    Ssd1315Driver() : i2c_(nullptr) {}

    /**
     * @brief Инициализировать драйвер с I2C транспортом и конфигурацией
     * @param i2c Ссылка на I2C транспорт
     * @param cfg Конфигурация дисплея
     * @return Результат операции
     */
    OledResult init(II2c& i2c, const OledConfig& cfg);

    /**
     * @brief Включить/выключить дисплей
     * @param on true - включить, false - выключить (sleep)
     * @return Результат операции
     */
    OledResult setPower(bool on);

    /**
     * @brief Установить контраст
     * @param value Уровень контраста 0-255
     * @return Результат операции
     */
    OledResult setContrast(uint8_t value);

    /**
     * @brief Включить/выключить инверсию
     * @param on true - инверсия включена
     * @return Результат операции
     */
    OledResult setInvert(bool on);

    /**
     * @brief Записать буфер в GDDRAM
     * @param buffer Указатель на буфер (width * height / 8 байт)
     * @param size Размер буфера
     * @return Результат операции
     */
    OledResult writeBuffer(const uint8_t* buffer, size_t size);

    /**
     * @brief Проверить готовность драйвера
     */
    bool isReady() const
    {
        return initialized_;
    }

    /**
     * @brief Получить текущую конфигурацию
     */
    const OledConfig& config() const
    {
        return cfg_;
    }

  private:
    /**
     * @brief Отправить команду (control byte D/C#=0)
     * @param cmd Байт команды
     * @return true если успешно
     */
    bool writeCommand(uint8_t cmd);

    /**
     * @brief Отправить команду с параметрами
     * @param cmds Массив команд
     * @param len Длина массива
     * @return true если успешно
     */
    bool writeCommands(const uint8_t* cmds, size_t len);

    /**
     * @brief Отправить данные в GDDRAM (control byte D/C#=1)
     * @param data Указатель на данные
     * @param len Длина данных
     * @return true если успешно
     */
    bool writeData(const uint8_t* data, size_t len);

    II2c* i2c_;
    OledConfig cfg_;
    bool initialized_ = false;
};

}  // namespace oled_lib

#else  // OLED_ENABLED == 0

namespace oled
{

// Заглушка
class Ssd1315Driver
{
  public:
    Ssd1315Driver() {}
    template <typename T1, typename T2>
    OledResult init(T1&, const T2&)
    {
        return OledResult::Disabled;
    }
    OledResult setPower(bool)
    {
        return OledResult::Disabled;
    }
    OledResult setContrast(uint8_t)
    {
        return OledResult::Disabled;
    }
    OledResult setInvert(bool)
    {
        return OledResult::Disabled;
    }
    OledResult writeBuffer(const uint8_t*, size_t)
    {
        return OledResult::Disabled;
    }
    bool isReady() const
    {
        return false;
    }
};

}  // namespace oled

#endif  // OLED_ENABLED

#endif  // OLED_SSD1315_DRIVER_HPP
