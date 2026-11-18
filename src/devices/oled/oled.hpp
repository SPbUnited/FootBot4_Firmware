#pragma once

#include <oled/OledSsd1315.hpp>

namespace devices::oled
{

class OledDriver : public oled_lib::OledConfig
{
  public:
    oled_lib::OledSsd1315 display;

    OledDriver(oled_lib::OledConfig cfg, I2C_HandleTypeDef *hi2c);

    /** Initializes display; returns false if the controller does not respond. */
    bool init();
};

}  // namespace devices::oled
