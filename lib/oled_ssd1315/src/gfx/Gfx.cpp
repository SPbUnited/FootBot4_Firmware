/**
 * @file Gfx.cpp
 * @brief Реализация графического слоя
 */

#include "../../include/oled/domain/Gfx.hpp"

#if OLED_ENABLED

#include <algorithm>
#include <cstring>

#include "Font5x7.hpp"
#include "FontCyrillic5x7.hpp"

namespace oled_lib
{

void Gfx::init(uint8_t* buffer, uint16_t width, uint16_t height)
{
    buffer_ = buffer;
    width_ = width;
    height_ = height;
    cursorX_ = 0;
    cursorY_ = 0;
    textScale_ = 1;
    textColor_ = true;
}

void Gfx::clear()
{
    if (buffer_)
    {
        memset(buffer_, 0x00, bufferSize());
    }
}

void Gfx::fill(bool color)
{
    if (buffer_)
    {
        memset(buffer_, color ? 0xFF : 0x00, bufferSize());
    }
}

void Gfx::pixel(int x, int y, bool color)
{
    // Проверка границ
    if (x < 0 || x >= width_ || y < 0 || y >= height_)
    {
        return;
    }
    if (!buffer_)
    {
        return;
    }

    // Вычисление позиции в буфере
    // Буфер организован: page[0..7] * width колонок
    // Каждый байт = 8 вертикальных пикселей (LSB = верхний)
    uint16_t page = y / 8;
    uint8_t bit = y % 8;
    size_t idx = static_cast<size_t>(page) * width_ + x;

    if (color)
    {
        buffer_[idx] |= (1 << bit);
    }
    else
    {
        buffer_[idx] &= ~(1 << bit);
    }
}

void Gfx::line(int x0, int y0, int x1, int y1, bool color)
{
    // Алгоритм Брезенхэма
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        pixel(x0, y0, color);

        if (x0 == x1 && y0 == y1)
        {
            break;
        }

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void Gfx::hLine(int x, int y, int w, bool color)
{
    for (int i = 0; i < w; ++i)
    {
        pixel(x + i, y, color);
    }
}

void Gfx::vLine(int x, int y, int h, bool color)
{
    for (int i = 0; i < h; ++i)
    {
        pixel(x, y + i, color);
    }
}

void Gfx::rect(int x, int y, int w, int h, bool color)
{
    if (w <= 0 || h <= 0)
        return;

    hLine(x, y, w, color);          // Верхняя
    hLine(x, y + h - 1, w, color);  // Нижняя
    vLine(x, y, h, color);          // Левая
    vLine(x + w - 1, y, h, color);  // Правая
}

void Gfx::rectFill(int x, int y, int w, int h, bool color)
{
    for (int j = 0; j < h; ++j)
    {
        hLine(x, y + j, w, color);
    }
}

void Gfx::setCursor(int x, int y)
{
    cursorX_ = x;
    cursorY_ = y;
}

void Gfx::setTextSize(uint8_t scale)
{
    textScale_ = (scale > 0) ? scale : 1;
}

void Gfx::setTextColor(bool color)
{
    textColor_ = color;
}

void Gfx::drawChar(int x, int y, char c, bool color, uint8_t scale)
{
    // Проверка диапазона символа
    if (c < FONT_FIRST_CHAR || c > FONT_LAST_CHAR)
    {
        c = ' ';  // Заменяем неизвестные символы пробелом
    }

    // Индекс в таблице шрифта
    size_t charIdx = static_cast<size_t>(c - FONT_FIRST_CHAR) * FONT_WIDTH;

    // Рисуем каждую колонку символа
    for (uint8_t col = 0; col < FONT_WIDTH; ++col)
    {
        uint8_t colData = FONT_5X7[charIdx + col];

        // Рисуем каждый бит (строку)
        for (uint8_t row = 0; row < FONT_HEIGHT; ++row)
        {
            bool pixelOn = (colData >> row) & 0x01;

            // Если пиксель включён (или инверсия для фона)
            if (pixelOn)
            {
                // С учётом масштаба
                for (uint8_t sy = 0; sy < scale; ++sy)
                {
                    for (uint8_t sx = 0; sx < scale; ++sx)
                    {
                        pixel(x + col * scale + sx, y + row * scale + sy, color);
                    }
                }
            }
        }
    }
}

int Gfx::decodeUtf8(const char* str, uint16_t& codepoint)
{
    if (!str || !*str)
    {
        codepoint = 0;
        return 0;
    }

    uint8_t b0 = static_cast<uint8_t>(str[0]);

    // ASCII (0xxxxxxx)
    if ((b0 & 0x80) == 0)
    {
        codepoint = b0;
        return 1;
    }

    // 2-байтовая последовательность (110xxxxx 10xxxxxx)
    if ((b0 & 0xE0) == 0xC0)
    {
        if (!str[1])
            return 0;
        uint8_t b1 = static_cast<uint8_t>(str[1]);
        if ((b1 & 0xC0) != 0x80)
            return 0;

        codepoint = ((b0 & 0x1F) << 6) | (b1 & 0x3F);
        return 2;
    }

    // 3-байтовая последовательность (1110xxxx 10xxxxxx 10xxxxxx)
    if ((b0 & 0xF0) == 0xE0)
    {
        if (!str[1] || !str[2])
            return 0;
        uint8_t b1 = static_cast<uint8_t>(str[1]);
        uint8_t b2 = static_cast<uint8_t>(str[2]);
        if ((b1 & 0xC0) != 0x80 || (b2 & 0xC0) != 0x80)
            return 0;

        codepoint = ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
        return 3;
    }

    // 4-байтовая последовательность - не поддерживаем (codepoint > 16 бит)
    // Пропускаем как ошибку
    codepoint = '?';
    return 1;
}

void Gfx::drawGlyph(int x, int y, uint16_t codepoint, bool color, uint8_t scale)
{
    const uint8_t* glyph = nullptr;

    // ASCII символы
    if (codepoint >= FONT_FIRST_CHAR && codepoint <= FONT_LAST_CHAR)
    {
        size_t charIdx = static_cast<size_t>(codepoint - FONT_FIRST_CHAR) * FONT_WIDTH;
        glyph = &FONT_5X7[charIdx];
    }
    // Кириллица
    else
    {
        glyph = getCyrillicGlyph(codepoint);
    }

    // Если глиф не найден - рисуем пробел
    if (!glyph)
    {
        return;
    }

    // Рисуем каждую колонку символа
    for (uint8_t col = 0; col < FONT_WIDTH; ++col)
    {
        uint8_t colData = glyph[col];

        // Рисуем каждый бит (строку)
        for (uint8_t row = 0; row < FONT_HEIGHT; ++row)
        {
            bool pixelOn = (colData >> row) & 0x01;

            if (pixelOn)
            {
                // С учётом масштаба
                for (uint8_t sy = 0; sy < scale; ++sy)
                {
                    for (uint8_t sx = 0; sx < scale; ++sx)
                    {
                        pixel(x + col * scale + sx, y + row * scale + sy, color);
                    }
                }
            }
        }
    }
}

void Gfx::print(const char* str)
{
    if (!str)
        return;

    while (*str)
    {
        uint16_t codepoint;
        int len = decodeUtf8(str, codepoint);

        if (len == 0)
        {
            // Ошибка декодирования - пропускаем байт
            str++;
            continue;
        }

        str += len;

        // Перенос строки
        if (codepoint == '\n')
        {
            cursorX_ = 0;
            cursorY_ += (FONT_HEIGHT + 1) * textScale_;
            continue;
        }

        // Возврат каретки
        if (codepoint == '\r')
        {
            cursorX_ = 0;
            continue;
        }

        // Рисуем символ
        drawGlyph(cursorX_, cursorY_, codepoint, textColor_, textScale_);

        // Сдвигаем курсор (ширина символа + 1 пиксель межбуквенный интервал)
        cursorX_ += (FONT_WIDTH + 1) * textScale_;

        // Автоперенос на следующую строку
        if (cursorX_ + static_cast<int>(FONT_WIDTH * textScale_) > static_cast<int>(width_))
        {
            cursorX_ = 0;
            cursorY_ += (FONT_HEIGHT + 1) * textScale_;
        }
    }
}

}  // namespace oled_lib

#endif  // OLED_ENABLED
