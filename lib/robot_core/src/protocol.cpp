#include "robot_core/protocol.hpp"

#include <cmath>
namespace robot_core
{
namespace
{
int signed_byte(uint8_t value)
{
    return value < 128 ? value : int(value) - 256;
}
float fp8(uint8_t value)
{
    // Preserve the legacy E4M3 conversion, including zero for subnormals.
    const unsigned exponent = (value >> 3) & 15;
    if (!exponent)
        return 0;
    const float magnitude = std::ldexp(1.0f + float(value & 7) / 8, int(exponent) - 7);
    return value & 128 ? -magnitude : magnitude;
}
}  // namespace
DecodeStatus decode_command(const uint8_t *data, size_t length, const RobotSettings &settings,
                            uint32_t received_us, RobotCommand &command)
{
    if (!data || length == 0 || length > 32)
        return DecodeStatus::malformed;
    if ((data[0] & 15) != settings.robot_id)
        return DecodeStatus::ignored;
    const unsigned type = data[0] >> 4;
    if (type != 0 && type != 15)
        return DecodeStatus::unsupported;
    if (length != 6)
        return DecodeStatus::malformed;
    if (data[0] == data[1] && data[0] == data[2] && data[0] == data[3])
        return DecodeStatus::ignored;
    const bool modern = type == 15;
    const float x =
        modern ? fp8(data[1]) * 0.01f : signed_byte(data[1]) / 127.0f * settings.max_linear_vel;
    const float y =
        modern ? fp8(data[2]) * 0.01f : signed_byte(data[2]) / 127.0f * settings.max_linear_vel;
    const unsigned flags = data[5];
    RobotCommand result{};
    result.local_x = y;
    result.local_y = -x;
    result.relative_heading = flags & 4;
    result.angular =
        result.relative_heading
            ? -signed_byte(data[3]) * 3.14159265358979323846f / 128.0f
            : -(modern ? fp8(data[3]) : signed_byte(data[3]) / 127.0f * settings.max_angular_vel);
    result.dribbler = flags & 2 ? data[4] & 15 : 0;
    result.kicker = data[4] >> 4;
    if (flags & 64)
        result.kicker_mode = KICK_FRONT;
    else if (flags & 32)
        result.kicker_mode = KICK_UP;
    else if (flags & 16)
        result.kicker_mode = flags & 8 ? AUTOKICK_MOMENTUM : AUTOKICK_FRONT;
    else if (flags & 8)
        result.kicker_mode = AUTOKICK_UP;
    result.received_us = received_us;
    command = result;
    return DecodeStatus::accepted;
}
void apply_command(Robot &robot, const RobotCommand &command)
{
    robot.set_time(command.received_us);
    robot.set_target_local_linear_vel(command.local_x, command.local_y, !command.relative_heading);
    if (command.relative_heading)
        robot.set_target_angular_dpos(command.angular);
    else
        robot.set_target_angular_vel(command.angular);
    robot.set_dribbler_setting(command.dribbler);
    robot.set_kicker_setting(command.kicker);
    robot.set_kicker_mode(command.kicker_mode);
}
}  // namespace robot_core
