#pragma once

#include "drivers/gpio/gpio.hpp"
#include "drivers/system_clock/system_clock.hpp"
#include "stm32f4xx_hal.h"

namespace drivers::buzzer
{

struct BuzzerConfig
{
    drivers::gpio::GPIOOutputDriver *buzzer_pin;
    drivers::gpio::GPIOOutputDriver *buzzer_pin2;
};

class Buzzer : public BuzzerConfig
{
  private:
  public:
    Buzzer(BuzzerConfig config) : BuzzerConfig(config) {}

    void init() {}

    void buzz(uint32_t freq, uint32_t dur_ms)
    {
        // buzzer_pin->write(1);
        // buzzer_pin2->write(1);
        // HAL_Delay(500);
        // buzzer_pin->write(0);
        // buzzer_pin2->write(0);
        uint32_t period = 1000000 / freq;

        for (uint32_t eta = 0; eta < dur_ms * 1000; eta += period)
        {
            uint32_t time = system_clock::micros();
            buzzer_pin->toggle();
            buzzer_pin2->toggle();

            while (system_clock::micros() - time < period)
                ;
        }
    }
};

}  // namespace drivers::buzzer
