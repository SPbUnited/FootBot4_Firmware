#include "adapter.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
namespace simulation
{
namespace
{
template <class T>
T *required(T *device, const std::string &name)
{
    if (!device)
        throw std::runtime_error("Missing Webots device: " + name);
    return device;
}
uint32_t time_us(double seconds)
{
    return static_cast<uint32_t>(static_cast<uint64_t>(std::llround(seconds * 1e6)));
}
}  // namespace
Adapter::Adapter(webots::Robot &robot, const Config &config) : robot_(robot), config_(config)
{
    for (size_t i = 0; i < 4; ++i)
    {
        const auto prefix = "wheel" + config.wheels[i];
        motors_[i] = required(robot.getMotor(prefix + "_motor"), prefix + "_motor");
        encoders_[i] = required(robot.getPositionSensor(prefix + "_encoder"), prefix + "_encoder");
        motors_[i]->setPosition(std::numeric_limits<double>::infinity());
        motors_[i]->setVelocity(0);
        encoders_[i]->enable(10);
    }
    if (!config.imu.empty())
    {
        imu_ = required(robot.getInertialUnit(config.imu), config.imu);
        imu_->enable(10);
    }
    else
        std::cerr << "No IMU configured: heading uses wheel odometry.\n";
    if (!config.receiver.empty())
    {
        receiver_ = required(robot.getReceiver(config.receiver), config.receiver);
        receiver_->enable(10);
    }
    if (!config.dribbler.empty())
    {
        dribbler_ = required(robot.getMotor(config.dribbler), config.dribbler);
        dribbler_->setPosition(std::numeric_limits<double>::infinity());
        dribbler_->setVelocity(0);
    }
    std::cerr
        << "Kicker physics unavailable; kicker control outputs are not applied to the world.\n";
}
robot_core::SensorFrame Adapter::sense()
{
    robot_core::SensorFrame frame{};
    const double now = robot_.getTime();
    const double dt = now - previous_time_;
    for (size_t i = 0; i < 4; ++i)
    {
        const double position = encoders_[i]->getValue();
        if (!std::isfinite(position))
            throw std::runtime_error("Nonfinite wheel encoder reading");
        if (have_previous_ && dt > 0)
            frame.wheels.vec[i] = float(config_.signs[i] * (position - previous_[i]) / dt);
        previous_[i] = position;
    }
    frame.wheels_measured = have_previous_;
    have_previous_ = true;
    previous_time_ = now;
    if (imu_)
    {
        frame.yaw = float(config_.yaw_sign * imu_->getRollPitchYaw()[2]);
        frame.yaw_valid = std::isfinite(frame.yaw);
    }
    frame.timestamp_us = time_us(now);
    return frame;
}
void Adapter::commands(robot_core::Robot &controller)
{
    if (!receiver_)
        return;
    while (receiver_->getQueueLength() > 0)
    {
        robot_core::RobotCommand command;
        if (robot_core::decode_command(static_cast<const uint8_t *>(receiver_->getData()),
                                       receiver_->getDataSize(), controller,
                                       time_us(robot_.getTime()),
                                       command) == robot_core::DecodeStatus::accepted)
            robot_core::apply_command(controller, command);
        receiver_->nextPacket();
    }
}
void Adapter::apply(const robot_core::ActuatorFrame &output)
{
    for (size_t i = 0; i < 4; ++i)
    {
        const double limit = motors_[i]->getMaxVelocity();
        const double target = config_.signs[i] * output.wheels.vec[i];
        motors_[i]->setVelocity(output.power_off ? 0 : std::clamp(target, -limit, limit));
    }
    if (dribbler_)
    {
        const double limit = dribbler_->getMaxVelocity();
        dribbler_->setVelocity(
            output.power_off ? 0 : std::clamp(double(output.dribbler_velocity), -limit, limit));
    }
    else if (output.dribbler_velocity != 0 && !warned_dribbler_)
    {
        std::cerr << "Dribbler command ignored: no dribbler motor configured.\n";
        warned_dribbler_ = true;
    }
    if ((output.kicker.charge || output.kicker.straight || output.kicker.chip) && !warned_kicker_)
    {
        std::cerr << "Kicker request received without simulated kicker feedback/actuator.\n";
        warned_kicker_ = true;
    }
}
}  // namespace simulation
