#include "screen.hpp"
#include "devices/oled/robot_icon.hpp"
#include "devices/oled/wave.hpp"
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
struct BootStatus
{
    BootState state = BootState::PENDING;
    uint8_t detail = 0;
};

BootStatus boot_status[static_cast<size_t>(BootComponent::COUNT)];
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
            status.state == BootState::PENDING)
        {
            warnings++;
        }
    }
    return warnings;
}

void render_boot_page(Frame &frame, int page)
{
    if (page == 0)
    {
        std::snprintf(frame.lines[0], sizeof(frame.lines[0]), "BOOT 1/5 WARN:%u", warning_count());
        std::snprintf(frame.lines[1], sizeof(frame.lines[1]), "DRIVES %u/4 IMU %s", drive_count(), state_text(BootComponent::IMU));
        std::snprintf(frame.lines[2], sizeof(frame.lines[2]), "OLED %s NRF %s", state_text(BootComponent::OLED),
                       state_text(BootComponent::RADIO));
        std::snprintf(frame.lines[3], sizeof(frame.lines[3]), "CAN %s  I2C %s", state_text(BootComponent::CAN),
                       state_text(BootComponent::I2C));
    }
    else if (page == 1)
    {
        std::snprintf(frame.lines[0], sizeof(frame.lines[0]), "MCU PERIPHERALS 2/5");
        std::snprintf(frame.lines[1], sizeof(frame.lines[1]), "U1 %s U4 %s", state_text(BootComponent::IMU_UART),
                       state_text(BootComponent::LOG_UART));
        std::snprintf(frame.lines[2], sizeof(frame.lines[2]), "SPI %s  ADC %s", state_text(BootComponent::SPI),
                       state_text(BootComponent::ANALOG_INPUT));
        std::snprintf(frame.lines[3], sizeof(frame.lines[3]), "GPIO %s", state_text(BootComponent::GPIO));
    }
    else if (page == 2)
    {
        std::snprintf(frame.lines[0], sizeof(frame.lines[0]), "ROBOT SUBSYSTEMS 3/5");
        std::snprintf(frame.lines[1], sizeof(frame.lines[1]), "KICKER %s", state_text(BootComponent::KICKER));
        std::snprintf(frame.lines[2], sizeof(frame.lines[2]), "CHASSIS %s", state_text(BootComponent::CHASSIS));
        std::snprintf(frame.lines[3], sizeof(frame.lines[3]), "ODOMETER %s", state_text(BootComponent::ODOMETER));
    }
    else if (page == 3)
    {
        std::snprintf(frame.lines[0], sizeof(frame.lines[0]), "BUS ERRORS 4/5");
        std::snprintf(frame.lines[1], sizeof(frame.lines[1]), "CAN %s %s", state_text(BootComponent::CAN),
            boot_status[static_cast<size_t>(BootComponent::CAN)].state == BootState::FAILED
                ? "HAL/filter/RX"
                : "");
        std::snprintf(frame.lines[2], sizeof(frame.lines[2]), "I2C %s %s", state_text(BootComponent::I2C),
            boot_status[static_cast<size_t>(BootComponent::I2C)].state == BootState::FAILED
                ? "HAL init"
                : "");
        std::snprintf(frame.lines[3], sizeof(frame.lines[3]), "SPI %s U1 %s", state_text(BootComponent::SPI),
                       state_text(BootComponent::IMU_UART));
    }
    else
    {
        std::snprintf(frame.lines[0], sizeof(frame.lines[0]), "DEVICE ERRORS 5/5");
        BootState imu_state = boot_status[static_cast<size_t>(BootComponent::IMU)].state;
        uint8_t imu_detail = boot_status[static_cast<size_t>(BootComponent::IMU)].detail;
        const char *imu_error = imu_detail == 1   ? "UART1 init"
                                : imu_detail == 2 ? "no reply"
                                : imu_detail == 3 ? "config/mode"
                                                  : "";
        std::snprintf(frame.lines[1], sizeof(frame.lines[1]), "IMU %s %.12s", state_text(BootComponent::IMU),
                       imu_state == BootState::FAILED ? imu_error : "");
        uint8_t mask = boot_status[static_cast<size_t>(BootComponent::DRIVES)].detail;
        std::snprintf(frame.lines[2], sizeof(frame.lines[2]), "DRIVES %u/4 mask:%X", drive_count(), mask);
        if (drive_count() < 4)
        {
            char missing_ids[24];
            format_missing_drive_ids(mask, missing_ids);
            std::snprintf(frame.lines[3], sizeof(frame.lines[3]), "%s", missing_ids);
        }
        else
        {
            std::snprintf(frame.lines[3], sizeof(frame.lines[3]), "NRF %s ADC %s", state_text(BootComponent::RADIO),
                           state_text(BootComponent::ANALOG_INPUT));
        }
    }

}

