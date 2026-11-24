#include "apps/screen/screen.hpp"
#include "devices/device_manager.hpp"
#include "kernel/kernel.hpp"
#include <cstdio>
using namespace apps::screen;
void advance(uint32_t ms) { fakeTick += ms; update(); }
void click(int pin, bool longPress = false) {
    drivers::in_pins[pin].value = true; advance(10); advance(30);
    if (longPress) advance(700);
    drivers::in_pins[pin].value = false; advance(10); advance(30);
    advance(100);
}
int main() {
    using namespace devices;
    using namespace drivers;
    init();
    auto &rows = oled_drv.display.rows;
    assert(rows[0] == "PLAY MODE" && rows[1] == "ID: 07");
    unsigned playFrames = oled_drv.frames;
    click(BUTTON_SELECT);
    advance(1000);
    assert(oled_drv.frames == playFrames && eeprom::writes == 0 && !robot_dev.locked);
    for (int pin : {BUTTON_ADDR_UP, BUTTON_ADDR_DOWN, BUTTON_SELECT}) {
        in_pins[pin].value = true;
        advance(10);
        advance(30);
        advance(1000);
        assert(robot_dev.play_mode == robot::PLAY && oled_drv.frames == playFrames);
        advance(167);
        assert(oled_drv.frames == playFrames + 1);
        in_pins[pin].value = false;
        advance(100);
        assert(oled_drv.frames == playFrames + 2);
        in_pins[pin].value = true;
        advance(10);
        advance(30);
        advance(2850); // final wave frame, before entering TEST
        assert(robot_dev.play_mode == robot::PLAY && oled_drv.frames == playFrames + 3);
        advance(160);
        assert(robot_dev.play_mode == robot::TEST);
        advance(1000);
        in_pins[pin].value = false;
        advance(40);
        assert(rows[0] == "MENU 1/5" && rows[1] == "TO PLAY MODE");
        if (pin != BUTTON_SELECT) {
            robot_dev.play_mode = robot::PLAY;
            playFrames = oled_drv.frames;
        }
    }
    advance(100);
    click(BUTTON_SELECT, true); // return home for telemetry checks
    unsigned frames = oled_drv.frames;
    advance(100); assert(oled_drv.frames == frames); // unchanged image
    robot_dev.pos_global_current.x = -123456789;
    advance(100); assert(rows[1] == "X: ---");
    robot_dev.pos_global_current.y = NAN;
    advance(100); assert(rows[2] == "Y: ---");
    // Both direction keys together must not switch pages.
    in_pins[BUTTON_ADDR_UP].value = in_pins[BUTTON_ADDR_DOWN].value = true;
    advance(10); advance(30); advance(500);
    assert(rows[0].find("POS") != std::string::npos);
    in_pins[BUTTON_ADDR_UP].value = in_pins[BUTTON_ADDR_DOWN].value = false;
    advance(10); advance(30);
    click(BUTTON_SELECT); // root
    assert(rows[0] == "MENU 1/5" && rows[1] == "TO PLAY MODE");
    click(BUTTON_ADDR_DOWN);
    click(BUTTON_SELECT); // telemetry
    click(BUTTON_ADDR_UP); // wrap to drive 4
    assert(rows[0] == "Drive 4" && rows[1] == "Board: --- C" && rows[3] == "CAN feedback: NO");
    can_drive_connected[3] = true;
    can_driver_pcb_temperature[3] = 42.5f;
    can_motor_temperature[3] = 81.0f;
    advance(100);
    assert(rows[1] == "Board: 42.5 C" && rows[2] == "Motor: 81.0 C" && rows[3] == "CAN feedback: YES");
    click(BUTTON_ADDR_DOWN); click(BUTTON_ADDR_DOWN); click(BUTTON_ADDR_DOWN);
    assert(rows[1] == "Battery: --- V");
    can_battery_voltage = 23.45f;
    advance(100);
    assert(rows[1] == "Battery: 23.45 V" && rows[2] == "Kicker (V): 120.500");
    click(BUTTON_SELECT, true);
    click(BUTTON_ADDR_DOWN); click(BUTTON_SELECT); // settings
    assert(robot_dev.locked);
    robot_dev.unrelated = 99; // runtime override must not be persisted
    click(BUTTON_SELECT); click(BUTTON_ADDR_UP); // ID 8
    click(BUTTON_SELECT);
    assert(eeprom::writes == 1 && eeprom::stored.robot_id == 8 && robot_dev.robot_id == 8);
    assert(eeprom::stored.unrelated == 12.5f && eeprom::stored.signature == 123);
    assert(rows[1] == "Saved");
    advance(800);
    click(BUTTON_SELECT); click(BUTTON_SELECT); // same ID
    assert(eeprom::writes == 1);
    click(BUTTON_SELECT); click(BUTTON_ADDR_UP); click(BUTTON_SELECT, true); // cancel edit
    assert(eeprom::writes == 1 && robot_dev.robot_id == 8 && robot_dev.locked);
    click(BUTTON_SELECT, true); assert(!robot_dev.locked); // leave settings
    click(BUTTON_ADDR_DOWN); click(BUTTON_SELECT); click(BUTTON_SELECT); // version
    std::string version = rows[1]; advance(1800); assert(rows[1] != version);
    click(BUTTON_SELECT, true); click(BUTTON_ADDR_DOWN); click(BUTTON_SELECT); // reboot
    assert(robot_dev.locked);
    click(BUTTON_SELECT); assert(!robot_dev.locked && !kernel::rebooted);
    suspendForShell(); frames = oled_drv.frames;
    advance(1000); assert(oled_drv.frames == frames);
    click(BUTTON_SELECT, true); assert(oled_drv.frames > frames);
    click(BUTTON_ADDR_DOWN); click(BUTTON_SELECT); // Tests
    assert(rows[0] == "TESTS 1/3" && robot_dev.locked);
    click(BUTTON_SELECT); click(BUTTON_SELECT); // speed editor
    click(BUTTON_ADDR_UP);
    assert(robot_dev.test_dribbler == 0);
    click(BUTTON_SELECT); click(BUTTON_ADDR_DOWN); click(BUTTON_SELECT);
    assert(robot_dev.test_dribbler == 1);
    click(BUTTON_SELECT, true); assert(robot_dev.test_dribbler == 0);
    click(BUTTON_ADDR_DOWN); click(BUTTON_SELECT); click(BUTTON_SELECT); // voltage editor
    click(BUTTON_ADDR_UP);
    assert(robot_dev.test_kicker && robot_dev.test_voltage == 0);
    click(BUTTON_SELECT); assert(robot_dev.test_voltage == 1);
    click(BUTTON_ADDR_DOWN); click(BUTTON_SELECT);
    assert(robot_dev.test_kick == robot::KICK_UP && rows[0] == "KICKER 2/3");
    robot_dev.test_kick = robot::DISABLED; // driver consumed the first request
    click(BUTTON_ADDR_DOWN); click(BUTTON_SELECT);
    assert(robot_dev.test_kick == robot::KICK_FRONT && rows[0] == "KICKER 3/3");
    click(BUTTON_SELECT, true);
    assert(!robot_dev.test_kicker && robot_dev.test_voltage == 0 &&
           robot_dev.test_kick == robot::DISABLED);
    click(BUTTON_ADDR_DOWN); click(BUTTON_SELECT); // Direction
    assert(robot_dev.direction_test == -1);
    click(BUTTON_SELECT); click(BUTTON_ADDR_DOWN); click(BUTTON_SELECT); // 0 -> 359 degrees
    click(BUTTON_ADDR_DOWN); click(BUTTON_SELECT);
    assert(robot_dev.direction_test == 359);
    click(BUTTON_SELECT); assert(robot_dev.direction_test == -1);
    click(BUTTON_SELECT); click(BUTTON_SELECT, true);
    assert(robot_dev.direction_test == -1 && rows[0] == "TESTS 3/3");
    click(BUTTON_SELECT, true); assert(!robot_dev.locked);
    for (int i = 0; i < 4; ++i) click(BUTTON_ADDR_UP);
    assert(rows[0] == "MENU 1/5" && rows[1] == "TO PLAY MODE");
    frames = oled_drv.frames;
    click(BUTTON_SELECT);
    advance(100);
    assert(robot_dev.play_mode == robot::PLAY && oled_drv.frames == frames + 1);
    assert(rows[0] == "PLAY MODE" && rows[1] == "ID: 08");
    robot_dev.pos_global_current.x = 42;
    advance(1000);
    assert(oled_drv.frames == frames + 1);
    std::puts("screen integration tests passed (mock OLED/storage)");
    low_battery = true;
    advance(100);
    assert(rows[2] == "Low Battery");
    frames = oled_drv.frames;
    click(BUTTON_SELECT);
    assert(oled_drv.frames == frames && rows[2] == "Low Battery");
}
