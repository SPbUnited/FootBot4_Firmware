#include "mainloop.hpp"

#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"
#include "inttypes.h"

namespace apps::mainloop
{

MainloopState state = {0};

static uint32_t timer = 0;

bool is_loop_pending()
{
    return drivers::system_clock::micros() - timer > Ts_us * 0.95;
}

void loop()
{
    while (drivers::system_clock::micros() - timer < Ts_us)
        ;
    state.delta = drivers::system_clock::micros() - timer;
    timer = drivers::system_clock::micros();

    drivers::leds.display_number(state.counter);

    state.counter++;
}

MainloopState get_state()
{
    return state;
}

}  // namespace apps::mainloop
