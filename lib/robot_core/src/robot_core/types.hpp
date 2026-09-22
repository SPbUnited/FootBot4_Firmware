#pragma once
#include <cstddef>
#include <cstdint>
namespace robot_core
{
inline constexpr uint32_t control_period_us = 10000;
inline constexpr float control_period_s = 0.01f;
union StateVector
{
    float vec[3];
    struct
    {
        float x, y, theta;
    };
};
union WheelVector
{
    float vec[4];
    struct
    {
        float fl, rl, rr, fr;
    };
};
struct Geometry
{
    float wheel_angles[4] = {1.0472f, 2.4433f, -2.4433f, -1.0472f};
    float wheel_radius = 0.034f * 4 / 5;
    float robot_radius = 0.0765f;
};
struct SensorFrame
{
    WheelVector wheels{};  // rad/s, fl/rl/rr/fr, canonical wheel signs
    float yaw = 0;         // radians in controller coordinates
    bool yaw_valid = false;
    bool wheels_measured = false;  // MCU currently echoes commands
    bool ball_deep = false, ball_front = false;
    float capacitor_voltage = 0;
    bool shutdown_requested = false;
    uint32_t timestamp_us = 0;
};
// Semantic persistent levels; electrical active-low conversion belongs to adapters.
struct KickerOutput
{
    bool charge = false, discharge = false, straight = false, chip = false;
};
struct ActuatorFrame
{
    WheelVector wheels{};
    float dribbler_velocity = 0;
    bool dribbler_changed = false;
    KickerOutput kicker{};
    bool idle = false, power_off = false;
};
}  // namespace robot_core
