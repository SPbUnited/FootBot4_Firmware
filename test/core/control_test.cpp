#include <cmath>
#include <cstdlib>
#include <iostream>

#include "robot_core/protocol.hpp"
using namespace robot_core;
namespace
{
void check(bool condition, const char *message)
{
    if (!condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}
void near(float actual, float expected, const char *message, float tolerance = 1e-5f)
{
    if (std::abs(actual - expected) > tolerance || !std::isfinite(actual))
    {
        std::cerr << "actual=" << actual << " expected=" << expected << '\n';
        check(false, message);
    }
}
ActuatorFrame tick(Robot &robot, uint32_t time)
{
    SensorFrame input{};
    input.timestamp_us = time;
    robot.sense(input);
    robot.plan();
    return robot.act();
}
void motion()
{
    auto settings = default_settings();
    Robot robot(settings), other(settings);
    robot.set_target_local_linear_vel(10, 0, true);
    robot.set_target_angular_vel(30);
    tick(robot, 10000);
    near(robot.vel_local_output_smoothed.x, 1, "linear acceleration");
    near(robot.vel_local_output_smoothed.theta, 1, "angular acceleration");
    for (unsigned i = 2; i <= 20; ++i)
        tick(robot, i * 10000);
    near(robot.vel_local_output_smoothed.x, 3.2f, "linear limit");
    near(robot.vel_local_output_smoothed.theta, 10, "angular limit");
    near(other.vel_local_output_smoothed.x, 0, "independent instances");
    robot.reset();
    near(robot.vel_local_output_smoothed.x, 0, "reset smoothing");
    check(robot.kicker.state == PREPARE && robot.now_us == 0, "reset timers and kicker");
    robot.set_dribbler_setting(4);
    auto output = tick(robot, 10000);
    near(output.dribbler_velocity, 150, "dribbler scale");
    check(output.dribbler_changed, "dribbler update");
    check(!tick(robot, 20000).dribbler_changed, "dribbler update consumed");
    robot.set_time(0xfffffff0u);
    robot.set_target_local_linear_vel(1, 0, true);
    check(!tick(robot, 10000).idle, "idle wraparound");
    check(tick(robot, 10000001).idle, "idle timeout");
}
void frames()
{
    Robot robot(default_settings());
    SensorFrame input{};
    input.yaw_valid = true;
    input.yaw = 3.1f;
    robot.sense(input);
    robot.set_target_angular_dpos(0.1f);
    robot.plan();
    near(robot.vel_local_output.theta, 0.6f, "heading wrap", 1e-4f);
    input.yaw = -3.1f;
    robot.sense(input);
    robot.plan();
    near(robot.vel_local_output.theta, (6.3f - 2 * 3.14159265358979323846f) * 6,
         "heading error crosses wrap", 1e-4f);
    robot.reset();
    input.yaw = 1.57079632679f;
    robot.sense(input);
    robot.set_target_local_linear_vel(1, 0, false);
    input.yaw = 0;
    robot.sense(input);
    robot.plan();
    near(robot.vel_local_output.x, 0, "global hold x");
    near(robot.vel_local_output.y, 1, "global hold y");
    robot.set_target_local_linear_vel(1, 0, true);
    robot.plan();
    near(robot.vel_local_output.x, 1, "local target");
    input.shutdown_requested = true;
    robot.sense(input);
    robot.plan();
    check(robot.act().power_off, "shutdown forwarded to adapter");
    Kinematics k;
    auto wheels = k.wheels({1, 0, 0});
    near(wheels.fl, -std::sin(1.0472f) / 0.0272f, "front left geometry");
    near(wheels.fr, -wheels.fl, "wheel ordering/sign");
    auto body = k.body({{1, 2, 3, 4}});
    near(body.x, 0.03788364f, "legacy inverse x", 1e-5f);
    Odometer odom({0.01f});
    odom.update({1, 2, 0});
    near(odom.state.x, 0.01f, "legacy odometry x");
    near(odom.state.y, 0.02f, "legacy odometry y");
}
void packets()
{
    auto settings = default_settings();
    RobotCommand command;
    uint8_t bytes[] = {15, 0, 127, 127, 0x43, 0x42};
    check(decode_command(bytes, 6, settings, 123, command) == DecodeStatus::accepted,
          "old packet");
    near(command.local_x, 3.2f, "packet axes");
    near(command.angular, -10, "packet angular");
    check(command.dribbler == 3 && command.kicker == 4 && command.kicker_mode == KICK_FRONT,
          "packet flags");
    for (size_t length = 0; length < 6; ++length)
        check(decode_command(bytes, length, settings, 0, command) == DecodeStatus::malformed,
              "short packet");
    check(decode_command(nullptr, 6, settings, 0, command) == DecodeStatus::malformed,
          "null packet");
    bytes[0] = 14;
    check(decode_command(bytes, 6, settings, 0, command) == DecodeStatus::ignored, "other robot");
    bytes[0] = 0xff;
    bytes[1] = 0x38;
    bytes[2] = 0xb8;
    bytes[3] = 0x38;
    bytes[5] = 0;
    check(decode_command(bytes, 6, settings, 0, command) == DecodeStatus::accepted, "new packet");
    near(command.local_x, -0.01f, "fp8 negative");
    near(command.local_y, -0.01f, "fp8 positive");
    near(command.angular, -1, "fp8 angular");
    bytes[0] = 15;
    bytes[3] = 64;
    bytes[5] = 4;
    decode_command(bytes, 6, settings, 456, command);
    Robot robot(settings);
    robot.pos_global_current.theta = 1;
    apply_command(robot, command);
    near(robot.pos_global_target.theta, 1 - 1.57079632679f, "relative heading command");
    check(robot.nonidle_timer == 456, "explicit command time");
}
void kicker()
{
    Kicker kicker;
    SensorFrame input{};
    input.capacitor_voltage = 100;
    auto output = kicker.update(1, AUTOKICK_FRONT, input);
    check(kicker.state == KICK && !output.straight, "prepare");
    output = kicker.update(1, AUTOKICK_FRONT, input);
    check(!output.straight, "autokick requires ball");
    input.ball_deep = true;
    input.ball_front = true;
    kicker.update(1, AUTOKICK_FRONT, input);
    input.timestamp_us = 100;
    output = kicker.update(1, AUTOKICK_FRONT, input);
    check(output.straight && kicker.state == AFTER_KICK, "autokick");
    input.timestamp_us = 10101;
    input.capacitor_voltage = 0;
    kicker.update(1, AUTOKICK_FRONT, input);
    check(kicker.state == PREPARE, "after kick transition");
    output = kicker.update(0, DISABLED, input);
    check(!output.straight && !output.chip, "kick released");
}
void replay()
{
    Robot first(default_settings()), second(default_settings());
    auto run = [](Robot &r)
    {
        ActuatorFrame output{};
        for (unsigned i = 0; i < 200; ++i)
        {
            SensorFrame input{};
            input.timestamp_us = i * control_period_us;
            input.wheels = output.wheels;
            r.sense(input);
            RobotCommand cmd{};
            cmd.local_x = (i < 100 ? 1 : -1);
            cmd.angular = 0.3f;
            cmd.received_us = input.timestamp_us;
            apply_command(r, cmd);
            r.plan();
            output = r.act();
        }
        return output;
    };
    const auto a = run(first), b = run(second);
    for (unsigned i = 0; i < 4; ++i)
        near(a.wheels.vec[i], b.wheels.vec[i], "repeatable replay");
    first.reset();
    const auto c = run(first);
    for (unsigned i = 0; i < 4; ++i)
        near(a.wheels.vec[i], c.wheels.vec[i], "reset replay");
}
}  // namespace
int main()
{
    motion();
    frames();
    packets();
    kicker();
    replay();
    std::cout << "All core checks passed\n";
}
