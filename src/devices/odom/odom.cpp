#include "odom.hpp"

namespace devices::odom
{

Odometer::Odometer(OdometerConfig config) : OdometerConfig(config) {}

void Odometer::init() {}

void Odometer::update(devices::chassis::StateVector vel)
{
    state.x = state.x + vel.x * Ts;
    state.y = state.y + vel.y * Ts;
    state.theta = state.theta + vel.theta * Ts;
}

void Odometer::getState(devices::chassis::StateVector *state)
{
    *state = this->state;
}

}  // namespace devices::odom
