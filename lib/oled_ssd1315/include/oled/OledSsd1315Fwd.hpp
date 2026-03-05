/**
 * @file OledSsd1315Fwd.hpp
 * @brief Forward declarations для OLED SSD1315
 *
 * Используйте этот заголовок для forward declarations в других заголовках,
 * чтобы минимизировать зависимости.
 */

#ifndef OLED_SSD1315_FWD_HPP
#define OLED_SSD1315_FWD_HPP

namespace oled_lib
{

class OledSsd1315;
class Gfx;
class Ssd1315Driver;
struct OledConfig;
enum class OledResult;
enum class VccMode;

// Интерфейс I2C транспорта
struct II2c;

}  // namespace oled_lib

#endif  // OLED_SSD1315_FWD_HPP
