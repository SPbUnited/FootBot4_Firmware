#pragma once

#include "i2c/i2c.hpp"
#include "led/led.hpp"
#include "oled/oled.hpp"
#include "stm32f4xx_hal.h"
#include "uart/uart.hpp"

namespace drivers
{

extern uart::UartDriver uart4;
extern uart::UartDriver uart1;
extern i2c::I2cDriver i2c2;
extern led::LedDriver leds;
extern oled::OledDriver oled_drv;

}  // namespace drivers
