#include "chassis.hpp"
namespace devices::chassis
{
Chassis::Chassis(ChassisConfig config)
    : ChassisConfig(config),
      kinematics(robot_core::Geometry{{config.wheel_angles[0], config.wheel_angles[1],
                                       config.wheel_angles[2], config.wheel_angles[3]},
                                      config.wheel_radius,
                                      config.robot_radius})
{
}
void Chassis::init() {}
void Chassis::setVel(StateVector vel)
{
    bldcs->setDriveVel(kinematics.wheels(vel));
}
void Chassis::getVel(StateVector *vel)
{
    bldc::BldcsVec wheels{};
    bldcs->getDriveVel(&wheels);
    *vel = kinematics.body(wheels);
}
void Chassis::getVel(StateVector *vel, bldc::BldcsVec wheels)
{
    *vel = kinematics.body(wheels);
}
}  // namespace devices::chassis
