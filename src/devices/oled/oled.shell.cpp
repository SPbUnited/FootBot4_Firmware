#include "devices/device_manager.hpp"

namespace
{

int oledPrint(int argc, char *argv[])
{
    devices::oled_drv.display.clear();
    devices::oled_drv.display.setCursor(0, 0);
    devices::oled_drv.display.printf("%d parameter(s)\n", argc);
    for (int i = 0; i < argc; i++)
    {
        devices::oled_drv.display.printf("%s\n", argv[i]);
    }

    devices::oled_drv.display.flush();

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), oledPrint,
                 oledPrint, write info on OLED screen);

}  // namespace
