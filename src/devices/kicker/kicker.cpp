#ifdef BARE_METAL

#include "kicker.hpp"
// #include <math.h>

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

void Kicker::apply(const robot_core::KickerOutput &output)
{
    charge_pin->write(!output.charge);
    discharge_pin->write(!output.discharge);
    straight_pin->write(!output.straight);
    chip_pin->write(!output.chip);
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

#endif
