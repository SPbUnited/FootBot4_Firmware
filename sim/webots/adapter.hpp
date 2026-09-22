#pragma once
#include <array>
#include <string>
#include <webots/InertialUnit.hpp>
#include <webots/Motor.hpp>
#include <webots/PositionSensor.hpp>
#include <webots/Receiver.hpp>
#include <webots/Robot.hpp>

#include "robot_core/protocol.hpp"
namespace simulation
{
struct Config
{
    // Entries are always fl, rl, rr, fr, irrespective of model numbering.
    std::array<std::string, 4> wheels{{"0", "1", "2", "3"}};
    std::array<double, 4> signs{{1, 1, 1, 1}};
    std::string imu = "imu", receiver, dribbler;
    double yaw_sign = 1;
};
class Adapter
{
  public:
    Adapter(webots::Robot &robot, const Config &config);
    robot_core::SensorFrame sense();
    void commands(robot_core::Robot &controller);
    void apply(const robot_core::ActuatorFrame &output);

  private:
    webots::Robot &robot_;
    Config config_;
    std::array<webots::Motor *, 4> motors_{};
    std::array<webots::PositionSensor *, 4> encoders_{};
    std::array<double, 4> previous_{};
    double previous_time_ = 0;
    bool have_previous_ = false;
    webots::InertialUnit *imu_ = nullptr;
    webots::Receiver *receiver_ = nullptr;
    webots::Motor *dribbler_ = nullptr;
    bool warned_kicker_ = false, warned_dribbler_ = false;
};
}  // namespace simulation
