#include "kicker.hpp"
// #include <math.h>
#include "devices/device_manager.hpp"
#include "kernel/kernel.hpp"

namespace devices::kicker
{

Kicker::Kicker(KickerConfig config) : KickerConfig(config) {}

// Main receive function
void Kicker::init()
{
    target = 0;
}

float Kicker::get_voltage()
{
    double voltage = (adc_pin->read() * 3300) / (4096 - 1);
    actual_voltage = 140 * voltage * 0.001f;
    // kinfo("voltage");
    return actual_voltage;
}

void Kicker::set_target(uint16_t voltage)
{
    target = voltage;
}

void Kicker::update()
{
    get_voltage();

    float MIN_VOLTAGE = 100;
    float MAX_VOLTAGE = 330;

    if (devices::robot_dev.kicker_setting == 0)
    {
        target = 0;
    }
    else
    {
        target =
            (devices::robot_dev.kicker_setting - 1) / (15.0 - 1) * (MAX_VOLTAGE - MIN_VOLTAGE) +
            MIN_VOLTAGE;
    }

    if (target >= 330)
    {
        target = 330;
    }
    else if (target <= 0)
    {
        target = 0;
    }
    if ((state == PREPARE) || !prepared)
    {
        straight_pin->write(true);
        chip_pin->write(true);
        double bound = target * 0.05;
        if (bound < 5)
        {
            bound = 5;
        }
        else if (bound > 10)
        {
            bound = 10;
        }
        if (target - actual_voltage > bound)
        {
            discharge_pin->write(true);
            charge_pin->write(false);
            prepared = false;
        }
        else if (target - actual_voltage < -bound)
        {
            discharge_pin->write(false);
            charge_pin->write(true);
            prepared = false;
        }
        else
        {
            discharge_pin->write(true);
            charge_pin->write(true);
            // state = KICK;
            prepared = true;
            if (devices::robot_dev.kicker_mode != devices::robot::DISABLED)
            {
                state = KICK;
            }
        }
        if (read_deep())
        {
            drivers::out_pins[drivers::LED_DRV2].write(true);
        }
        else
        {
            drivers::out_pins[drivers::LED_DRV2].write(false);
        }

        if (read_front())
        {
            drivers::out_pins[drivers::LED_DRV3].write(true);
        }
        else
        {
            drivers::out_pins[drivers::LED_DRV3].write(false);
        }
    }
    else if (state == KICK)
    {
        discharge_pin->write(true);
        charge_pin->write(true);

        state = PREPARE;
        
        if (devices::robot_dev.kicker_mode == devices::robot::KICK_FRONT)
        {
            timer = drivers::system_clock::micros();
            straight_pin->write(false);
            state = AFTER_KICK;
        }
        else if (devices::robot_dev.kicker_mode == devices::robot::KICK_UP)
        {
            timer = drivers::system_clock::micros();
            chip_pin->write(false);
            state = AFTER_KICK;
        }
        else if (devices::robot_dev.kicker_mode == devices::robot::AUTOKICK_FRONT)
        {
            // timer = drivers::system_clock::micros();
            if (read_deep() && read_front())
            {
                timer = drivers::system_clock::micros();
                straight_pin->write(false);
                state = AFTER_KICK;
            }
        }
        else if (devices::robot_dev.kicker_mode == devices::robot::AUTOKICK_UP)
        {
            if (read_deep() && read_front())
            {
                timer = drivers::system_clock::micros();
                chip_pin->write(false);
                state = AFTER_KICK;
            }
        }
        else if (devices::robot_dev.kicker_mode == devices::robot::AUTOKICK_MOMENTUM)
        {
            // timer = drivers::system_clock::micros();
            if (read_deep() || read_front())
            {
                timer = drivers::system_clock::micros();
                straight_pin->write(false);
                state = AFTER_KICK;
            }
        }

        // if(drivers::system_clock::micros() - timer > 5000)
        // {
        //     state = PREPARE;
        // }
        // kernel::delay_ms(2);
        // if
        // drivers::system_clock::micros()
        
    }
    else if (state == AFTER_KICK)
    {
        if((drivers::system_clock::micros() - timer > 10000) && (actual_voltage < 10))
        {
            state = PREPARE;
        }
    }
}

bool Kicker::read_deep()
{
    return !ball_checker_deep->read();
}

bool Kicker::read_front()
{
    return !ball_checker_front->read();
}

}  // namespace devices::kicker