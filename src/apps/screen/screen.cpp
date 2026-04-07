#include "screen.hpp"

#include "apps/mainloop/mainloop.hpp"
#include "devices/device_manager.hpp"

#define display devices::oled_drv.display

namespace apps::screen
{

void screen()
{
    display.rectFill(0, 0, 126, 32, false);  // Очищаем область
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.printf("robot  %2d | v%s\n---------------------x: %6.3f |y: %6.3f\ntheta: %6.3f",
                   devices::robot_dev.robot_id, FIRMWARE_VERSION, devices::robot_dev.current_pos.x,
                   devices::robot_dev.current_pos.y, devices::robot_dev.current_pos.theta);
    display.flush();
}

}  // namespace apps::screen
