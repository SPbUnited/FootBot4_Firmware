#pragma once

#include "bootstrap/bootstrap.hpp"
#include "buzzer/buzzer.hpp"
#include "i2c/i2c.hpp"
#include "led/led.hpp"
#include "oled/oled.hpp"
#include "stm32f4xx_hal.h"
#include "system_clock/system_clock.hpp"
#include "uart/uart.hpp"

namespace drivers
{

extern bootstrap::Bootstrap bootstrap_drv;
extern system_clock::SystemClock system_clock_drv;
extern uart::UartDriver uart4;
extern uart::UartDriver uart1;
extern i2c::I2cDriver i2c2;
extern led::LedDriver leds;
extern oled::OledDriver oled_drv;
extern buzzer::Buzzer buzzer_drv;

}  // namespace drivers
