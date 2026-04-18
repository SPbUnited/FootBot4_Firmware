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

}

float Kicker::get_voltage()
{
    return adc_pin.read();
}
void Kicker::set_target(uint16_t voltage)
{
    target = voltage;
}
void Kicker::update()
{
    float actual_voltage = get_voltage();
    if (target - actual_voltage > target * 0.1)
    {
        discharge_pin.write(false);
        charge_pin.write(true);

    }
    else if (target - actual_voltage < -target * 0.1)
    {
        discharge_pin.write(true);
        charge_pin.write(false);
    }
    else
    {
        discharge_pin.write(false);
        charge_pin.write(false);
    }
}

}