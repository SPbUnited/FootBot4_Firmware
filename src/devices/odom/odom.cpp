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

void Odometer::updateLocalDelta(devices::chassis::StateVector delta, float heading)
{
    float heading_delta = heading - state.theta;
    while (heading_delta > M_PI)
    {
        heading_delta -= 2.0f * M_PI;
    }
    while (heading_delta < -M_PI)
    {
        heading_delta += 2.0f * M_PI;
    }
    float mid_heading = state.theta + heading_delta * 0.5f;
    devices::chassis::StateVector global_delta = rotate(delta, mid_heading);
    state.x += global_delta.x;
    state.y += global_delta.y;
    state.theta = heading;
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
