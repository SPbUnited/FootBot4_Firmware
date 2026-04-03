#include "bldc.hpp"

#include "devices/device_manager.hpp"
#include "half_float.h"

namespace devices::bldc
{

void BldcsDriver::motorWrite(uint8_t motor_addr, uint8_t register_addr, uint8_t *data, uint8_t len)
{
    uint32_t id = motor_addr << 8 | register_addr;
    can->write(id, data, len);
}

void BldcsDriver::setDriveRegisterFloat(uint8_t register_addr, float value)
{
    motorWrite(0, register_addr, (uint8_t *)&value, 4);
}

void BldcsDriver::setDriveVel(BldcsVec vel)
{
    uint8_t payload[8] = {0};

    for (size_t i = 0; i < 4; i++)
    {
        float value = -vel.vec[i];  // Positive speed should be CCW viewed from the top
        ushort value_fp16 = float_to_half(value);
        reinterpret_cast<uint16_t *>(payload)[i] = value_fp16;
    }

    motorWrite(0, 96, payload, 8);

    kdebug("setDriveVel: %f %f %f %f", vel.vec[0], vel.vec[1], vel.vec[2], vel.vec[3]);
}

void BldcsDriver::getDriveVel(BldcsVec *vel)
{
    kdebug("getDriveVel: %f %f %f %f", vel->vec[0], vel->vec[1], vel->vec[2], vel->vec[3]);
}

void BldcsDriver::setDribblerVel(float vel)
{
    kdebug("setDribblerVel: %f", vel);

    union
    {
        float value;
        uint8_t buffer[4];
    } target_speed;
    target_speed.value = vel;
    motorWrite(7, 0x0A, target_speed.buffer, 4);
    // drivers::can_drv.write(0x70A, target_speed.buffer, 4);
}

void BldcsDriver::getDribblerVel(float *vel)
{
    *vel = 0;

    kdebug("getDribblerVel: %f", *vel);
}

}  // namespace devices::bldc