Frame render(uint32_t now) {
    Frame f;
    auto &r = devices::robot_dev;
    static const char *root[] = {"TO PLAY MODE", "Telemetry", "Settings", "System", "Tests"};
    static const char *settings[] = {"Robot ID"};
    static const char *system[] = {"Firmware version", "Reboot", "boot_page"};
    static const char *reboot[] = {"Cancel", "Reboot"};
    switch (menu.page) {
    case Page::Root: list(f, "MENU", root, 5); break;
    case Page::Tests: {
        const char *items[] = {"Dribbler", "Kicker", "Square", "Direction"};
        list(f, "TESTS", items, 4);
        break;
    }
    case Page::Square: {
        using SquareStatus = devices::robot::Robot::SquareStatus;
        const char *status = "Done";
        switch (r.square_status) {
        case SquareStatus::Idle: status = "Ready"; break;
        case SquareStatus::Running: status = "Running"; break;
        case SquareStatus::Done: break;
        case SquareStatus::NoImu: status = "IMU unavailable"; break;
        case SquareStatus::NoFeedback: status = "No wheel feedback"; break;
        case SquareStatus::Timeout: status = "Test timed out"; break;
        }
        std::snprintf(f.lines[0], 22, "SQUARE 4 x 25 cm");
        std::snprintf(f.lines[1], 22, "%s", status);
        if (r.square_status == SquareStatus::Running)
            std::snprintf(f.lines[2], 22, "Sides done: %u/4", r.square_side);
        else
            std::snprintf(f.lines[2], 22, "OK:start");
        std::snprintf(f.lines[3], 22, "HOLD:back");
        break;
    }
    case Page::Direction: {
        char angle[22];
        std::snprintf(angle, sizeof(angle), "Set angle: %d deg", int(menu.direction));
        const char *items[] = {angle, menu.driving ? "Stop motors" : "Start motors"};
        list(f, "DIRECTION 10%", items, 2);
        break;
    }
    case Page::Angle:
        std::snprintf(f.lines[0], 22, "DIRECTION ANGLE");
        std::snprintf(f.lines[1], 22, "Angle: %d deg", int(menu.draftAngle));
        std::snprintf(f.lines[3], 22, "OK:set HOLD:back");
        break;
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
    case Page::System: list(f, "SYSTEM", system, 3); break;
    case Page::BootPage: render_boot_page(f, menu.selected); break;
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
        if (view >= 4) {
            const int drive = view - 4;
            const bool connected = can_drive_connected[drive];
            char board[8], motor[8];
            number(board, sizeof(board), connected ? can_driver_pcb_temperature[drive] : NAN, 6, 1);
            number(motor, sizeof(motor), connected ? can_motor_temperature[drive] : NAN, 6, 1);
            std::snprintf(f.lines[0], 22, "Drive %d", drive + 1);
            std::snprintf(f.lines[1], 22, "Board: %s C", board);
            std::snprintf(f.lines[2], 22, "Motor: %s C", motor);
            std::snprintf(f.lines[3], 22, "CAN feedback: %s", connected ? "YES" : "NO");
            break;
        }
        if (view == 3) {
            const auto &tx = devices::nrf_feedback.txStats();
            std::snprintf(f.lines[0], 22, "Feedback TX");
            std::snprintf(f.lines[1], 22, "Radio: %s", tx.ready ? "READY" : "OFFLINE");
            std::snprintf(f.lines[2], 22, "Sent: %lu", static_cast<unsigned long>(tx.completed));
            if (tx.completed)
                std::snprintf(f.lines[3], 22, "Last TX: %lus ago",
                              static_cast<unsigned long>((now - tx.lastCompletedMs) / 1000));
            else
                std::snprintf(f.lines[3], 22, "Last TX: --");
            break;
        }
        std::snprintf(f.lines[0], 22, "ID:%02u %s %s", r.robot_id, r.play_mode == devices::robot::PLAY ? "PLAY" : "TEST", view == 1 ? "VEL" : view == 2 ? "KICK" : "POS");
        if (view == 1) {
            field(f, 1, "Vx:", r.vel_global_current.x);
            field(f, 2, "Vy:", r.vel_global_current.y);
            field(f, 3, "W:", r.vel_global_current.theta);
        } else if (view == 2) {
            const float voltage = can_battery_voltage;
            char battery[8];
            number(battery, sizeof(battery), voltage > 0 ? voltage : NAN, 6, 2);
            std::snprintf(f.lines[1], 22, "Battery: %s V", battery);
            field(f, 2, "Kicker (V):", devices::kicker_drv.actual_voltage);
            std::snprintf(f.lines[3], 22, "Deep:%s Front:%s",
                          devices::kicker_drv.read_deep() ? "YES" : "NO",
                          devices::kicker_drv.read_front() ? "YES" : "NO");
        } else {
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
    if (low_battery) {
        static bool warningShown = false;
        auto &oled = devices::oled_drv;
        if (!oled.ready() || (warningShown && lastErrors == oled.transfer_errors)) return;
        lastErrors = oled.transfer_errors;
        auto &display = oled.display;
        display.clear();
        display.setTextColor(true);
        // Inclusive horizontal spans of the lightning bolt, one per OLED row.
        constexpr uint8_t bolt[][2] = {
            {69,69}, {67,69}, {66,68}, {64,68}, {63,67},
            {62,67}, {61,66}, {60,66}, {59,65}, {58,71},
            {57,70}, {56,69}, {63,68}, {62,67}, {62,66},
            {61,65}, {61,64}, {60,63}, {60,62}, {59,60}, {58,59},
        };
        for (unsigned y = 0; y < sizeof(bolt) / sizeof(bolt[0]); ++y) {
            display.line(bolt[y][0], y, bolt[y][1], y, true);
        }
        display.setTextSize(1);
        display.setCursor(31, 22);
        display.print("Low Battery");
        warningShown = oled.submit();
        return;
    }
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
    const bool wasSquare = menu.page == Page::Square;
    Effect effect = menu.event(e, robot.robot_id, now);
    robot.direction_test = menu.driving ? menu.direction : -1;
    robot.test_dribbler = menu.spinning ? menu.speed : 0;
    robot.test_kicker = menu.kickerPage();
    robot.test_voltage = menu.voltage;
    if ((wasKicker && !robot.test_kicker) || previousVoltage != menu.voltage) {
        robot.test_kick = devices::robot::DISABLED;
        devices::kicker_drv.stop();
    }
    if (wasSquare && menu.page != Page::Square)
        robot.stop_square_test(devices::robot::Robot::SquareStatus::Idle);
    robot.set_control_locked(menu.locked());
    if (effect == Effect::StartSquare)
        robot.start_square_test();
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

void set_boot_status(BootComponent component, BootState state, uint8_t detail)
{
    if (component < BootComponent::COUNT)
        boot_status[static_cast<size_t>(component)] = {state, detail};
}
}  // namespace apps::screen
