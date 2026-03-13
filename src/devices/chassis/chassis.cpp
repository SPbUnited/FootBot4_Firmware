#include "chassis.hpp"

#include <math.h>

#include "devices/device_manager.hpp"
#include "vt_linalg"

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

    vt::numeric_matrix<4, 3> J({
        {-cos(wheel_angles[0]), -sin(wheel_angles[0]), robot_radius},
        {-cos(wheel_angles[1]), -sin(wheel_angles[1]), robot_radius},
        {-cos(wheel_angles[2]), -sin(wheel_angles[2]), robot_radius},
        {-cos(wheel_angles[3]), -sin(wheel_angles[3]), robot_radius},
    });

    vt::numeric_vector<3> vel_vec({vel.x, vel.y, vel.theta});

    vt::numeric_vector<4> w =
        vt::numeric_matrix<4, 4>::diagonals(1.0 / wheel_radius) * J * vel_vec;

    for (int i = 0; i < 4; i++)
    {
        wheel_vels.vec[i] = w[i];
    }

    bldcs->setDriveVel(wheel_vels);
}
void Chassis::getVel(StateVector *vel)
{
    logDebug("getVel: %f %f %f", vel->x, vel->y, vel->theta);
}

}  // namespace devices::chassis