#include "screen.hpp"

#include "apps/mainloop/mainloop.hpp"
#include "devices/device_manager.hpp"

#define display devices::oled_drv.display

namespace apps::screen
{

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

void screen()
{
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
    display.printf("robot  %2d%c|%s---------------------x: %6.3f |y: %6.3f\ntheta: %6.3f",
                   devices::robot_dev.robot_id, was_id_changed ? '*' : ' ', vbuf, offset,
                   devices::robot_dev.current_pos.x, devices::robot_dev.current_pos.y,
                   devices::robot_dev.current_pos.theta);

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
        display.printf(" Hold the button to\nsave config to EEPROM\n");
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

        devices::eeprom::put(0, static_cast<devices::robot::RobotConfig>(devices::robot_dev));
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
    anim = ! anim;
}

}  // namespace apps::screen
