#pragma once

#include <oled/OledSsd1315.hpp>

namespace devices::oled
{

class OledDriver : public oled_lib::OledConfig
{
  public:
    oled_lib::OledSsd1315 display;

    OledDriver(oled_lib::OledConfig cfg, I2C_HandleTypeDef *hi2c);

    void init();
    bool ready();
    bool submit();
    void transferComplete(bool success);
    volatile uint32_t transfer_errors = 0;
    volatile uint32_t max_transfer_us = 0;
  private:
    I2C_HandleTypeDef *bus_;
    uint32_t started_us_ = 0;
};

}  // namespace devices::oled
