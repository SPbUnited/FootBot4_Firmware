#include "setting_broadcaster.hpp"

#include "devices/device_manager.hpp"

namespace apps::setting_broadcaster
{

void broadcast_settings()
{
    devices::bldcs_drv.broadcastNextSetting(devices::robot_dev.is_idle);
}

}  // namespace apps::setting_broadcaster
