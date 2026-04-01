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
    uint32_t delta = drivers::system_clock::micros() - timer;
    timer = drivers::system_clock::micros();

    if (state.counter % 1000 == 0)
    {
        state.deltamin = 0;
        state.deltamax = 0;
    }

    if (state.deltamax < delta)
    {
        state.deltamax = delta;
    }
    if (state.deltamin > delta || state.deltamin == 0)
    {
        state.deltamin = delta;
    }
    state.deltaavg = (0.9 * state.deltaavg + 0.1 * delta);

    drivers::leds.display_number(state.counter);

    state.counter++;

    uint8_t *array;
    float kekw = 200;
    uint8_t kkk[4];
    array = reinterpret_cast<uint8_t *>(&kekw);
    memcpy(kkk, array, 4);
    drivers::can_drv.write(0x70A, array, 4);
}

MainloopState get_state()
{
    return state;
}

}  // namespace apps::mainloop
