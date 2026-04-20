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
    actual_voltage = actual_voltage * 0.9f + 135 * voltage * 0.001f * 0.1f;
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
        straight_pin->write(false);
        chip_pin->write(false);
        if (target - actual_voltage > target * 0.05)
        {
            discharge_pin->write(true);
            charge_pin->write(false);
            prepared = false;
        }
        else if (target - actual_voltage < -target * 0.05)
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
        }
    }
    else if (state == KICK)
    {
        discharge_pin->write(false);
        charge_pin->write(false);
        if (0)
        {
            straight_pin->write(true);
        }
        else if (1)
        {
            chip_pin->write(true);
        }
    }
}

}