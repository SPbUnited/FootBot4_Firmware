#include "screen.hpp"

#include "apps/mainloop/mainloop.hpp"
#include "devices/device_manager.hpp"

#define display devices::oled_drv.display

namespace apps::screen
{

void screen()
{
    devices::chassis::StateVector state = {0, 0, 0};
    devices::odom_dev.getState(&state);

    display.rectFill(0, 0, 126, 32, false);  // Очищаем область
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.printf("Version: %s\nrobot state\nx: %6.3f |y: %6.3f\ntheta: %6.3f", FIRMWARE_VERSION,
                   state.x, state.y, state.theta);
    display.flush();
}

}  // namespace apps::screen
