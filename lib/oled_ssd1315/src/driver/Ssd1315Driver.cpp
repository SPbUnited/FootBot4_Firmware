/**
 * @file Ssd1315Driver.cpp
 * @brief Реализация драйвера SSD1315
 */

#include "../../include/oled/domain/Ssd1315Driver.hpp"

#include <cstring>

#include "../../include/oled/adapters/PlatformDelay.hpp"

#if OLED_ENABLED

namespace oled_lib
{

OledResult Ssd1315Driver::init(II2c& i2c, const OledConfig& cfg)
{
    i2c_ = &i2c;
    cfg_ = cfg;
    initialized_ = false;

    // Проверка параметров
    if (cfg_.width == 0 || cfg_.width > 128)
    {
        return OledResult::InvalidArg;
    }
    if (cfg_.height != 32 && cfg_.height != 64)
    {
        return OledResult::InvalidArg;
    }

    // Аппаратный reset через callback или задержка для стабилизации
    hardwareResetSequence(cfg_.resetCallback);

    // === Последовательность инициализации SSD1315 ===

    // 1. Выключить дисплей
    if (!writeCommand(cmd::DISPLAY_OFF))
    {
        return OledResult::I2cError;
    }

    // 2. Настройка тактирования
    uint8_t clockCmd[] = {cmd::SET_CLOCK_DIV, cmd::DEFAULT_CLOCK_DIV};
    if (!writeCommands(clockCmd, sizeof(clockCmd)))
    {
        return OledResult::I2cError;
    }

    // 3. MUX Ratio (количество строк - 1)
    uint8_t muxCmd[] = {cmd::SET_MUX_RATIO, static_cast<uint8_t>(cfg_.height - 1)};
    if (!writeCommands(muxCmd, sizeof(muxCmd)))
    {
        return OledResult::I2cError;
    }

    // 4. Display offset
    uint8_t offsetCmd[] = {cmd::SET_DISPLAY_OFFSET, 0x00};
    if (!writeCommands(offsetCmd, sizeof(offsetCmd)))
    {
        return OledResult::I2cError;
    }

    // 5. Start line
    if (!writeCommand(cmd::SET_START_LINE | 0x00))
    {
        return OledResult::I2cError;
    }

    // 6. Charge Pump - зависит от VccMode
    uint8_t pumpValue = (cfg_.vccMode == VccMode::InternalChargePump) ? cmd::CHARGE_PUMP_ENABLE
                                                                      : cmd::CHARGE_PUMP_DISABLE;
    uint8_t pumpCmd[] = {cmd::SET_CHARGE_PUMP, pumpValue};
    if (!writeCommands(pumpCmd, sizeof(pumpCmd)))
    {
        return OledResult::I2cError;
    }

    // 7. Memory Addressing Mode - Horizontal для линейной заливки
    uint8_t memCmd[] = {cmd::SET_MEMORY_MODE, cmd::MEMORY_MODE_HORIZ};
    if (!writeCommands(memCmd, sizeof(memCmd)))
    {
        return OledResult::I2cError;
    }

    // 8. Segment remap и COM scan direction (для flip180)
    if (cfg_.flip180)
    {
        if (!writeCommand(cmd::SET_SEGMENT_REMAP_0))
            return OledResult::I2cError;
        if (!writeCommand(cmd::SET_COM_SCAN_INC))
            return OledResult::I2cError;
    }
    else
    {
        if (!writeCommand(cmd::SET_SEGMENT_REMAP_127))
            return OledResult::I2cError;
        if (!writeCommand(cmd::SET_COM_SCAN_DEC))
            return OledResult::I2cError;
    }

    // 9. COM Pins configuration
    uint8_t comPins = (cfg_.height == 64) ? cmd::COM_PINS_ALT_DISABLE : cmd::COM_PINS_SEQ_DISABLE;
    uint8_t comCmd[] = {cmd::SET_COM_PINS, comPins};
    if (!writeCommands(comCmd, sizeof(comCmd)))
    {
        return OledResult::I2cError;
    }

    // 10. Контраст по умолчанию
    uint8_t contrastCmd[] = {cmd::SET_CONTRAST, cmd::DEFAULT_CONTRAST};
    if (!writeCommands(contrastCmd, sizeof(contrastCmd)))
    {
        return OledResult::I2cError;
    }

    // 11. Precharge period - зависит от VccMode
    uint8_t precharge = (cfg_.vccMode == VccMode::InternalChargePump) ? cmd::DEFAULT_PRECHARGE
                                                                      : cmd::DEFAULT_PRECHARGE_EXT;
    uint8_t prechargeCmd[] = {cmd::SET_PRECHARGE, precharge};
    if (!writeCommands(prechargeCmd, sizeof(prechargeCmd)))
    {
        return OledResult::I2cError;
    }

    // 12. VCOM Deselect level
    uint8_t vcomCmd[] = {cmd::SET_VCOM_DESELECT, cmd::DEFAULT_VCOM};
    if (!writeCommands(vcomCmd, sizeof(vcomCmd)))
    {
        return OledResult::I2cError;
    }

    // 13. Отключить скроллинг
    if (!writeCommand(cmd::DEACTIVATE_SCROLL))
    {
        return OledResult::I2cError;
    }

    // 14. Вывод из RAM (не тестовый режим)
    if (!writeCommand(cmd::ENTIRE_DISPLAY_RAM))
    {
        return OledResult::I2cError;
    }

    // 15. Нормальный режим (не инверсия)
    if (!writeCommand(cmd::SET_NORMAL_DISPLAY))
    {
        return OledResult::I2cError;
    }

    // 16. Включить дисплей
    if (!writeCommand(cmd::DISPLAY_ON))
    {
        return OledResult::I2cError;
    }

    initialized_ = true;
    return OledResult::Ok;
}

OledResult Ssd1315Driver::setPower(bool on)
{
    if (!initialized_)
    {
        return OledResult::NotInitialized;
    }

    if (on)
    {
        // Включение: сначала charge pump (если internal), потом дисплей
        if (cfg_.vccMode == VccMode::InternalChargePump)
        {
            uint8_t pumpCmd[] = {cmd::SET_CHARGE_PUMP, cmd::CHARGE_PUMP_ENABLE};
            if (!writeCommands(pumpCmd, sizeof(pumpCmd)))
            {
                return OledResult::I2cError;
            }
        }
        if (!writeCommand(cmd::DISPLAY_ON))
        {
            return OledResult::I2cError;
        }
    }
    else
    {
        // Выключение: сначала дисплей, потом charge pump
        if (!writeCommand(cmd::DISPLAY_OFF))
        {
            return OledResult::I2cError;
        }
        if (cfg_.vccMode == VccMode::InternalChargePump)
        {
            uint8_t pumpCmd[] = {cmd::SET_CHARGE_PUMP, cmd::CHARGE_PUMP_DISABLE};
            if (!writeCommands(pumpCmd, sizeof(pumpCmd)))
            {
                return OledResult::I2cError;
            }
        }
    }

    return OledResult::Ok;
}

OledResult Ssd1315Driver::setContrast(uint8_t value)
{
    if (!initialized_)
    {
        return OledResult::NotInitialized;
    }

    uint8_t cmd[] = {cmd::SET_CONTRAST, value};
    return writeCommands(cmd, sizeof(cmd)) ? OledResult::Ok : OledResult::I2cError;
}

OledResult Ssd1315Driver::setInvert(bool on)
{
    if (!initialized_)
    {
        return OledResult::NotInitialized;
    }

    uint8_t c = on ? cmd::SET_INVERSE_DISPLAY : cmd::SET_NORMAL_DISPLAY;
    return writeCommand(c) ? OledResult::Ok : OledResult::I2cError;
}

OledResult Ssd1315Driver::writeBuffer(const uint8_t* buffer, size_t size)
{
    if (!initialized_)
    {
        return OledResult::NotInitialized;
    }

    if (buffer == nullptr)
    {
        return OledResult::InvalidArg;
    }

    size_t expectedSize = (cfg_.width * cfg_.height) / 8;
    if (size != expectedSize)
    {
        return OledResult::InvalidArg;
    }

    // Установить диапазон колонок (0 - width-1)
    uint8_t colCmd[] = {cmd::SET_COLUMN_ADDR, 0, static_cast<uint8_t>(cfg_.width - 1)};
    if (!writeCommands(colCmd, sizeof(colCmd)))
    {
        return OledResult::I2cError;
    }

    // Установить диапазон страниц (0 - pages-1)
    uint8_t pages = cfg_.height / 8;
    uint8_t pageCmd[] = {cmd::SET_PAGE_ADDR, 0, static_cast<uint8_t>(pages - 1)};
    if (!writeCommands(pageCmd, sizeof(pageCmd)))
    {
        return OledResult::I2cError;
    }

    // Отправить данные буфера
    if (!writeData(buffer, size))
    {
        return OledResult::I2cError;
    }

    return OledResult::Ok;
}

bool Ssd1315Driver::writeCommand(uint8_t c)
{
    uint8_t buf[2] = {cmd::CONTROL_COMMAND, c};
    return i2c_->write(cfg_.i2cAddr7, buf, 2);
}

bool Ssd1315Driver::writeCommands(const uint8_t* cmds, size_t len)
{
    // Отправляем все команды одним пакетом: control byte (0x00) + команды
    if (len == 0)
        return true;
    if (len > MAX_CMD_SIZE - 1)
        return false;  // Защита от переполнения буфера

    uint8_t buf[MAX_CMD_SIZE];
    buf[0] = cmd::CONTROL_COMMAND;  // 0x00 - режим команд
    memcpy(buf + 1, cmds, len);

    return i2c_->write(cfg_.i2cAddr7, buf, len + 1);
}

bool Ssd1315Driver::writeData(const uint8_t* data, size_t len)
{
    // Используем пакетный режим: один control byte (Co=0, D/C#=1) + много данных
    // Размер чанка зависит от платформы (см. OledConfig.hpp)

    constexpr size_t CHUNK_SIZE = OLED_I2C_CHUNK_SIZE;

    for (size_t offset = 0; offset < len; offset += CHUNK_SIZE)
    {
        size_t chunkLen = (len - offset > CHUNK_SIZE) ? CHUNK_SIZE : (len - offset);

        // Буфер: control byte + данные
        uint8_t buf[CHUNK_SIZE + 1];
        buf[0] = cmd::CONTROL_DATA;  // Co=0, D/C#=1 - режим данных
        memcpy(buf + 1, data + offset, chunkLen);

        if (!i2c_->write(cfg_.i2cAddr7, buf, chunkLen + 1))
        {
            return false;
        }
    }

    return true;
}

}  // namespace oled_lib

#endif  // OLED_ENABLED
