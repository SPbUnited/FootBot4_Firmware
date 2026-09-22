#pragma once
#include "robot.hpp"
namespace robot_core
{
struct RobotCommand
{
    float local_x = 0, local_y = 0, angular = 0;
    bool relative_heading = false;
    uint8_t dribbler = 0, kicker = 0;
    KickerMode kicker_mode = DISABLED;
    uint32_t received_us = 0;
};
enum class DecodeStatus
{
    accepted,
    ignored,
    malformed,
    unsupported
};
DecodeStatus decode_command(const uint8_t *data, size_t length, const RobotSettings &settings,
                            uint32_t received_us, RobotCommand &command);
void apply_command(Robot &robot, const RobotCommand &command);
}  // namespace robot_core
