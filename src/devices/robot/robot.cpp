#include "robot.hpp"

#include "devices/device_manager.hpp"

namespace devices::robot
{

void Robot::set_target_linear_vel(float vel_x, float vel_y)
{
    linear_mode = VELOCITY;

    target_vel.x = vel_x;
    target_vel.y = vel_y;

    kverbose("set_target_linear_vel(vel_x = %f, vel_y = %f): target_vel.x = %f, target_vel.y = %f",
             vel_x, vel_y, target_vel.x, target_vel.y);
}

void Robot::set_target_angular_vel(float vel_theta)
{
    angle_mode = ANGLEVEL;

    target_vel.theta = vel_theta;

    kverbose("set_target_angular_vel(vel_theta = %f): target_vel.theta = %f", vel_theta,
             target_vel.theta);
}

void Robot::set_target_angular_dpos(float dpos_theta)
{
    angle_mode = ANGLEPOS;

    target_pos.theta = current_pos.theta + dpos_theta;

    kverbose("set_target_angular_dpos(dpos_theta = %f): target_pos.theta = %f", dpos_theta,
             target_pos.theta);
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

void Robot::init(RobotConfig config)
{
    static_cast<RobotConfig&>(*this) = config;
}

void Robot::sense()
{
    chassis_drv.getVel(&current_vel);
    odom_dev.update(current_vel);
    odom_dev.getState(&current_pos);
    kicker_drv.get_voltage();

    devices::nrf24_recv.recv();
}

void Robot::plan()
{
    if (linear_mode == VELOCITY)
    {
        // Just keep current target_vel
    }
    else if (linear_mode == COORDINATE)
    {
        // TODO position control
    }

    if (angle_mode == ANGLEVEL)
    {
        // Just keep current target_vel
    }
    else if (angle_mode == ANGLEPOS)
    {
        float error = target_pos.theta - current_pos.theta;
        target_vel.theta = error * angle_kp;
    }

    kicker_drv.set_target(50);
}

void Robot::act()
{
    float vel_abs = sqrt(target_vel.x * target_vel.x + target_vel.y * target_vel.y);
    if (vel_abs > max_linear_vel)
    {
        target_vel.x = target_vel.x / vel_abs * max_linear_vel;
        target_vel.y = target_vel.y / vel_abs * max_linear_vel;
    }

    if (fabs(target_vel.theta) > max_angular_vel)
    {
        target_vel.theta = target_vel.theta / fabs(target_vel.theta) * max_angular_vel;
    }

    chassis_drv.setVel(target_vel);

    if (dribbler_update)
    {
        bldcs_drv.setDribblerVel(dribbler_setting * dribbler_setting_to_vel);
        dribbler_update = false;
    }

    kicker_drv.update();
}

void setTargetDangle(float angle)
{
    robot_dev.set_target_angular_dpos(angle);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), setTargetDangle,
                 setTargetDangle, set target angle);

}  // namespace devices::robot
