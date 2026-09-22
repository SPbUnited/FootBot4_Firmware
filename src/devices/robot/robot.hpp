#pragma once
#include "devices/bldc/bldc.hpp"
#include "devices/chassis/chassis.hpp"
#include "devices/imu/imu.hpp"
#include "devices/kicker/kicker.hpp"
#include "devices/odom/odom.hpp"
#include "robot_core/robot.hpp"
namespace devices::robot
{
using namespace robot_core;
struct RobotConfig
{
    devices::odom::Odometer &odom_dev;
    devices::chassis::Chassis &chassis_drv;
    devices::bldc::BldcsDriver &bldcs_drv;
    devices::imu::BNO055 &bno055_drv;
    devices::kicker::Kicker &kicker_drv;
};

uint8_t calculate_signature(RobotSettings config);
struct Robot : robot_core::Robot, RobotConfig
{
    Robot(RobotSettings &settings, RobotConfig &config)
        : robot_core::Robot(
              settings,
              Geometry{{config.chassis_drv.wheel_angles[0], config.chassis_drv.wheel_angles[1],
                        config.chassis_drv.wheel_angles[2], config.chassis_drv.wheel_angles[3]},
                       config.chassis_drv.wheel_radius,
                       config.chassis_drv.robot_radius}),
          RobotConfig(config)
    {
    }
    void init();
    void init(RobotSettings &settings);
    void sense();
    void act();
    void set_target_local_linear_vel(float x, float y, bool local = false);
};
}  // namespace devices::robot
