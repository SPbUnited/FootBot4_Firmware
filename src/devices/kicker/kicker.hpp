#pragma once

#include <inttypes.h>
#include "drivers/driver_manager.hpp"
#include "stm32f4xx_hal.h"

namespace devices::kicker
{

enum KICKER_STATES
{
  PREPARE,
  KICK,
};

struct KickerConfig
{
    drivers::gpio::GPIOAnalogInputDriver *adc_pin;  // Time step in seconds
    drivers::gpio::GPIOOutputDriver *charge_pin;
    drivers::gpio::GPIOOutputDriver *discharge_pin;
    drivers::gpio::GPIOOutputDriver *straight_pin;
    drivers::gpio::GPIOOutputDriver *chip_pin;
};



class Kicker: public KickerConfig
{
  private:
    double target = 0;
    double actual_voltage = 0;
    uint8_t state = PREPARE;
    bool prepared = false;

  public:
    Kicker(KickerConfig config);

    void init();

    void update();
    void set_target(uint16_t voltage);
    float get_voltage();

};
}  // namespace devices::nrf24
