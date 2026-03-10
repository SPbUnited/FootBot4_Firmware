#include "devices/device_manager.hpp"

namespace
{

int chassisPrint(float x, float y, float theta)
{
    logInfo("setting vel: %f %f %f", x, y, theta);
    devices::chassis::StateVector vel = {x, y, theta};
    devices::chassis_drv.setVel(vel);

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), chassisPrint,
                 chassisPrint, write info on chassis);

}