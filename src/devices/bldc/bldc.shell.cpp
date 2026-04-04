#include "devices/device_manager.hpp"

void setVelP(float val)
{
    devices::bldcs_drv.drive_vel_p = val;
    devices::bldcs_drv.setDriveRegisterFloat(devices::bldc::CANFuocoRegisterMap::PID_VELOCITY_P_RW,
                                             val);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), setVelP, setVelP,
                 set velocity PID P gain);

void setVelI(float val)
{
    devices::bldcs_drv.drive_vel_i = val;
    devices::bldcs_drv.setDriveRegisterFloat(devices::bldc::CANFuocoRegisterMap::PID_VELOCITY_I_RW,
                                             val);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), setVelI, setVelI,
                 set velocity PID I gain);

void setVelLimit(float val)
{
    devices::bldcs_drv.drive_vel_limit = val;
    devices::bldcs_drv.setDriveRegisterFloat(
        devices::bldc::CANFuocoRegisterMap::PID_VELOCITY_LIMIT_RW, val);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), setVelLimit,
                 setVelLimit, set velocity PID output limit);
