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

float current_progress = 0;

void progress_start()
{
    current_progress = 0;
}

float progress_tick(float progress)
{
    current_progress += progress;
    return current_progress;
}

void progress_finish()
{
    current_progress = 0;
}

}  // namespace kernel
