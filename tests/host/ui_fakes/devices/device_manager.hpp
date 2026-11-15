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
namespace robot {
enum { PLAY, TEST };
struct RobotSettings { float unrelated = 12.5f; uint8_t robot_id = 7, signature = 123; };
struct Robot : RobotSettings {
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
struct Kicker { double actual_voltage = 120.5; };
inline Kicker kicker_drv;
}
