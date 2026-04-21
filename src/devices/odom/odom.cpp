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
    thetaAntiWindup();
}

void Odometer::getState(devices::chassis::StateVector *state)
{
    *state = this->state;
}

void Odometer::thetaAntiWindup()
{
    const float M_PI = 3.14159265358979323846;

    while (state.theta > M_PI)
    {
        state.theta -= 2 * M_PI;
    }
    while (state.theta < -M_PI)
    {
        state.theta += 2 * M_PI;
    }
}

}  // namespace devices::odom
