#include "kernel.hpp"

#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"

namespace kernel
{

void reboot()
{
    HAL_NVIC_SystemReset();
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), reboot, reboot,
                 Reboot the robot);

}  // namespace kernel
