#include "mainloop.hpp"

#include "devices/bldc/bldc.hpp"
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
constexpr float battery_critical_voltage = 19.8f;
constexpr float battery_warning_voltage = 21.0f;
constexpr float motor_overheat_celsius = 75.0f;
constexpr uint32_t battery_warning_duration_us = 10U * 1000000U;

constexpr drivers::buzzer::MelodyNote alarm_melody[] = {
    {drivers::buzzer::Pitch::A, 5, drivers::buzzer::Length::Eighth},
    {drivers::buzzer::Pitch::Rest, 5, drivers::buzzer::Length::Eighth},
};

constexpr drivers::buzzer::MelodyNote warning_melody[] = {
    {drivers::buzzer::Pitch::C, 5, drivers::buzzer::Length::Sixteenth},
    {drivers::buzzer::Pitch::Rest, 5, drivers::buzzer::Length::EighthDotted},
};

bool any_motor_overheated()
{
    for (size_t motor = 0; motor < 4; ++motor)
    {
        if (can_motor_temperature[motor] > motor_overheat_celsius)
        {
            return true;
        }
    }
    return false;
}

void update_power_alarm(uint32_t now_us)
{
    static bool alarm_latched = false;
    static bool warning_active = false;
    static uint32_t warning_started_us = 0;
    const float battery_voltage = can_battery_voltage;
    const bool critical_voltage =
        battery_voltage > 0.0f && battery_voltage < battery_critical_voltage;

    if (any_motor_overheated() || critical_voltage)
    {
        alarm_latched = true;
    }

    if (alarm_latched)
    {
        if (!drivers::buzzer_drv.is_playing())
        {
            drivers::buzzer_drv.play(alarm_melody, sizeof(alarm_melody) / sizeof(alarm_melody[0]),
                                     240);
        }
        return;
    }

    if (battery_voltage >= battery_critical_voltage && battery_voltage < battery_warning_voltage)
    {
        if (!warning_active)
        {
            warning_active = true;
            warning_started_us = now_us;
        }
        if (now_us - warning_started_us < battery_warning_duration_us &&
            !drivers::buzzer_drv.is_playing())
        {
            drivers::buzzer_drv.play(warning_melody,
                                     sizeof(warning_melody) / sizeof(warning_melody[0]), 240);
        }
    }
    else
    {
        warning_active = false;
        if (drivers::buzzer_drv.is_playing())
        {
            drivers::buzzer_drv.stop();
        }
    }
}

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
    update_power_alarm(drivers::system_clock::micros());

    // Plan
    mainloop_monitor_stage = 3;
    devices::robot_dev.plan();

    // Act
    mainloop_monitor_stage = 4;
    devices::robot_dev.act();

    ++mainloop_monitor_cycles;
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
