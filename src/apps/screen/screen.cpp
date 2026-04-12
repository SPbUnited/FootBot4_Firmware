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
    display.printf("robot  %2d |%s---------------------x: %6.3f |y: %6.3f\ntheta: %6.3f",
                   devices::robot_dev.robot_id, vbuf, offset, devices::robot_dev.current_pos.x,
                   devices::robot_dev.current_pos.y, devices::robot_dev.current_pos.theta);
    display.flush();

    counter++;
    if (counter > a + a + t1 + t2)
    {
        counter = 0;
    }

    drivers::gpio_out.write(drivers::gpio::LED_DRV1,
                            drivers::gpio_in.read(drivers::gpio::BUTTON_ADDR_UP));
    drivers::gpio_out.write(drivers::gpio::LED_DRV2,
                            drivers::gpio_in.read(drivers::gpio::BUTTON_ADDR_DOWN));
    drivers::gpio_out.write(drivers::gpio::LED_DRV3,
                            drivers::gpio_in.read(drivers::gpio::BUTTON_SELECT));
    drivers::gpio_out.write(drivers::gpio::LED_STM32, true);

    static bool old_up = false;
    static bool old_down = false;
    static bool old_select = false;

    if (drivers::gpio_in.read(drivers::gpio::BUTTON_ADDR_UP))
    {
        devices::robot_dev.inc_id();
    }
    if (drivers::gpio_in.read(drivers::gpio::BUTTON_ADDR_DOWN))
    {
        devices::robot_dev.dec_id();
    }

    old_up = drivers::gpio_in.read(drivers::gpio::BUTTON_ADDR_UP);
    old_down = drivers::gpio_in.read(drivers::gpio::BUTTON_ADDR_DOWN);
    old_select = drivers::gpio_in.read(drivers::gpio::BUTTON_SELECT);
}

}  // namespace apps::screen
