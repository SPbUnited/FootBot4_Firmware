#pragma once

#include <inttypes.h>
#include "drivers/driver_manager.hpp"
#include "stm32f4xx_hal.h"

namespace devices::kicker
{

struct KickerConfig
{
    drivers::gpio::GPIOAnalogInputDriver adc_pin;  // Time step in seconds
    drivers::gpio::GPIOOutputDriver charge_pin;
    drivers::gpio::GPIOOutputDriver discharge_pin;
    drivers::gpio::GPIOOutputDriver straight_pin;
    drivers::gpio::GPIOOutputDriver chip_pin;
};

class Kicker: public KickerConfig
{
  public:
    Kicker(KickerConfig config);

    void init();

    void update();
};
}  // namespace devices::nrf24
