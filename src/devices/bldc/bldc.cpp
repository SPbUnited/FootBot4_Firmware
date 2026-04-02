#include "bldc.hpp"

#include "devices/device_manager.hpp"

namespace devices::bldc
{

void BldcsDriver::setDriveVel(BldcsVec vel)
{
    kdebug("setDriveVel: %f %f %f %f", vel.vec[0], vel.vec[1], vel.vec[2], vel.vec[3]);
}
void BldcsDriver::getDriveVel(BldcsVec *vel)
{
    kdebug("getDriveVel: %f %f %f %f", vel->vec[0], vel->vec[1], vel->vec[2], vel->vec[3]);
}

}  // namespace devices::bldc