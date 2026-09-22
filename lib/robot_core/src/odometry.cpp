#include "robot_core/odometry.hpp"

#include <cmath>

namespace robot_core
{
constexpr double pi = 3.14159265358979323846;

Odometer::Odometer(OdometerConfig config) : OdometerConfig(config) {}

void Odometer::init()
{
    state = {};
}

void Odometer::update(StateVector vel)
{
    state.x = state.x + vel.x * Ts;
    state.y = state.y + vel.y * Ts;
    state.theta = state.theta + vel.theta * Ts;
    thetaAntiWindup();
}

void Odometer::getState(StateVector *state)
{
    *state = this->state;
}

void Odometer::thetaAntiWindup()
{
    while (state.theta > pi)
    {
        state.theta -= 2 * pi;
    }
    while (state.theta < -pi)
    {
        state.theta += 2 * pi;
    }
}

StateVector Odometer::rotate(StateVector vec, float angle)
{
    float c = cos(angle);
    float s = sin(angle);
    return {vec.x * c - vec.y * s, vec.x * s + vec.y * c, vec.theta};
}

}  // namespace robot_core
