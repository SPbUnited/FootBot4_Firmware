/**
 * @file Gfx.hpp
 * @brief Графический слой: буфер, примитивы, текст
 */

#ifndef OLED_GFX_HPP
#define OLED_GFX_HPP

#include <cstddef>
#include <cstdint>

#include "../OledConfig.hpp"

#if OLED_ENABLED

namespace oled_lib
{

/**
 * @brief Графический контекст для работы с framebuffer
 *
 * Буфер организован постранично (page mode):
 * - каждая страница = 8 строк пикселей
 * - байт = 8 вертикальных пикселей в одной колонке
 * - LSB = верхний пиксель страницы
 */
class Gfx
{
  public:
    /**
     * @brief Конструктор по умолчанию (для статического размещения)
     */
    Gfx() : buffer_(nullptr), width_(0), height_(0) {}

    /**
     * @brief Инициализация с буфером
     * @param buffer Внешний буфер (передаётся пользователем)
     * @param width Ширина дисплея в пикселях
     * @param height Высота дисплея в пикселях (должна быть кратна 8)
     */
    void init(uint8_t* buffer, uint16_t width, uint16_t height);

    /**
     * @brief Проверка инициализации
     */
    bool isInitialized() const
    {
        return buffer_ != nullptr;
    }

    /**
     * @brief Получить указатель на буфер
     */
    uint8_t* buffer()
    {
        return buffer_;
    }
    const uint8_t* buffer() const
    {
        return buffer_;
    }

    /**
     * @brief Размер буфера в байтах
     */
    size_t bufferSize() const
    {
        return static_cast<size_t>(width_) * height_ / 8;
    }

    /**
     * @brief Ширина в пикселях
     */
    uint16_t width() const
    {
        return width_;
    }

    /**
     * @brief Высота в пикселях
     */
    uint16_t height() const
    {
        return height_;
    }

    // === Очистка и заливка ===

    /**
     * @brief Очистить буфер (все пиксели выключены)
     */
    void clear();

    /**
     * @brief Заполнить буфер
     * @param color true = все пиксели включены
     */
    void fill(bool color);

    // === Примитивы ===

    /**
     * @brief Установить пиксель
     * @param x Координата X (0 = левый край)
     * @param y Координата Y (0 = верхний край)
     * @param color true = включён
     */
    void pixel(int x, int y, bool color);

    /**
     * @brief Нарисовать линию (алгоритм Брезенхэма)
     */
    void line(int x0, int y0, int x1, int y1, bool color);

    /**
     * @brief Нарисовать прямоугольник (контур)
     */
    void rect(int x, int y, int w, int h, bool color);

    /**
     * @brief Нарисовать залитый прямоугольник
     */
    void rectFill(int x, int y, int w, int h, bool color);

    // === Текст ===

    /**
     * @brief Установить позицию курсора для текста
     */
    void setCursor(int x, int y);

    /**
     * @brief Установить масштаб текста (1 = 5x7, 2 = 10x14 и т.д.)
     */
    void setTextSize(uint8_t scale);

    /**
     * @brief Установить цвет текста
     * @param color true = белый на чёрном
     */
    void setTextColor(bool color);

    /**
     * @brief Вывести строку (поддержка UTF-8, включая русский)
     */
    void print(const char* str);

    /**
     * @brief Вывести один ASCII символ
     */
    void drawChar(int x, int y, char c, bool color, uint8_t scale);

    /**
     * @brief Вывести символ по Unicode codepoint (поддержка кириллицы)
     */
    void drawGlyph(int x, int y, uint16_t codepoint, bool color, uint8_t scale);

  private:
    /**
     * @brief Быстрая горизонтальная линия
     */
    void hLine(int x, int y, int w, bool color);

    /**
     * @brief Быстрая вертикальная линия
     */
    void vLine(int x, int y, int h, bool color);

    /**
     * @brief Декодировать UTF-8 символ из строки
     * @param str Указатель на текущую позицию в строке
     * @param codepoint Выходной Unicode codepoint
     * @return Количество прочитанных байт (1-4), или 0 при ошибке
     */
    static int decodeUtf8(const char* str, uint16_t& codepoint);

    uint8_t* buffer_;
    uint16_t width_;
    uint16_t height_;

    // Состояние текста
    int cursorX_ = 0;
    int cursorY_ = 0;
    uint8_t textScale_ = 1;
    bool textColor_ = true;
};

}  // namespace oled_lib

#else  // OLED_ENABLED == 0

namespace oled
{

// Заглушка
class Gfx
{
  public:
    Gfx() {}
    void init(uint8_t*, uint16_t, uint16_t) {}
    bool isInitialized() const
    {
        return false;
    }
    uint8_t* buffer()
    {
        return nullptr;
    }
    const uint8_t* buffer() const
    {
        return nullptr;
    }
    size_t bufferSize() const
    {
        return 0;
    }
    uint16_t width() const
    {
        return 0;
    }
    uint16_t height() const
    {
        return 0;
    }
    void clear() {}
    void fill(bool) {}
    void pixel(int, int, bool) {}
    void line(int, int, int, int, bool) {}
    void rect(int, int, int, int, bool) {}
    void rectFill(int, int, int, int, bool) {}
    void setCursor(int, int) {}
    void setTextSize(uint8_t) {}
    void setTextColor(bool) {}
    void print(const char*) {}
    void drawChar(int, int, char, bool, uint8_t) {}
    void drawGlyph(int, int, uint16_t, bool, uint8_t) {}
};

}  // namespace oled

#endif  // OLED_ENABLED

#endif  // OLED_GFX_HPP
