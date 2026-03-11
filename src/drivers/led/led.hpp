#pragma once

#include "inttypes.h"

namespace drivers::led
{

enum LED
{
    STM32,
    DRV1,
    DRV2,
    DRV3,
    DRV4,
    DRV5,
    DATA_TRANSFER_STATUS_1,
    DATA_TRANSFER_STATUS_2,
    NUM_LEDS
};

struct LedDescriptor;

class LedDriver
{
  private:
  public:
    LedDriver() {}

    void init_led(LedDescriptor led);

    void init();

    void write(LED led, bool state);

    void toggle(LED led);

    void display_number(uint8_t value);
};

}  // namespace drivers::led
