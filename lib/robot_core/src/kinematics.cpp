#include "robot_core/kinematics.hpp"

#include <cmath>
namespace robot_core
{
Kinematics::Kinematics(Geometry geometry)
{
    // Preserve the firmware's calibrated inverse; recalibration is a separate change.
    constexpr double inverse[3][4] = {{-0.372225, -0.276332, 0.276332, 0.372225},
                                      {0.394976, -0.394976, -0.394976, 0.394976},
                                      {3.95441, 2.58154, 2.58154, 3.95441}};
    // Cache the matrices, as in the original controller, to avoid trigonometry per tick.
    for (unsigned i = 0; i < 4; ++i)
    {
        forward_[i][0] = -std::sin(geometry.wheel_angles[i]) / double(geometry.wheel_radius);
        forward_[i][1] = std::cos(geometry.wheel_angles[i]) / double(geometry.wheel_radius);
        forward_[i][2] = geometry.robot_radius / double(geometry.wheel_radius);
        for (unsigned r = 0; r < 3; ++r)
            inverse_[r][i] = inverse[r][i] * geometry.wheel_radius;
    }
}
WheelVector Kinematics::wheels(StateVector body) const
{
    WheelVector result{};
    for (unsigned i = 0; i < 4; ++i)
        result.vec[i] =
            float(forward_[i][0] * body.x + forward_[i][1] * body.y + forward_[i][2] * body.theta);
    return result;
}
StateVector Kinematics::body(WheelVector wheels) const
{
    StateVector result{};
    for (unsigned r = 0; r < 3; ++r)
    {
        double value = 0;
        for (unsigned c = 0; c < 4; ++c)
            value += inverse_[r][c] * wheels.vec[c];
        result.vec[r] = float(value);
    }
    return result;
}
}  // namespace robot_core
