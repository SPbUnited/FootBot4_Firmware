#include "robot.hpp"

// #include "devices/device_manager.hpp"
#include "kernel/kernel.hpp"
#include "math.hpp"

namespace devices::robot
{

uint8_t calculate_signature(RobotSettings config)
{
    uint8_t signature = 0;
    uint8_t *data = reinterpret_cast<uint8_t *>(&config);

    for (size_t i = 0; i < sizeof(RobotSettings) - 2; i++)
    {
        signature ^= data[i];
    }

    return signature;
}

void Robot::set_target_local_linear_vel(float vel_local_x, float vel_local_y,
                                        bool is_velocity_local)
{
    linear_mode = is_velocity_local ? VELOCITY_LOCAL : VELOCITY_GLOBAL;

    if (linear_mode == VELOCITY_LOCAL)
    {
        vel_global_target.x = vel_local_x;
        vel_global_target.y = vel_local_y;
    }
    else
    {
        devices::chassis::StateVector vel_global_target_lin =
            odom_dev.rotate({vel_local_x, vel_local_y, 0}, pos_global_current.theta);

        vel_global_target.x = vel_global_target_lin.x;
        vel_global_target.y = vel_global_target_lin.y;
    }

    kverbose(
        "set_target_local_linear_vel(vel_local_x = %f, vel_local_y = %f): vel_global_target.x = "
        "%f, "
        "vel_global_target.y = %f",
        vel_local_x, vel_local_y, vel_global_target.x, vel_global_target.y);
}

void Robot::set_target_global_linear_vel(float vel_global_x, float vel_global_y)
{
    linear_mode = VELOCITY_GLOBAL;

    vel_global_target.x = vel_global_x;
    vel_global_target.y = vel_global_y;

    kverbose(
        "set_target_global_linear_vel(vel_global_x = %f, vel_global_y = %f): vel_global_target.x "
        "= %f, "
        "vel_global_target.y = %f",
        vel_global_x, vel_global_y, vel_global_target.x, vel_global_target.y);
}

void Robot::set_target_angular_vel(float vel_theta)
{
    angle_mode = ANGLEVEL;

    vel_global_target.theta = vel_theta;

    kverbose("set_target_angular_vel(vel_theta = %f): vel_global_target.theta = %f", vel_theta,
             vel_global_target.theta);
}

void Robot::set_target_angular_dpos(float dpos_theta)
{
    angle_mode = ANGLEPOS;

    vel_global_target.theta = 0;
    pos_global_target.theta = pos_global_current.theta + dpos_theta;

    kverbose("set_target_angular_dpos(dpos_theta = %f): pos_global_target.theta = %f", dpos_theta,
             pos_global_target.theta);
}

void Robot::set_target_angle(float theta)
{
    angle_mode = ANGLEPOS;

    vel_global_target.theta = 0;
    pos_global_target.theta = theta;

    kverbose("set_target_angle(theta = %f): pos_global_target.theta = %f", theta,
             pos_global_target.theta);
}

void Robot::set_max_linear_vel(float max_vel)
{
    this->max_linear_vel = max_vel;

    kverbose("set_max_linear_vel(max_vel = %f)", max_vel);
}

void Robot::set_max_linear_accel(float max_accel)
{
    this->max_linear_accel = max_accel;

    kverbose("set_max_linear_accel(max_accel = %f)", max_accel);
}

void Robot::set_dribbler_setting(uint8_t setting)
{
    if (dribbler_setting == setting)
    {
        return;
    }

    dribbler_setting = setting;
    dribbler_update = true;

    kverbose("set_dribbler_setting(setting = %d): dribbler_setting = %d", setting,
             dribbler_setting);
}

void Robot::set_kicker_setting(uint8_t setting)
{
    kicker_setting = setting;

    kverbose("set_kicker_setting(setting = %d): kicker_setting = %d", setting, kicker_setting);
}

void Robot::set_kicker_mode(KickerMode mode)
{
    kicker_mode = mode;

    kverbose("set_kicker_mode(mode = %d): kicker_mode = %d", mode, kicker_mode);
}

void Robot::inc_id()
{
    robot_id++;
    robot_id %= 16;
    kverbose("inc_id(): robot_id = %d", robot_id);
}

void Robot::dec_id()
{
    robot_id--;
    robot_id %= 16;
    kverbose("dec_id(): robot_id = %d", robot_id);
}

void Robot::init() {}

void Robot::init(RobotSettings &settings)
{
    static_cast<RobotSettings &>(*this) = settings;

    sense();
    pos_global_target.theta = pos_global_current.theta;
    vel_local_output_smoothed = {0, 0, 0};
}

void Robot::sense()
{
    chassis_drv.getVel(&vel_global_current);
    odom_dev.update(vel_global_current);

    bno055_drv.get_angles();
    odom_dev.state.theta = -bno055_drv.euler.yaw;
    odom_dev.thetaAntiWindup();

    odom_dev.getState(&pos_global_current);
    kicker_drv.get_voltage();
}

void Robot::plan()
{
    if (linear_mode == VELOCITY_GLOBAL)
    {
        vel_local_output.x = vel_global_target.x;
        vel_local_output.y = vel_global_target.y;

        vel_local_output = odom_dev.rotate(vel_local_output, -pos_global_current.theta);
    }
    else if (linear_mode == VELOCITY_LOCAL)
    {
        vel_local_output.x = vel_global_target.x;
        vel_local_output.y = vel_global_target.y;
    }
    else if (linear_mode == COORDINATE)
    {
        // TODO position control
    }

    if (angle_mode == ANGLEVEL)
    {
        // Just keep current target_vel
        // vel_local_output.theta = vel_global_target.theta;
        pos_global_target.theta +=
            MIN(MAX(vel_global_target.theta, -max_angular_vel * 0.8), max_angular_vel * 0.8) *
            Ts_s;

        // if (pos_global_target.theta - pos_global_current.theta > M_PI * 0.8)
        // {
        //     pos_global_target.theta = pos_global_current.theta + M_PI * 0.8;
        // }
        // else if (pos_global_target.theta - pos_global_current.theta < -M_PI * 0.8)
        // {
        //     pos_global_target.theta = pos_global_current.theta - M_PI * 0.8;
        // }
    }
    else if (angle_mode == ANGLEPOS)
    {
    }

    float error = pos_global_target.theta - pos_global_current.theta;
    while (error > M_PI)
    {
        error -= 2 * M_PI;
    }
    while (error < -M_PI)
    {
        error += 2 * M_PI;
    }
    vel_local_output.theta = error * angle_kp;  // + vel_global_target.theta;

    // kicker_drv.set_target(100);
}

void Robot::act()
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
        MIN(MAX((vel_local_output.x - vel_local_output_smoothed.x) / Ts_s, -max_linear_accel),
            max_linear_accel) *
        Ts_s;
    vel_local_output_smoothed.y +=
        MIN(MAX((vel_local_output.y - vel_local_output_smoothed.y) / Ts_s, -max_linear_accel),
            max_linear_accel) *
        Ts_s;
    vel_local_output_smoothed.theta +=
        MIN(MAX((vel_local_output.theta - vel_local_output_smoothed.theta) / Ts_s,
                -max_angular_accel),
            max_angular_accel) *
        Ts_s;

    chassis_drv.setVel(vel_local_output_smoothed);

    if (dribbler_update)
    {
        bldcs_drv.setDribblerVel(dribbler_setting * dribbler_setting_to_vel);
        dribbler_update = false;
    }

    kicker_drv.update();
}

// void setTargetDangle(float angle)
// {
//     robot_dev.set_target_angular_dpos(angle);
// }
// SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), setTargetDangle,
//                  setTargetDangle, set target angle);

}  // namespace devices::robot
