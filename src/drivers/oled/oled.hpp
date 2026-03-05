#pragma once

#include <oled/OledSsd1315.hpp>

namespace drivers::oled
{

class OledDriver : public oled_lib::OledConfig
{
  public:
    oled_lib::OledSsd1315 display;

    OledDriver(oled_lib::OledConfig cfg, I2C_HandleTypeDef *hi2c);

    void init();
};

}  // namespace drivers::oled
