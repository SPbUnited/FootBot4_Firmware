#include "odom.hpp"

#include "math.hpp"

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
    while (state.theta > M_PI)
    {
        state.theta -= 2 * M_PI;
    }
    while (state.theta < -M_PI)
    {
        state.theta += 2 * M_PI;
    }
}

devices::chassis::StateVector Odometer::rotate(devices::chassis::StateVector vec, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return {vec.x * c - vec.y * s, vec.x * s + vec.y * c, vec.theta};
}

}  // namespace devices::odom
