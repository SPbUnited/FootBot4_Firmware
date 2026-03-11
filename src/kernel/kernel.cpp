#include "kernel.hpp"

#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"

namespace kernel
{

void error(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    drivers::uart4.printf("[ERROR]: ");
    drivers::uart4.vprintf(msg, args);
    va_end(args);

    while (1)
        ;
}

void reboot()
{
    HAL_NVIC_SystemReset();
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), reboot, reboot,
                 Reboot the robot);

}  // namespace kernel
