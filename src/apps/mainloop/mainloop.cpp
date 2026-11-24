#include "mainloop.hpp"

#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"
#include "inttypes.h"

extern "C"
{
    volatile uint32_t mainloop_monitor_cycles = 0;
    volatile uint32_t mainloop_monitor_stage = 0;
}

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
    mainloop_monitor_stage = 1;

    // Timer
    static uint32_t timer = 0;
    while (drivers::system_clock::micros() - timer < Ts_us)
        ;
    uint32_t delta = drivers::system_clock::micros() - timer;
    timer = drivers::system_clock::micros();

    // Sense
    mainloop_monitor_stage = 2;
    devices::robot_dev.sense();
    devices::nrf24_recv.recv_irq();
    devices::nrf24_recv.fetch();

    // Plan
    mainloop_monitor_stage = 3;
    devices::robot_dev.plan();

    // Act
    mainloop_monitor_stage = 4;
    devices::robot_dev.act();

    ++mainloop_monitor_cycles;
    static uint8_t led_cycles = 0;
    if (++led_cycles == 10)
    {
        drivers::out_pins[drivers::LED_STM32].toggle();
        led_cycles = 0;
    }
    mainloop_monitor_stage = 0;
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
