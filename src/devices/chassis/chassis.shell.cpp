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

int chassisGetVelPrint(float w0, float w1, float w2, float w3)
{
    devices::bldc::BldcsVec wheel_vels = {{w0, w1, w2, w3}};
    devices::chassis::StateVector vel;
    devices::chassis_drv.getVel(&vel, wheel_vels);
    logInfo("getting vel: %f %f %f", vel.x, vel.y, vel.theta);

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), chassisGetVelPrint,
                 chassisGetVelPrint, get vel from wheel_vels);

}