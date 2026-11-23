#pragma once
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
inline uint32_t fakeTick = 0;
inline uint32_t HAL_GetTick() { return fakeTick; }
namespace drivers {
enum { BUTTON_ADDR_UP, BUTTON_ADDR_DOWN, BUTTON_SELECT };
struct Pin { bool value = false; bool read() const { return value; } };
inline Pin in_pins[3];
}
namespace devices {
struct Feedback {
    struct Stats { bool ready; uint32_t completed, lastCompletedMs; };
    Stats txStats() const { return {}; }
};
inline Feedback nrf_feedback;
namespace robot {
enum { PLAY, TEST };
enum { DISABLED, KICK_UP, KICK_FRONT };
struct RobotSettings { float unrelated = 12.5f; uint8_t robot_id = 7, signature = 123; };
struct Robot : RobotSettings {
    float dribbler_setting_to_vel = 37.5f;
    int test_direction_deg = -1;  // Negative while the direction test is stopped.
    uint8_t test_dribbler = 0, test_voltage = 0;
    bool test_kicker = false;
    int test_kick = DISABLED;
    enum class SquareStatus { Idle, Running, Done, NoImu, NoFeedback, Timeout };
    SquareStatus square_status = SquareStatus::Idle;
    unsigned square_side = 0;
    void start_square_test() { square_status = SquareStatus::Running; }
    void stop_square_test(SquareStatus status) { square_status = status; }
    bool locked = false;
    int play_mode = PLAY;
    struct Vec { float x = 0, y = 0, theta = 0; } pos_global_current, vel_global_current;
    void set_control_locked(bool v) { locked = v; }
};
}
inline robot::Robot robot_dev;
namespace eeprom {
inline robot::RobotSettings stored;
inline unsigned writes = 0;
inline void get(int, robot::RobotSettings &s) { s = stored; }
inline void put(int, robot::RobotSettings s) { stored = s; ++writes; }
}
struct Display {
    std::string rows[4];
    int x = 0, y = 0, scale = 1;
    void clear() { for (auto &s : rows) s.clear(); }
    void rectFill(int x, int y, int w, int h, bool) { assert(x >= 0 && y >= 0 && x + w <= 128 && y + h <= 32); }
    void line(int x0, int y0, int x1, int y1, bool) {
        assert(x0 >= 0 && x0 < 128 && x1 >= 0 && x1 < 128);
        assert(y0 >= 0 && y0 < 32 && y1 >= 0 && y1 < 32);
    }
    void setTextColor(bool) {}
    void setTextSize(int s) { scale = s; }
    void setCursor(int a, int b) { x = a; y = b; }
    void print(const char *s) {
        assert(x + std::strlen(s) * 6 * scale <= 128);
        assert(y >= 0 && y + 8 * scale <= 32);
        rows[y / 8] = s;
    }
};
struct Oled {
    Display display;
    uint32_t transfer_errors = 0;
    unsigned frames = 0;
    bool ready() { return true; }
    bool submit() { ++frames; return true; }
};
inline Oled oled_drv;
namespace kicker { inline double voltageForLevel(uint8_t level) { return level ? 100 + (level - 1) * 230.0 / 14 : 0; } }
struct Kicker {
    double actual_voltage = 120.5;
    bool read_deep() { return false; }
    bool read_front() { return false; }
    void stop() {}
};
inline Kicker kicker_drv;
}
