#include "robot.hpp"

#include "devices/device_manager.hpp"
#include "kernel/kernel.hpp"
#include "math.hpp"


namespace devices::robot
{

Robot::Robot(RobotSettings &settings, RobotConfig &config) : RobotSettings(settings), RobotConfig(config)
{
    play_mode = PLAY;
    CANFuocoRegisterMapNames = std::map<CANFuocoRegisterMap, uint8_t *>{
        {ANGLE_K, reinterpret_cast<uint8_t *>(&this->angle_k)},
        {ANGLE_B, reinterpret_cast<uint8_t *>(&this->angle_b)},
    };
}

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

void Robot::update_nonidle_timer()
{
    nonidle_timer = drivers::system_clock::micros();
}

void Robot::set_target_local_linear_vel(float vel_local_x, float vel_local_y,
                                        bool is_velocity_local)
{
    if (control_locked_) return;
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

void Robot::set_target_angular_vel(float vel_theta)
{
    if (control_locked_) return;
    angle_mode = ANGLEVEL;

    vel_global_target.theta = vel_theta;

    kverbose("set_target_angular_vel(vel_theta = %f): vel_global_target.theta = %f", vel_theta,
             vel_global_target.theta);
}

void Robot::set_target_angular_dpos(float dpos_theta)
{
    if (control_locked_) return;
    angle_mode = ANGLEPOS;

    vel_global_target.theta = 0;
    pos_global_target.theta = pos_global_current.theta + dpos_theta;

    kverbose("set_target_angular_dpos(dpos_theta = %f): pos_global_target.theta = %f", dpos_theta,
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
    if (control_locked_) return;
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
    if (control_locked_) return;
    kicker_setting = setting;

    kverbose("set_kicker_setting(setting = %d): kicker_setting = %d", setting, kicker_setting);
}

void Robot::set_kicker_mode(KickerMode mode)
{
    if (control_locked_) return;
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
    devices::chassis::StateVector local_position_delta{};
    bool has_position_delta = chassis_drv.getPositionDelta(&local_position_delta);

    float heading = odom_dev.state.theta;
    imu_heading_valid = bno055_drv.get_angles();
    if (imu_heading_valid)
    {
        heading = -bno055_drv.euler.yaw;
    }
    if (has_position_delta)
    {
        odom_dev.updateLocalDelta(local_position_delta, heading);
    }
    else
    {
        odom_dev.state.theta = heading;
        odom_dev.thetaAntiWindup();
    }

    odom_dev.getState(&pos_global_current);
    // kicker_drv.get_voltage();
}

void Robot::start_square_test()
{
    if (!imu_heading_valid)
    {
        kerror("Square test blocked: IMU heading unavailable");
        return;
    }

    square_side = 0;
    square_running = true;
    linear_mode = VELOCITY_GLOBAL;
    angle_mode = ANGLEPOS;
    square_heading = pos_global_current.theta;
    pos_global_target.theta = square_heading;
    square_start = pos_global_current;
    square_turning = false;
    vel_global_target = {0.35f * cos(square_heading), 0.35f * sin(square_heading), 0.0f};
    update_nonidle_timer();
    kinfo("Square test started: side=0.25m, sides=4");
}

void Robot::update_square_test()
{
    if (!square_running)
    {
        return;
    }

    if (!imu_heading_valid)
    {
        vel_global_target = {0.0f, 0.0f, 0.0f};
        square_running = false;
        play_mode = PLAY;
        kerror("Square test stopped: IMU heading lost");
        return;
    }

    if (square_turning)
    {
        float angle_error = pos_global_target.theta - pos_global_current.theta;
        while (angle_error > M_PI)
        {
            angle_error -= 2.0f * M_PI;
        }
        while (angle_error < -M_PI)
        {
            angle_error += 2.0f * M_PI;
        }
        if (fabs(angle_error) > 0.05f)
        {
            return;
        }

        square_turning = false;
        square_start = pos_global_current;
        vel_global_target = {0.35f * cos(square_heading), 0.35f * sin(square_heading), 0.0f};
        return;
    }

    float dx = pos_global_current.x - square_start.x;
    float dy = pos_global_current.y - square_start.y;
    float distance = dx * cos(square_heading) + dy * sin(square_heading);
    if (distance < 0.25f)
    {
        return;
    }

    square_side++;
    vel_global_target = {0.0f, 0.0f, 0.0f};
    if (square_side >= 4)
    {
        square_running = false;
        play_mode = PLAY;
        kinfo("Square test complete");
        return;
    }

    square_turning = true;
    square_heading += static_cast<float>(M_PI) / 2.0f;
    pos_global_target.theta = square_heading;
    kinfo("Square test: side %u/4 complete", square_side);
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
        vel_local_output.theta = vel_global_target.theta;
        // pos_global_target.theta = pos_global_target.theta

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
        float error = pos_global_target.theta - pos_global_current.theta;
        while (error > M_PI)
        {
            error -= 2 * M_PI;
        }
        while (error < -M_PI)
        {
            error += 2 * M_PI;
        }
        vel_local_output.theta =
            error * (angle_b + sqrt(vel_local_output.x * vel_local_output.x +
                                     vel_local_output.y * vel_local_output.y) *
                                    angle_k);  // динамический коэф
    }

}

void Robot::clear_commands()
{
    linear_mode = VELOCITY_LOCAL;
    angle_mode = ANGLEVEL;
    vel_global_target = {0, 0, 0};
    pos_global_target = pos_global_current;
    vel_local_output = vel_local_output_smoothed = {0, 0, 0};
    next_waypoint = 0;
    for (auto &waypoint : waypoints) waypoint = {};
    dribbler_setting = kicker_setting = 0;
    kicker_mode = DISABLED;
    dribbler_update = true;
}

void Robot::set_control_locked(bool locked)
{
    if (control_locked_ == locked) return;
    // Keep outputs inhibited while clearing both software and hardware RX queues.
    control_locked_ = true;
    clear_commands();
    bldcs_drv.setDriveVel({{0, 0, 0, 0}});
    bldcs_drv.setDribblerVel(0);
    kicker_drv.stop();
    devices::nrf24_recv.discard_pending();
    control_locked_ = locked;
}

void Robot::act()
{
    if (control_locked_) clear_commands();
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

    is_idle = drivers::system_clock::micros() - nonidle_timer > 10000000;

    if (dribbler_update)
    {
        bldcs_drv.setDribblerVel(dribbler_setting * dribbler_setting_to_vel);
        dribbler_update = false;
    }

    kicker_drv.update();

    if (!drivers::in_pins[drivers::BUTTON_TURN_OFF].read())
    {
        drivers::bootstrap_drv.turn_off();
    }
}

// void setTargetDangle(float angle)
// {
//     robot_dev.set_target_angular_dpos(angle);
// }
// SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), setTargetDangle,
//                  setTargetDangle, set target angle);

void Robot::nrfm_callback(uint32_t m_id, uint8_t *buf, size_t len)
{
    CANFuocoRegisterMap id = static_cast<CANFuocoRegisterMap>(m_id);
    switch (id)
    {
        case CANFuocoRegisterMap::ANGLE_K:
        case CANFuocoRegisterMap::ANGLE_B:
            memcpy(CANFuocoRegisterMapNames.at(id), buf, len);
            break;
        
        default:
            break;
    }
}


}  // namespace devices::robot
