#include "mainloop.hpp"

#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"
#include "inttypes.h"

namespace apps::mainloop
{

bool is_loop_pending(uint32_t time_elapsed, uint32_t period)
{
    return time_elapsed > period * 0.95;
}

float dribbler_target = 0.0;
bool dribbler_update = true;

void loop()
{
    static uint32_t timer = 0;
    while (drivers::system_clock::micros() - timer < Ts_us)
        ;
    uint32_t delta = drivers::system_clock::micros() - timer;
    timer = drivers::system_clock::micros();

    logSetLevel(&devices::logger::uartLog, LOG_NONE);

    // drivers::leds.display_number(state.counter);

    devices::chassis::StateVector vel = {0, 0, 0};
    devices::chassis_drv.getVel(&vel);
    devices::odom_dev.update(vel);

    if (dribbler_update)
    {
        devices::bldcs_drv.setDribblerVel(dribbler_target);
        dribbler_update = false;
    }
    logSetLevel(&devices::logger::uartLog, LOG_DEBUG);
}

}  // namespace apps::mainloop

float set_dribbler_target(float target)
{
    apps::mainloop::dribbler_target = target;
    apps::mainloop::dribbler_update = true;
    return apps::mainloop::dribbler_target;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC),
                 set_dribbler_target, set_dribbler_target, set dribbler target);
