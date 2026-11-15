#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
namespace drivers {
namespace system_clock { inline uint32_t micros() { return 0; } }
enum { BUTTON_TURN_OFF };
struct Pin { bool read() { return true; } };
inline Pin in_pins[1];
struct Bootstrap { void turn_off() {} };
inline Bootstrap bootstrap_drv;
}
namespace devices {
namespace bldc {
struct BldcsVec { float vec[4]; };
struct BldcsDriver {
    float dribbler = 0;
    BldcsVec drive{};
    void setDribblerVel(float v) { dribbler = v; }
    void setDriveVel(BldcsVec v) { drive = v; }
};
}
namespace chassis {
struct StateVector { float x, y, theta; };
struct Chassis {
    StateVector output{};
    void setVel(StateVector v) { output = v; }
    void getVel(StateVector *v) { *v = output; }
};
}
namespace imu {
struct BNO055 { struct { float yaw = 0; } euler; void get_angles() {} };
}
namespace odom {
struct Odometer {
    chassis::StateVector state{};
    chassis::StateVector rotate(chassis::StateVector v, float) { return v; }
    void update(chassis::StateVector) {}
    void thetaAntiWindup() {}
    void getState(chassis::StateVector *v) { *v = state; }
};
}
namespace kicker {
struct Kicker { unsigned stops = 0; void stop() { ++stops; } void update() {} };
}
struct Radio { unsigned discards = 0; void discard_pending() { ++discards; } };
inline Radio nrf24_recv;
}
