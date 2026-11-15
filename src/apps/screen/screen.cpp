#include "screen.hpp"
#include "menu.hpp"
#include "devices/device_manager.hpp"
#include "kernel/kernel.hpp"
#include <cstdio>
#include <cstring>
#include <cmath>

namespace apps::screen {
namespace {
Menu menu;
Button up, down, select;
Button testEntry[3];
bool waitForRelease = false;
bool shellView = false, validFrame = false;
uint32_t lastFrame = 0, lastErrors = 0;
struct Frame { char lines[4][22] = {}; int highlight = -1; bool large = false; } previous;

void number(char *out, size_t size, double value, unsigned width, unsigned precision) {
    int n = std::snprintf(out, size, "%.*f", precision, value);
    if (!std::isfinite(value) || n < 0 || unsigned(n) > width) std::snprintf(out, size, "---");
}
void field(Frame &f, int row, const char *label, double value) {
    char buf[32]; number(buf, sizeof(buf), value, 12, 3);
    std::snprintf(f.lines[row], 22, "%s %s", label, buf);
}
void list(Frame &f, const char *title, const char *const *items, int count) {
    std::snprintf(f.lines[0], 22, "%s %d/%d", title, menu.selected + 1, count);
    for (int row = 0; row < 3 && menu.first + row < count; ++row)
        std::snprintf(f.lines[row + 1], 22, "%s", items[menu.first + row]);
    f.highlight = menu.selected - menu.first + 1;
}
Frame render(uint32_t now) {
    Frame f;
    auto &r = devices::robot_dev;
    static const char *root[] = {"Telemetry", "Settings", "System"};
    static const char *settings[] = {"Robot ID"};
    static const char *system[] = {"Firmware version", "Reboot"};
    static const char *reboot[] = {"Cancel", "Reboot"};
    switch (menu.page) {
    case Page::Root: list(f, "MENU", root, 3); break;
    case Page::Settings: list(f, "SETTINGS", settings, 1); break;
    case Page::System: list(f, "SYSTEM", system, 2); break;
    case Page::Reboot: list(f, "REBOOT?", reboot, 2); break;
    case Page::EditId:
        std::snprintf(f.lines[0], 22, "ROBOT ID");
        std::snprintf(f.lines[1], 22, "%02u", menu.editId);
        std::snprintf(f.lines[3], 22, "OK:save HOLD:back"); f.large = true; break;
    case Page::Saved: std::snprintf(f.lines[1], 22, "Saved"); break;
    case Page::Version: {
        std::snprintf(f.lines[0], 22, "FIRMWARE");
        const char *version = FIRMWARE_VERSION;
        size_t length = std::strlen(version), offset = 0;
        if (length > 21) {
            size_t travel = length - 21, phase = (now / 300) % (2 * travel + 8);
            offset = phase < 4 ? 0 : phase < travel + 4 ? phase - 4 : phase < travel + 8 ? travel : 2 * travel + 8 - phase;
        }
        std::snprintf(f.lines[1], 22, "%.21s", version + offset);
        std::snprintf(f.lines[3], 22, "HOLD:back"); break;
    }
    case Page::Home:
    case Page::Telemetry: {
        int view = menu.page == Page::Home ? menu.home : menu.telemetry;
        std::snprintf(f.lines[0], 22, "ID:%02u %s %s", r.robot_id, r.play_mode == devices::robot::PLAY ? "PLAY" : "TEST", view == 1 ? "VEL" : view == 2 ? "KICK" : "POS");
        if (view == 1) {
            field(f, 1, "Vx:", r.vel_global_current.x);
            field(f, 2, "Vy:", r.vel_global_current.y);
            field(f, 3, "W:", r.vel_global_current.theta);
        } else if (view == 2) field(f, 1, "Voltage:", devices::kicker_drv.actual_voltage);
        else {
            field(f, 1, "X:", r.pos_global_current.x);
            field(f, 2, "Y:", r.pos_global_current.y);
            char angle[32], voltage[32];
            number(angle, sizeof(angle), r.pos_global_current.theta, 7, 2);
            number(voltage, sizeof(voltage), devices::kicker_drv.actual_voltage, 6, 1);
            std::snprintf(f.lines[3], 22, "A:%s V:%s", angle, voltage);
        }
        break;
    }
    }
    return f;
}
void draw(const Frame &f) {
    auto &d = devices::oled_drv.display;
    d.clear();
    for (int row = 0; row < 4; ++row) {
        if (f.large && row == 2) continue;
        bool selected = f.highlight == row;
        if (selected) d.rectFill(0, row * 8, 128, 8, true);
        d.setTextColor(!selected);
        d.setTextSize(f.large && row == 1 ? 2 : 1);
        d.setCursor(f.large && row == 1 ? 52 : 0, row * 8);
        d.print(f.lines[row]);
    }
    d.setTextSize(1); d.setTextColor(true);
}
}
void init() {
    menu = {}; up = {}; down = {}; select = {};
    validFrame = shellView = false;
    for (auto &button : testEntry) button = {};
    waitForRelease = false;
    lastFrame = HAL_GetTick() - 100;
    update();
}
void suspendForShell() { shellView = true; validFrame = false; }
void update() {
    uint32_t now = HAL_GetTick();
    bool u = drivers::in_pins[drivers::BUTTON_ADDR_UP].read();
    bool d = drivers::in_pins[drivers::BUTTON_ADDR_DOWN].read();
    bool ok = drivers::in_pins[drivers::BUTTON_SELECT].read();
    if (devices::robot_dev.play_mode == devices::robot::PLAY) {
        up = {}; down = {}; select = {};
        const bool pressed[] = {u, d, ok};
        for (int i = 0; i < 3; ++i) {
            auto &button = testEntry[i];
            button.sample(pressed[i], now, Event::Up);
            if (pressed[i] && button.pressed && uint32_t(now - button.since) > 3000) {
                devices::robot_dev.play_mode = devices::robot::TEST;
                waitForRelease = true;
                for (auto &entry : testEntry) entry = {};
                break;
            }
        }
        return;
    }
    if (waitForRelease) {
        if (u || d || ok) return;
        waitForRelease = false;
    }
    Event ue = up.sample(u, now, Event::Up), de = down.sample(d, now, Event::Down);
    Event se = select.sample(ok, now, Event::Select);
    Event e = se != Event::None ? se : (u && d) ? Event::None : ue != Event::None ? ue : de;
    if (e != Event::None) shellView = false;
    menu.tick(now);
    Effect effect = menu.event(e, devices::robot_dev.robot_id, now);
    devices::robot_dev.set_control_locked(menu.locked());
    if (effect == Effect::SaveId) {
        devices::robot::RobotSettings saved{};
        devices::eeprom::get(0, saved);
        saved.robot_id = menu.editId;
        devices::eeprom::put(0, saved);
        devices::robot_dev.robot_id = menu.editId;
        menu.savedAt = HAL_GetTick();
    } else if (effect == Effect::Reboot) kernel::reboot();
    if (!devices::oled_drv.ready()) return;
    if (lastErrors != devices::oled_drv.transfer_errors) {
        lastErrors = devices::oled_drv.transfer_errors; validFrame = false;
    }
    if (shellView || uint32_t(now - lastFrame) < 100) return;
    lastFrame = now;
    Frame frame = render(now);
    if (validFrame && frame.highlight == previous.highlight && frame.large == previous.large &&
        std::memcmp(frame.lines, previous.lines, sizeof(frame.lines)) == 0) return;
    draw(frame);
    if (devices::oled_drv.submit()) {
        previous = frame; validFrame = true;
    }
}
}
