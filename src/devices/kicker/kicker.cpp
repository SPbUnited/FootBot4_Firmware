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
    target = 150;
}

float Kicker::get_voltage()
{
    double voltage = (adc_pin->read() * 3300) / (4096 - 1);
    actual_voltage = actual_voltage * 0.9f + 140 * voltage * 0.001f * 0.1f;
    // kinfo("voltage");
    return actual_voltage;
    
}

void Kicker::set_target(uint16_t voltage)
{
    target = voltage;
}

void Kicker::update()
{
    if ((state == PREPARE) || !prepared)
    {
        straight_pin->write(true);
        chip_pin->write(true);
        double bound = target * 0.05;
        if (bound < 5)
        {
            bound = 5;
        }
        else if (bound > 15)
        {
            bound = 15;
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
    }
    else if (state == KICK)
    {
        discharge_pin->write(true);
        charge_pin->write(true);
        if (kick_type == STRAIGHT)
        {
            straight_pin->write(false);
        }
        else if (kick_type == CHIP)
        {
            chip_pin->write(false);
        }
        kernel::scheduler::delay_ms(2);
        // if 
        state = PREPARE;
    }
}

}