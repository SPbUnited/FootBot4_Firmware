#pragma once

#include "devices/bldc/bldc.hpp"
#include "devices/chassis/chassis.hpp"
#include "devices/imu/imu.hpp"
#include "devices/odom/odom.hpp"

namespace devices::robot
{

enum LinearControlMode
{
    VELOCITY,
    COORDINATE,
};

enum AngleControlMode
{
    ANGLEVEL,
    ANGLEPOS,
};

enum KickerMode
{
    DISABLED = 0b000,
    KICK_FRONT = 0b100,
    KICK_UP = 0b101,
    AUTOKICK_FRONT = 0b110,
    AUTOKICK_UP = 0b111,
};

struct RobotConfig
{
    devices::odom::Odometer &odom_dev;
    devices::chassis::Chassis &chassis_drv;
    devices::bldc::BldcsDriver &bldcs_drv;
    devices::imu::BNO055 &bno055_drv;
};

struct RobotSettings
{
    float dribbler_setting_to_vel;
    float kicker_setting_to_voltage;
    float angle_kp;

    float max_linear_vel;
    float max_linear_accel;
    float max_angular_vel;
    float max_angular_accel;

    uint8_t robot_id;

    uint8_t signature;
};

uint8_t calculate_signature(RobotSettings config);

struct Robot : RobotSettings, RobotConfig
{
    Robot(RobotSettings &settings, RobotConfig &config)
        : RobotSettings(settings), RobotConfig(config)
    {
    }

    LinearControlMode linear_mode;
    AngleControlMode angle_mode;

    devices::chassis::StateVector target_vel;
    devices::chassis::StateVector target_pos;

    devices::chassis::StateVector current_vel;
    devices::chassis::StateVector current_pos;

    size_t next_waypoint;
    struct Waypoint
    {
        devices::chassis::StateVector vel;
        devices::chassis::StateVector pos;
    };
    Waypoint waypoints[16];

    KickerMode kicker_mode;
    uint8_t kicker_setting;
    uint8_t dribbler_setting;
    bool dribbler_update;

    void set_target_linear_vel(float vel_x, float vel_y);
    void set_target_angular_vel(float vel_theta);
    void set_target_angular_dpos(float dpos_theta);

    void set_max_linear_vel(float max_vel);
    void set_max_linear_accel(float max_accel);
    void set_max_angular_vel(float max_vel);
    void set_max_angular_accel(float max_accel);

    void set_dribbler_setting(uint8_t setting);
    void set_kicker_setting(uint8_t setting);
    void set_kicker_mode(KickerMode mode);

    void inc_id();
    void dec_id();

    void init();
    void init(RobotSettings &settings);
    void sense();
    void plan();
    void act();
};

}  // namespace devices::robot
