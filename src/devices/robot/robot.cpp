#include "robot.hpp"
namespace devices::robot
{
uint8_t calculate_signature(RobotSettings config)
{
    uint8_t signature = 0;
    auto *data = reinterpret_cast<uint8_t *>(&config);
    for (size_t i = 0; i < sizeof(RobotSettings) - 2; ++i)
        signature ^= data[i];
    return signature;
}
void Robot::init()
{
    robot_core::Robot::reset();
}
void Robot::init(RobotSettings &settings)
{
    static_cast<RobotSettings &>(*this) = settings;
    robot_core::Robot::reset();
    sense();
    pos_global_target.theta = pos_global_current.theta;
}
void Robot::set_target_local_linear_vel(float x, float y, bool local)
{
    set_time(drivers::system_clock::micros());
    robot_core::Robot::set_target_local_linear_vel(x, y, local);
}
void Robot::sense()
{
    SensorFrame frame{};
    bldcs_drv.getDriveVel(&frame.wheels);
    bno055_drv.get_angles();
    frame.yaw = -bno055_drv.euler.yaw;
    frame.yaw_valid = true;
    frame.capacitor_voltage = kicker_drv.get_voltage();
    frame.ball_deep = kicker_drv.read_deep();
    frame.ball_front = kicker_drv.read_front();
    frame.shutdown_requested = !drivers::in_pins[drivers::BUTTON_TURN_OFF].read();
    frame.timestamp_us = drivers::system_clock::micros();
    robot_core::Robot::sense(frame);
    odom_dev.state = odometer.state;
}
void Robot::act()
{
    const auto output = robot_core::Robot::act();
    bldcs_drv.setDriveVel(output.wheels);
    if (output.dribbler_changed)
        bldcs_drv.setDribblerVel(output.dribbler_velocity);
    kicker_drv.apply(output.kicker);
    kicker_drv.target = kicker.target;
    kicker_drv.state = kicker.state;
    drivers::out_pins[drivers::LED_DRV2].write(sensors.ball_deep);
    drivers::out_pins[drivers::LED_DRV3].write(sensors.ball_front);
    if (output.power_off)
        drivers::bootstrap_drv.turn_off();
}
}  // namespace devices::robot
