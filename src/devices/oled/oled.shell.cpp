#include "devices/device_manager.hpp"
#include "apps/screen/screen.hpp"

namespace
{

int oledPrint(int argc, char *argv[])
{
    if (devices::robot_dev.play_mode != devices::robot::TEST) return -1;
    if (!devices::oled_drv.ready()) return -1;
    apps::screen::suspendForShell();
    devices::oled_drv.display.clear();
    devices::oled_drv.display.setCursor(0, 0);
    devices::oled_drv.display.printf("%d parameter(s)\n", argc);
    for (int i = 0; i < argc; i++)
    {
        devices::oled_drv.display.printf("%s\n", argv[i]);
    }

    devices::oled_drv.submit();

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), oledPrint,
                 oledPrint, write info on OLED screen);

}  // namespace
