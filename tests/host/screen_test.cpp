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
    click(BUTTON_SELECT);
    advance(1000);
    assert(oled_drv.frames == 0 && eeprom::writes == 0 && !robot_dev.locked);
    for (int pin : {BUTTON_ADDR_UP, BUTTON_ADDR_DOWN, BUTTON_SELECT}) {
        in_pins[pin].value = true;
        advance(10);
        advance(30);
        advance(3000);
        assert(robot_dev.play_mode == robot::PLAY && oled_drv.frames == 0);
        advance(10);
        assert(robot_dev.play_mode == robot::TEST);
        advance(1000);
        in_pins[pin].value = false;
        advance(40);
        assert(rows[0].find("ID:07 TEST POS") == 0);
        if (pin != BUTTON_SELECT) {
            robot_dev.play_mode = robot::PLAY;
            oled_drv.frames = 0;
        }
    }
    advance(100);
    assert(rows[0].find("ID:07 TEST") == 0);
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
    assert(rows[0] == "MENU 1/4" && rows[1] == "TO PLAY MODE");
    click(BUTTON_ADDR_DOWN);
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
    for (int i = 0; i < 3; ++i) click(BUTTON_ADDR_UP);
    assert(rows[0] == "MENU 1/4" && rows[1] == "TO PLAY MODE");
    frames = oled_drv.frames;
    click(BUTTON_SELECT);
    assert(robot_dev.play_mode == robot::PLAY && oled_drv.frames == frames);
    std::puts("screen integration tests passed (mock OLED/storage)");
}
