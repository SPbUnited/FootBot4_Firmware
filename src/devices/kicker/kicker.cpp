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
    target = devices::robot_dev.kicker_setting * (300/15);
    if (target >= 300)
    {
        target = 300;
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
        if (devices::robot_dev.kicker_mode == devices::robot::KICK_FRONT)
        {
            straight_pin->write(false);
        }
        else if (devices::robot_dev.kicker_mode == devices::robot::KICK_UP)
        {
            chip_pin->write(false);
        }
        else if(devices::robot_dev.kicker_mode == devices::robot::AUTOKICK_FRONT)
        {
            straight_pin->write(false);
        }
        else if(devices::robot_dev.kicker_mode == devices::robot::AUTOKICK_UP)
        {
            chip_pin->write(false);
        }
        
        kernel::delay_ms(2);
        // if 
        state = PREPARE;
    }
}

}