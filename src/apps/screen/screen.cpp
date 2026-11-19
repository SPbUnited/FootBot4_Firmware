#include "screen.hpp"
#include "devices/oled/robot_icon.hpp"
#include "devices/oled/wave.hpp"
#include "menu.hpp"
#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"
#include "drivers/system_clock/system_clock.hpp"
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
bool playFramePending = false;
unsigned shownWave = 0;
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
    static const char *root[] = {"TO PLAY MODE", "Telemetry", "Settings", "System", "Tests"};
    static const char *settings[] = {"Robot ID"};
    static const char *system[] = {"Firmware version", "Reboot"};
    static const char *reboot[] = {"Cancel", "Reboot"};
    switch (menu.page) {
    case Page::Root: list(f, "MENU", root, 5); break;
    case Page::Tests: {
        const char *items[] = {"Dribbler", "Kicker"};
        list(f, "TESTS", items, 2);
        break;
    }
    case Page::Dribbler: {
        char speed[22], value[16];
        number(value, sizeof(value), menu.speed * r.dribbler_setting_to_vel, 9, 1);
        std::snprintf(speed, sizeof(speed), "Set speed: %s", value);
        const char *items[] = {speed, menu.spinning ? "Stop dribbler" : "Start dribbler"};
        list(f, "DRIBBLER", items, 2);
        break;
    }
    case Page::Kicker: {
        char voltage[22];
        std::snprintf(voltage, sizeof(voltage), "Set voltage: %.1fV",
                      devices::kicker::voltageForLevel(menu.voltage));
        const char *items[] = {voltage, "Chip Kick", "Straight Kick"};
        list(f, "KICKER", items, 3);
        break;
    }
    case Page::Speed:
        std::snprintf(f.lines[0], 22, "DRIBBLER SPEED");
        field(f, 1, "Speed:", menu.draft * r.dribbler_setting_to_vel);
        std::snprintf(f.lines[3], 22, "OK:set HOLD:back");
        break;
    case Page::Voltage:
        std::snprintf(f.lines[0], 22, "KICKER VOLTAGE");
        std::snprintf(f.lines[1], 22, "Set: %.1f V", devices::kicker::voltageForLevel(menu.draft));
        field(f, 2, "Actual:", devices::kicker_drv.actual_voltage);
        std::snprintf(f.lines[3], 22, "OK:set HOLD:back");
        break;
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
bool submitPlayFrame(unsigned wave)
{
    auto &oled = devices::oled_drv;
    if (!oled.ready()) return false;

    Frame frame;
    std::snprintf(frame.lines[0], 22, "PLAY MODE");
    std::snprintf(frame.lines[1], 22, "ID: %02u", devices::robot_dev.robot_id);
    draw(frame);
    devices::oled::drawRobotIcon(oled.display);
    devices::oled::drawWave(oled.display, 0, wave * (devices::oled::waveWidth - 4) / devices::oled::waveFrames, 15, 5);
    return oled.submit();
}

}
void init() {
    menu = {}; up = {}; down = {}; select = {};
    validFrame = shellView = false;
    for (auto &button : testEntry) button = {};
    waitForRelease = false;
    shownWave = 0;
    playFramePending = devices::robot_dev.play_mode == devices::robot::PLAY;
    lastFrame = HAL_GetTick() - 100;
    update();
}
void suspendForShell() { shellView = true; validFrame = false; }
void update() {
    uint32_t now = HAL_GetTick();
    if (playFramePending) {
        if (uint32_t(now - lastFrame) < 100) return;
        lastFrame = now;
        if (submitPlayFrame(0)) {
            shownWave = 0;
            playFramePending = false;
            validFrame = false;
            devices::robot_dev.play_mode = devices::robot::PLAY;
        }
        return;
    }
    bool u = drivers::in_pins[drivers::BUTTON_ADDR_UP].read();
    bool d = drivers::in_pins[drivers::BUTTON_ADDR_DOWN].read();
    bool ok = drivers::in_pins[drivers::BUTTON_SELECT].read();
    if (devices::robot_dev.play_mode == devices::robot::PLAY) {
        up = {}; down = {}; select = {};
        uint32_t held = 0;
        const bool pressed[] = {u, d, ok};
        for (int i = 0; i < 3; ++i) {
            auto &button = testEntry[i];
            button.sample(pressed[i], now, Event::Up);
            if (!pressed[i]) {
                button = {};
                continue;
            }
            if (button.pressed && uint32_t(now - button.since) > held)
                held = now - button.since;
            if (held > 1000 + devices::oled::waveDurationMs) {
                devices::robot_dev.play_mode = devices::robot::TEST;
                menu.go(Page::Root);
                shownWave = 0;
                validFrame = false;
                waitForRelease = true;
                for (auto &entry : testEntry) entry = {};
                return;
            }
        }
        const unsigned wave = held > 1000 ?
            ((held - 1000) * devices::oled::waveFrames + devices::oled::waveDurationMs - 1) /
            devices::oled::waveDurationMs : 0;
        if (wave != shownWave && uint32_t(now - lastFrame) >= 100) {
            lastFrame = now;
            if (submitPlayFrame(wave)) shownWave = wave;
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
    auto &robot = devices::robot_dev;
    const bool wasKicker = menu.kickerPage();
    const auto previousVoltage = menu.voltage;
    Effect effect = menu.event(e, robot.robot_id, now);
    robot.test_dribbler = menu.spinning ? menu.speed : 0;
    robot.test_kicker = menu.kickerPage();
    robot.test_voltage = menu.voltage;
    if ((wasKicker && !robot.test_kicker) || previousVoltage != menu.voltage) {
        robot.test_kick = devices::robot::DISABLED;
        devices::kicker_drv.stop();
    }
    robot.set_control_locked(menu.locked());
    if (effect == Effect::Kick && robot.test_kick == devices::robot::DISABLED)
        robot.test_kick = menu.chip ? devices::robot::KICK_UP : devices::robot::KICK_FRONT;
    if (effect == Effect::SaveId) {
        devices::robot::RobotSettings saved{};
        devices::eeprom::get(0, saved);
        saved.robot_id = menu.editId;
        devices::eeprom::put(0, saved);
        devices::robot_dev.robot_id = menu.editId;
        menu.savedAt = HAL_GetTick();
    } else if (effect == Effect::Reboot) {
        kernel::reboot();
    } else if (effect == Effect::ToPlay) {
        playFramePending = true;
        return;
    }
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


namespace
{
struct BootStatus
{
    BootState state = BootState::PENDING;
    uint8_t detail = 0;
};

BootStatus boot_status[static_cast<size_t>(BootComponent::COUNT)];
bool display_ready = false;
bool boot_report_active = false;
bool boot_complete = false;
uint32_t boot_report_started = 0;
constexpr uint32_t report_duration_us = 5000000;
constexpr uint8_t boot_page_count = 5;
uint8_t current_boot_page = 0;

uint8_t drive_count()
{
    uint8_t count = 0;
    uint8_t mask = boot_status[static_cast<size_t>(BootComponent::DRIVES)].detail;
    for (uint8_t index = 0; index < 4; ++index)
    {
        count += (mask >> index) & 1U;
    }
    return count;
}

void format_missing_drive_ids(uint8_t feedback_mask, char output[24])
{
    size_t length = 0;
    const char prefix[] = "MISS IDs:";
    for (size_t index = 0; index < sizeof(prefix) - 1; ++index)
    {
        output[length++] = prefix[index];
    }

    bool first = true;
    for (uint8_t index = 0; index < 4; ++index)
    {
        if ((feedback_mask & (1U << index)) != 0)
        {
            continue;
        }
        if (!first)
        {
            output[length++] = ',';
        }
        output[length++] = 'M';
        output[length++] = static_cast<char>('1' + index);
        first = false;
    }
    output[length] = '\0';
}

const char *state_text(BootComponent component)
{
    switch (boot_status[static_cast<size_t>(component)].state)
    {
        case BootState::OK:
            return "OK";
        case BootState::READY:
            return "READY";
        case BootState::WARNING:
            return "WARN";
        case BootState::FAILED:
            return "FAIL";
        default:
            return "WAIT";
    }
}

uint8_t warning_count()
{
    uint8_t warnings = 0;
    for (const BootStatus &status : boot_status)
    {
        if (status.state == BootState::WARNING || status.state == BootState::FAILED ||
            (boot_complete && status.state == BootState::PENDING))
        {
            warnings++;
        }
    }
    return warnings;
}

void render_boot_page(uint8_t page, uint32_t elapsed)
{
    if (!display_ready)
    {
        return;
    }

    display.clear();
    display.setCursor(0, 0);
    display.setTextSize(1);
    if (page == 0)
    {
        display.printf("BOOT %s WARN:%u\n", boot_complete ? "DONE" : "RUN", warning_count());
        display.printf("DRIVES %u/4 IMU %s\n", drive_count(), state_text(BootComponent::IMU));
        display.printf("OLED %s NRF %s\n", state_text(BootComponent::OLED),
                       state_text(BootComponent::RADIO));
        display.printf("CAN %s  I2C %s\n", state_text(BootComponent::CAN),
                       state_text(BootComponent::I2C));
    }
    else if (page == 1)
    {
        display.printf("MCU PERIPHERALS\n");
        display.printf("U1 %s U4 %s\n", state_text(BootComponent::IMU_UART),
                       state_text(BootComponent::LOG_UART));
        display.printf("SPI %s  ADC %s\n", state_text(BootComponent::SPI),
                       state_text(BootComponent::ANALOG_INPUT));
        display.printf("GPIO %s\n", state_text(BootComponent::GPIO));
    }
    else if (page == 2)
    {
        display.printf("ROBOT SUBSYSTEMS\n");
        display.printf("KICKER %s\n", state_text(BootComponent::KICKER));
        display.printf("CHASSIS %s\n", state_text(BootComponent::CHASSIS));
        display.printf("ODOMETER %s\n", state_text(BootComponent::ODOMETER));
    }
    else if (page == 3)
    {
        display.printf("BUS ERRORS\n");
        display.printf(
            "CAN %s %s\n", state_text(BootComponent::CAN),
            boot_status[static_cast<size_t>(BootComponent::CAN)].state == BootState::FAILED
                ? "HAL/filter/RX"
                : "");
        display.printf(
            "I2C %s %s\n", state_text(BootComponent::I2C),
            boot_status[static_cast<size_t>(BootComponent::I2C)].state == BootState::FAILED
                ? "HAL init"
                : "");
        display.printf("SPI %s U1 %s\n", state_text(BootComponent::SPI),
                       state_text(BootComponent::IMU_UART));
    }
    else
    {
        display.printf("DEVICE ERRORS\n");
        BootState imu_state = boot_status[static_cast<size_t>(BootComponent::IMU)].state;
        uint8_t imu_detail = boot_status[static_cast<size_t>(BootComponent::IMU)].detail;
        const char *imu_error = imu_detail == 1   ? "UART1 init"
                                : imu_detail == 2 ? "no UART reply"
                                : imu_detail == 3 ? "config/mode fail"
                                                  : "";
        display.printf("IMU %s %s\n", state_text(BootComponent::IMU),
                       imu_state == BootState::FAILED ? imu_error : "");
        uint8_t mask = boot_status[static_cast<size_t>(BootComponent::DRIVES)].detail;
        display.printf("DRIVES %u/4 mask:%X\n", drive_count(), mask);
        if (drive_count() < 4)
        {
            char missing_ids[24];
            format_missing_drive_ids(mask, missing_ids);
            display.printf("%s\n", missing_ids);
        }
        else
        {
            display.printf("NRF %s ADC %s\n", state_text(BootComponent::RADIO),
                           state_text(BootComponent::ANALOG_INPUT));
        }
    }

    display.setCursor(108, 0);
    display.printf("%u/%u", page + 1, boot_page_count);
    uint32_t remaining = elapsed >= report_duration_us ? 0 : report_duration_us - elapsed;
    int bar_width = static_cast<int>((remaining * 12U) / report_duration_us);
    display.rect(114, 25, 13, 7, true);
    display.rectFill(115, 26, bar_width, 5, true);
    display.flush();
}
}  // namespace

void begin_boot_report(bool is_display_ready)
{
    display_ready = is_display_ready;
    boot_report_active = true;
    boot_complete = false;
    boot_report_started = drivers::system_clock::micros();
    for (BootStatus &status : boot_status)
    {
        status = {};
    }
    set_boot_status(BootComponent::OLED, display_ready ? BootState::OK : BootState::FAILED);
    current_boot_page = 0;
    render_boot_page(current_boot_page, 0);
}

void set_boot_status(BootComponent component, BootState state, uint8_t detail)
{
    if (component >= BootComponent::COUNT)
    {
        return;
    }
    BootStatus &status = boot_status[static_cast<size_t>(component)];
    status.state = state;
    status.detail = detail;
    render_boot_page(current_boot_page, drivers::system_clock::micros() - boot_report_started);
}

void finish_boot_report()
{
    boot_complete = true;
    boot_report_started = drivers::system_clock::micros();
    current_boot_page = 0;
    render_boot_page(current_boot_page, 0);
}

void screen()
{
    if (!display_ready)
    {
        return;
    }

    if (boot_report_active)
    {
        static bool previous_up = false;
        static bool previous_down = false;
        static bool previous_select = false;
        bool up = drivers::in_pins[drivers::BUTTON_ADDR_UP].read();
        bool down = drivers::in_pins[drivers::BUTTON_ADDR_DOWN].read();
        bool select = drivers::in_pins[drivers::BUTTON_SELECT].read();
        bool up_pressed = up && !previous_up;
        bool down_pressed = down && !previous_down;
        bool any_pressed = up_pressed || down_pressed || (select && !previous_select);

        previous_up = up;
        previous_down = down;
        previous_select = select;

        if (up_pressed)
        {
            current_boot_page = (current_boot_page + 1) % boot_page_count;
        }
        else if (down_pressed)
        {
            current_boot_page = (current_boot_page + boot_page_count - 1) % boot_page_count;
        }
        if (any_pressed)
        {
            boot_report_started = drivers::system_clock::micros();
        }

        uint32_t elapsed = drivers::system_clock::micros() - boot_report_started;
        if (elapsed < report_duration_us)
        {
            render_boot_page(current_boot_page, elapsed);
            return;
        }
        boot_report_active = false;
    }
    // static uint32_t last_click = 0;
    // static bool is_updating = true;

    static bool was_id_changed = false;

    static int counter = 0;
    int version_len = strlen(FIRMWARE_VERSION);
    int version_field_width = 10;
    char vbuf[version_field_width + 1] = {' '};

    // https://www.desmos.com/calculator/goy31sxinl
    int t2 = 10;
    int a = version_len > version_field_width ? version_len - version_field_width : 0;
    int t1 = t2 - a;

    int A = MIN(MAX(counter - (t1 + a), 0), a);
    int B = MIN(MAX(counter - (t2 + t2 + a), 0), a);

    int offset = A - B;

    strncpy(vbuf, FIRMWARE_VERSION + offset, version_field_width);
    if (version_len < version_field_width)
    {
        for (size_t i = version_len; i < version_field_width; i++)
        {
            vbuf[i] = ' ';
        }
    }
    vbuf[version_field_width] = '\0';

    display.rectFill(0, 0, 126, 32, false);  // Очищаем область
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.printf(
        "robot  %2d%c|%s---------------------x: %6.3f |y: %6.3f\ntheta: %6.3f v: %6.1f",
        devices::robot_dev.robot_id, was_id_changed ? '*' : ' ', vbuf, offset,
        devices::robot_dev.pos_global_current.x, devices::robot_dev.pos_global_current.y,
        devices::robot_dev.pos_global_current.theta, devices::kicker_drv.actual_voltage);

    counter++;
    if (counter > a + a + t1 + t2)
    {
        counter = 0;
    }

    drivers::out_pins[drivers::LED_DRV1].write(drivers::in_pins[drivers::BUTTON_ADDR_UP].read());
    drivers::out_pins[drivers::LED_DRV2].write(drivers::in_pins[drivers::BUTTON_ADDR_DOWN].read());
    drivers::out_pins[drivers::LED_DRV3].write(drivers::in_pins[drivers::BUTTON_SELECT].read());
    drivers::out_pins[drivers::LED_STM32].toggle();

    static bool old_up = false;
    static bool old_down = false;
    static bool old_select = false;

    if (drivers::in_pins[drivers::BUTTON_ADDR_UP].read())
    {
        was_id_changed = true;
        devices::robot_dev.inc_id();
    }
    if (drivers::in_pins[drivers::BUTTON_ADDR_DOWN].read())
    {
        was_id_changed = true;
        devices::robot_dev.dec_id();
    }

    static int save_progress = 0;

    if (drivers::in_pins[drivers::BUTTON_SELECT].read())
    {
        save_progress++;
    }
    else
    {
        save_progress = 0;
    }

    if (save_progress != 0)
    {
        display.clear();
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.printf("    Hold to save    \n settings to EEPROM \n");
        display.printf("     [");
        for (int i = 0; i < 10; i++)
        {
            display.printf("%c", i <= save_progress ? '#' : ' ');
        }
        display.printf("]");
    }

    if (save_progress > 10)
    {
        kinfo("Saving robot config to EEPROM");
        display.clear();
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.printf("Saving robot\nconfig to EEPROM and\nrebooting...");
        display.flush();

        devices::eeprom::put(0, static_cast<devices::robot::RobotSettings>(devices::robot_dev));
        kernel::reboot();
    }

    old_up = drivers::in_pins[drivers::BUTTON_ADDR_UP].read();
    old_down = drivers::in_pins[drivers::BUTTON_ADDR_DOWN].read();
    old_select = drivers::in_pins[drivers::BUTTON_SELECT].read();

    // if (old_up || old_down || old_select)
    // {
    //     last_click = drivers::system_clock::micros();
    // }

    // if (drivers::system_clock::micros() - last_click < 10000000)
    // {
    //     display.flush();
    //     is_updating = true;
    // }
    // else
    // {
    //     if (is_updating)
    //     {
    //         display.clear();
    //         display.flush();
    //         is_updating = false;
    //     }
    // }
    display.flush();
    // anim = ! anim;
}


}
