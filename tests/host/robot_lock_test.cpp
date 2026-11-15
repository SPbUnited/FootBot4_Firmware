#include "devices/robot/robot.hpp"
#include <cassert>
#include <cstdio>
int main() {
    using namespace devices;
    odom::Odometer odom;
    chassis::Chassis chassis;
    bldc::BldcsDriver bldc;
    imu::BNO055 imu;
    kicker::Kicker kicker;
    robot::RobotSettings settings{};
    settings.max_linear_vel = settings.max_angular_vel = 10;
    settings.max_linear_accel = settings.max_angular_accel = 100;
    settings.dribbler_setting_to_vel = 1;
    robot::RobotConfig config{odom, chassis, bldc, imu, kicker};
    robot::Robot r(settings, config);
    r.pos_global_current = {};
    r.clear_commands();
    r.set_target_local_linear_vel(1, 0, true);
    r.set_target_angular_vel(1);
    r.set_dribbler_setting(5);
    r.plan(); r.act();
    assert(chassis.output.x == 1 && bldc.dribbler == 5);
    r.set_control_locked(true);
    assert(r.control_locked() && bldc.dribbler == 0 && kicker.stops == 1);
    assert(nrf24_recv.discards == 1);
    r.set_target_local_linear_vel(9, 9, true);
    r.set_target_angular_dpos(3);
    r.set_dribbler_setting(9); r.set_kicker_setting(9); r.set_kicker_mode(robot::KICK_FRONT);
    assert(r.vel_global_target.x == 0 && r.kicker_setting == 0 && r.kicker_mode == robot::DISABLED);
    // Even a source writing public targets directly cannot bypass act() while locked.
    r.vel_local_output = {9, 9, 9}; r.vel_local_output_smoothed = {9, 9, 9};
    r.dribbler_setting = 9; r.kicker_mode = robot::KICK_FRONT;
    r.act();
    assert(chassis.output.x == 0 && chassis.output.theta == 0 && bldc.dribbler == 0);
    r.set_control_locked(false);
    assert(!r.control_locked() && nrf24_recv.discards == 2);
    r.plan(); r.act();
    assert(chassis.output.x == 0 && chassis.output.theta == 0);
    r.set_target_local_linear_vel(1, 0, true); r.plan(); r.act();
    assert(chassis.output.x == 1);
    std::puts("robot lock tests passed (mock devices)");
}
