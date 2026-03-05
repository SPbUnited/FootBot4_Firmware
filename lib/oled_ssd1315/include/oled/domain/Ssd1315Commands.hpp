/**
 * @file Ssd1315Commands.hpp
 * @brief Константы команд контроллера SSD1315
 *
 * Все "магические числа" вынесены в constexpr.
 * Документация: SSD1315 Datasheet
 */

#ifndef OLED_SSD1315_COMMANDS_HPP
#define OLED_SSD1315_COMMANDS_HPP

#include <cstdint>

namespace oled_lib
{
namespace cmd
{

// === Control Byte ===
// D/C# бит определяет: команда (0) или данные (1)
// Co бит: 0 = только данные далее, 1 = каждый байт с control byte
constexpr uint8_t CONTROL_COMMAND = 0x00;       // Co=0, D/C#=0: команда
constexpr uint8_t CONTROL_COMMAND_CONT = 0x80;  // Co=1, D/C#=0: команда, продолжение
constexpr uint8_t CONTROL_DATA = 0x40;          // Co=0, D/C#=1: данные (пакетный режим)
constexpr uint8_t CONTROL_DATA_CONT = 0xC0;     // Co=1, D/C#=1: данные, продолжение

// === Fundamental Commands ===
constexpr uint8_t SET_CONTRAST = 0x81;         // +1 байт: уровень 0x00-0xFF
constexpr uint8_t ENTIRE_DISPLAY_RAM = 0xA4;   // Вывод из RAM
constexpr uint8_t ENTIRE_DISPLAY_ON = 0xA5;    // Все пиксели включены (тест)
constexpr uint8_t SET_NORMAL_DISPLAY = 0xA6;   // Нормальный режим
constexpr uint8_t SET_INVERSE_DISPLAY = 0xA7;  // Инверсия
constexpr uint8_t DISPLAY_OFF = 0xAE;          // Выключить дисплей (sleep)
constexpr uint8_t DISPLAY_ON = 0xAF;           // Включить дисплей

// === Addressing Setting Commands ===
constexpr uint8_t SET_MEMORY_MODE = 0x20;    // +1 байт: режим адресации
constexpr uint8_t MEMORY_MODE_HORIZ = 0x00;  // Horizontal Addressing Mode
constexpr uint8_t MEMORY_MODE_VERT = 0x01;   // Vertical Addressing Mode
constexpr uint8_t MEMORY_MODE_PAGE = 0x02;   // Page Addressing Mode (default)

constexpr uint8_t SET_COLUMN_ADDR = 0x21;  // +2 байта: start, end (0-127)
constexpr uint8_t SET_PAGE_ADDR = 0x22;    // +2 байта: start, end (0-7 для 64px)

// Page Addressing Mode команды (если используется PAGE mode)
constexpr uint8_t SET_PAGE_START_ADDR = 0xB0;  // | с номером страницы 0-7
constexpr uint8_t SET_LOW_COLUMN = 0x00;       // | с младшими 4 битами колонки
constexpr uint8_t SET_HIGH_COLUMN = 0x10;      // | со старшими 4 битами колонки

// === Hardware Configuration ===
constexpr uint8_t SET_START_LINE = 0x40;         // | с номером линии 0-63
constexpr uint8_t SET_SEGMENT_REMAP_0 = 0xA0;    // Колонка 0 -> SEG0
constexpr uint8_t SET_SEGMENT_REMAP_127 = 0xA1;  // Колонка 127 -> SEG0 (flip X)
constexpr uint8_t SET_MUX_RATIO = 0xA8;          // +1 байт: N-1 (15-63)
constexpr uint8_t SET_COM_SCAN_INC = 0xC0;       // COM0 -> COM[N-1]
constexpr uint8_t SET_COM_SCAN_DEC = 0xC8;       // COM[N-1] -> COM0 (flip Y)
constexpr uint8_t SET_DISPLAY_OFFSET = 0xD3;     // +1 байт: смещение 0-63
constexpr uint8_t SET_COM_PINS = 0xDA;           // +1 байт: конфигурация COM

// COM Pins Config значения
constexpr uint8_t COM_PINS_SEQ_DISABLE = 0x02;  // Sequential, Disable remap
constexpr uint8_t COM_PINS_ALT_DISABLE = 0x12;  // Alternative (128x64), Disable remap
constexpr uint8_t COM_PINS_SEQ_ENABLE = 0x22;   // Sequential, Enable remap
constexpr uint8_t COM_PINS_ALT_ENABLE = 0x32;   // Alternative, Enable remap

// === Timing & Driving ===
constexpr uint8_t SET_CLOCK_DIV = 0xD5;      // +1 байт: [7:4]=freq, [3:0]=div
constexpr uint8_t SET_PRECHARGE = 0xD9;      // +1 байт: [7:4]=phase2, [3:0]=phase1
constexpr uint8_t SET_VCOM_DESELECT = 0xDB;  // +1 байт: уровень VCOMH

// === Charge Pump ===
constexpr uint8_t SET_CHARGE_PUMP = 0x8D;  // +1 байт: enable/disable
constexpr uint8_t CHARGE_PUMP_DISABLE = 0x10;
constexpr uint8_t CHARGE_PUMP_ENABLE = 0x14;

// === Scroll Commands (опционально) ===
constexpr uint8_t DEACTIVATE_SCROLL = 0x2E;
constexpr uint8_t ACTIVATE_SCROLL = 0x2F;

// === Timing defaults ===
constexpr uint8_t DEFAULT_CLOCK_DIV = 0x80;      // Частота по умолчанию
constexpr uint8_t DEFAULT_CONTRAST = 0x7F;       // Контраст 50%
constexpr uint8_t DEFAULT_PRECHARGE = 0xF1;      // Для internal charge pump
constexpr uint8_t DEFAULT_PRECHARGE_EXT = 0x22;  // Для external VCC
constexpr uint8_t DEFAULT_VCOM = 0x40;           // VCOMH deselect level

}  // namespace cmd
}  // namespace oled_lib

#endif  // OLED_SSD1315_COMMANDS_HPP
