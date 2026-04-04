#include "setting_broadcaster.hpp"

#include "devices/device_manager.hpp"

namespace apps::setting_broadcaster
{

void broadcast_settings()
{
    devices::bldcs_drv.setDriveSettings();
}

}  // namespace apps::setting_broadcaster
