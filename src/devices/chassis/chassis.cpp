#include "chassis.hpp"

#include <math.h>

#include "devices/device_manager.hpp"

namespace devices::chassis
{

Chassis::Chassis(ChassisConfig config) : ChassisConfig(config) {}

void Chassis::init()
{
    logDebug(
        "Chassis initialized:\n wheel_angles: %f %f %f %f\n  wheel_radius: %f\n  "
        "robot_radius: %f",
        wheel_angles[0], wheel_angles[1], wheel_angles[2], wheel_angles[3], wheel_radius,
        robot_radius);
}

void Chassis::setVel(StateVector vel)
{
    logDebug("setVel: %f %f %f", vel.x, vel.y, vel.theta);

    bldc::BldcsVec wheel_vels;

    float J_sin[4] = {
        -sin(wheel_angles[0]),
        -sin(wheel_angles[1]),
        sin(wheel_angles[2]),
        sin(wheel_angles[3]),
    };

    float J_cos[4] = {
        cos(wheel_angles[0]),
        -cos(wheel_angles[1]),
        -cos(wheel_angles[2]),
        cos(wheel_angles[3]),
    };

    for (int i = 0; i < 4; i++)
    {
        wheel_vels.vec[i] =
            1 / wheel_radius * (vel.x * J_sin[i] + vel.y * J_cos[i] + vel.theta * robot_radius);
    }

    bldcs->setDriveVel(wheel_vels);
}
void Chassis::getVel(StateVector *vel)
{
    logDebug("getVel: %f %f %f", vel->x, vel->y, vel->theta);
}

}  // namespace devices::chassis