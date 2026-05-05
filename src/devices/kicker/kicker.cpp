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
        if (read_deep())
        {
            drivers::out_pins[drivers::LED_DRV2].write(true);
        }
        else{
            drivers::out_pins[drivers::LED_DRV2].write(false);
        }

        if (read_front())
        {
            drivers::out_pins[drivers::LED_DRV3].write(true);
        }
        else{
            drivers::out_pins[drivers::LED_DRV3].write(false);
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
            if (read_deep() && read_front())
            {
                straight_pin->write(false);
            }
        }
        else if(devices::robot_dev.kicker_mode == devices::robot::AUTOKICK_UP)
        {
            if (read_deep() && read_front())
            {
                chip_pin->write(false);
            }
        }
        
        // kernel::delay_ms(2);
        // if 
        state = PREPARE;
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

}