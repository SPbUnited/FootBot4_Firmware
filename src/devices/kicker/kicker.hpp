#pragma once

#ifdef BARE_METAL

#include <inttypes.h>

#include "drivers/driver_manager.hpp"

namespace devices::kicker
{

enum KICKER_STATES
{
    PREPARE,
    KICK,
    AFTER_KICK,
};

enum KICK_TYPES
{
    STRAIGHT,
    CHIP,
};

struct KickerConfig
{
    drivers::gpio::GPIOAnalogInputDriver *adc_pin;  // Time step in seconds
    drivers::gpio::GPIOOutputDriver *charge_pin;
    drivers::gpio::GPIOOutputDriver *discharge_pin;
    drivers::gpio::GPIOOutputDriver *straight_pin;
    drivers::gpio::GPIOOutputDriver *chip_pin;
    drivers::gpio::GPIOInputDriver *ball_checker_deep;
    drivers::gpio::GPIOInputDriver *ball_checker_front;
};

class Kicker : public KickerConfig
{
  private:
    bool prepared = false;
    uint8_t kick_type = STRAIGHT;

  public:
    uint8_t state = PREPARE;
    double target = 0;
    double actual_voltage = 0;
    uint32_t timer = 0;
    Kicker(KickerConfig config);

    void init();

    void update();
    void set_target(uint16_t voltage);
    bool read_deep();
    bool read_front();
    float get_voltage();
};
}  // namespace devices::kicker

#endif
