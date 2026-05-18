#include "mainloop.hpp"

#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"
#include "inttypes.h"

namespace apps::mainloop
{

constexpr float dribbler_setting_to_vel = 250.0 / 16;
constexpr float kicker_setting_to_voltage = 200.0 / 16;

bool is_loop_pending(uint32_t time_elapsed, uint32_t period)
{
    return time_elapsed > period * 0.95;
}

void loop()
{
    // Timer
    static uint32_t timer = 0;
    while (drivers::system_clock::micros() - timer < Ts_us)
        ;
    uint32_t delta = drivers::system_clock::micros() - timer;
    timer = drivers::system_clock::micros();

    // Sense
    devices::robot_dev.sense();
    devices::nrf24_recv.fetch();

    // Plan
    devices::robot_dev.plan();

    // Act
    devices::robot_dev.act();
}

}  // namespace apps::mainloop

// float set_dribbler_target(float target)
// {
//     apps::mainloop::dribbler_target = target;
//     apps::mainloop::dribbler_update = true;
//     return apps::mainloop::dribbler_target;
// }
// SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC),
//                  set_dribbler_target, set_dribbler_target, set dribbler target);
