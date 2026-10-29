#pragma once

#include "stm32f4xx_hal.h"

namespace drivers::buzzer
{

// Passive buzzer on PF13: it sounds only while the pin is toggled, so TIM7 toggles it from
// an interrupt and the main loop just switches the tone on and off
class Buzzer
{
  private:
    static constexpr uint32_t kToneFreq_Hz = 2000;

    bool m_on = false;

  public:
    Buzzer() {}

    void init();
    void set(bool on);
};

}  // namespace drivers::buzzer
