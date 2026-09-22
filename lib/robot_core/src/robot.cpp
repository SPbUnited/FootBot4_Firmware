#include "robot_core/robot.hpp"

#include <algorithm>
#include <cmath>
#define MIN(a, b) std::min((a), (b))
#define MAX(a, b) std::max((a), (b))
namespace robot_core
{
constexpr double pi = 3.14159265358979323846;

RobotSettings default_settings()
{
    return {600.0f / 16, 200.0f / 16, 6, 3.2f, 100, 10, 100, 15, 0};
}
void Robot::reset()
{
    const auto settings = static_cast<const RobotSettings &>(*this);
    const auto geometry = kinematics;
    *this = Robot(settings);
    kinematics = geometry;
}
void Robot::update_nonidle_timer()
{
    nonidle_timer = now_us;
}

void Robot::set_target_local_linear_vel(float vel_local_x, float vel_local_y,
                                        bool is_velocity_local)
{
    if (vel_local_x != 0.0 || vel_local_y != 0.0)
    {
        update_nonidle_timer();
    }

    linear_mode = is_velocity_local ? VELOCITY_LOCAL : VELOCITY_GLOBAL;

    if (linear_mode == VELOCITY_LOCAL)
    {
        vel_global_target.x = vel_local_x;
        vel_global_target.y = vel_local_y;
    }
    else
    {
        StateVector vel_global_target_lin =
            odometer.rotate({vel_local_x, vel_local_y, 0}, pos_global_current.theta);

        vel_global_target.x = vel_global_target_lin.x;
        vel_global_target.y = vel_global_target_lin.y;
    }
}

void Robot::set_target_angular_vel(float vel_theta)
{
    angle_mode = ANGLEVEL;

    vel_global_target.theta = vel_theta;
}

void Robot::set_target_angular_dpos(float dpos_theta)
{
    angle_mode = ANGLEPOS;

    vel_global_target.theta = 0;
    pos_global_target.theta = pos_global_current.theta + dpos_theta;
}

void Robot::set_max_linear_vel(float max_vel)
{
    this->max_linear_vel = max_vel;
}

void Robot::set_max_linear_accel(float max_accel)
{
    this->max_linear_accel = max_accel;
}

void Robot::set_dribbler_setting(uint8_t setting)
{
    if (dribbler_setting == setting)
    {
        return;
    }

    dribbler_setting = setting;
    dribbler_update = true;
}

void Robot::set_kicker_setting(uint8_t setting)
{
    kicker_setting = setting;
}

void Robot::set_kicker_mode(KickerMode mode)
{
    kicker_mode = mode;
}

void Robot::inc_id()
{
    robot_id++;
    robot_id %= 16;
}

void Robot::dec_id()
{
    robot_id--;
    robot_id %= 16;
}

void Robot::init()
{
    reset();
}

void Robot::init(RobotSettings &settings)
{
    static_cast<RobotSettings &>(*this) = settings;

    reset();
}

void Robot::sense(const SensorFrame &input)
{
    sensors = input;
    now_us = input.timestamp_us;
    vel_global_current = kinematics.body(input.wheels);
    odometer.update(vel_global_current);
    if (input.yaw_valid)
        odometer.state.theta = input.yaw;
    odometer.thetaAntiWindup();
    odometer.getState(&pos_global_current);
}

void Robot::plan()
{
    if (linear_mode == VELOCITY_GLOBAL)
    {
        vel_local_output.x = vel_global_target.x;
        vel_local_output.y = vel_global_target.y;

        vel_local_output = odometer.rotate(vel_local_output, -pos_global_current.theta);
    }
    else if (linear_mode == VELOCITY_LOCAL)
    {
        vel_local_output.x = vel_global_target.x;
        vel_local_output.y = vel_global_target.y;
    }
    else if (linear_mode == COORDINATE)
    {
    }

    if (angle_mode == ANGLEVEL)
    {
        vel_local_output.theta = vel_global_target.theta;
    }
    else if (angle_mode == ANGLEPOS)
    {
        float error = pos_global_target.theta - pos_global_current.theta;
        while (error > pi)
        {
            error -= 2 * pi;
        }
        while (error < -pi)
        {
            error += 2 * pi;
        }
        vel_local_output.theta =
            error * (angle_kp + sqrt(vel_local_output.x * vel_local_output.x +
                                     vel_local_output.y * vel_local_output.y) *
                                    0.001);  // динамический коэф
    }
}

ActuatorFrame Robot::act()
{
    float vel_abs =
        sqrt(vel_local_output.x * vel_local_output.x + vel_local_output.y * vel_local_output.y);
    if (vel_abs > max_linear_vel)
    {
        vel_local_output.x = vel_local_output.x / vel_abs * max_linear_vel;
        vel_local_output.y = vel_local_output.y / vel_abs * max_linear_vel;
    }

    if (fabs(vel_local_output.theta) > max_angular_vel)
    {
        vel_local_output.theta =
            vel_local_output.theta / fabs(vel_local_output.theta) * max_angular_vel;
    }

    vel_local_output_smoothed.x +=
        MIN(MAX((vel_local_output.x - vel_local_output_smoothed.x) / control_period_s,
                -max_linear_accel),
            max_linear_accel) *
        control_period_s;
    vel_local_output_smoothed.y +=
        MIN(MAX((vel_local_output.y - vel_local_output_smoothed.y) / control_period_s,
                -max_linear_accel),
            max_linear_accel) *
        control_period_s;
    vel_local_output_smoothed.theta +=
        MIN(MAX((vel_local_output.theta - vel_local_output_smoothed.theta) / control_period_s,
                -max_angular_accel),
            max_angular_accel) *
        control_period_s;

    ActuatorFrame output{};
    output.wheels = kinematics.wheels(vel_local_output_smoothed);
    is_idle = uint32_t(now_us - nonidle_timer) > 10000000;
    output.idle = is_idle;
    output.dribbler_velocity = dribbler_setting * dribbler_setting_to_vel;
    output.dribbler_changed = dribbler_update;
    dribbler_update = false;
    output.power_off = sensors.shutdown_requested;
    output.kicker = kicker.update(kicker_setting, kicker_mode, sensors);
    return output;
}
void Robot::set_max_angular_vel(float value)
{
    max_angular_vel = value;
}
void Robot::set_max_angular_accel(float value)
{
    max_angular_accel = value;
}
}  // namespace robot_core
