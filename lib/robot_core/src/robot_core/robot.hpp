#pragma once
#include "kicker.hpp"
#include "kinematics.hpp"
#include "odometry.hpp"

namespace robot_core
{

enum LinearControlMode
{
    VELOCITY_LOCAL,
    VELOCITY_GLOBAL,
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
    AUTOKICK_MOMENTUM = 0b1000,
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

    uint8_t robot_id{};

    uint8_t signature{};
};

RobotSettings default_settings();

struct Robot : RobotSettings
{
    explicit Robot(const RobotSettings &settings, Geometry geometry = {})
        : RobotSettings(settings), kinematics(geometry), odometer({control_period_s})
    {
    }
    void reset();
    void set_time(uint32_t now)
    {
        now_us = now;
    }
    Kinematics kinematics;
    Odometer odometer;
    SensorFrame sensors{};
    Kicker kicker;
    uint32_t now_us = 0;

    LinearControlMode linear_mode{};
    AngleControlMode angle_mode{};

    StateVector vel_local_output{};
    StateVector vel_local_output_smoothed{};

    StateVector vel_global_target{};
    StateVector pos_global_target{};

    StateVector vel_global_current{};
    StateVector pos_global_current{};

    size_t next_waypoint{};
    struct Waypoint
    {
        StateVector vel;
        StateVector pos;
    };
    Waypoint waypoints[16]{};

    KickerMode kicker_mode{};
    uint8_t kicker_setting{};
    uint8_t dribbler_setting{};
    bool dribbler_update{};

    uint32_t nonidle_timer{};
    bool is_idle{};

    void update_nonidle_timer();

    void set_target_local_linear_vel(float vel_x, float vel_y, bool is_velocity_local = false);
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
    void sense(const SensorFrame &input);
    void plan();
    ActuatorFrame act();
};

}  // namespace robot_core
