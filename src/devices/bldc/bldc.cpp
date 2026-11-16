#include "bldc.hpp"
#include <cstring>
#include "devices/device_manager.hpp"

#include "half_float.h"
#include "kernel/kernel.hpp"
#include "vt_linalg"

namespace devices::bldc
{

void BldcsDriver::init()
{
    kprogress_start("Awaiting BLDC initialization...");
    for (int i = 0; i < 4; i++)
    {
        HAL_Delay(100);
        kprogress_tick(25.0);
    }
    kprogress_finish("BLDC OK");

    setDriveSettings(true);
}

void BldcsDriver::setDriveSettings(bool is_verbose, bool is_idle)
{
    if (!isnan(drive_vel_p))
    {
        setDriveRegisterFloat(CANFuocoRegisterMap::PID_VELOCITY_P_RW, drive_vel_p);
        if (is_verbose)
        {
            kinfo("Set PID_VELOCITY_P_RW: %f", drive_vel_p);
        }
    }
    if (!isnan(drive_vel_i))
    {
        setDriveRegisterFloat(CANFuocoRegisterMap::PID_VELOCITY_I_RW, drive_vel_i);
        if (is_verbose)
        {
            kinfo("Set PID_VELOCITY_I_RW: %f", drive_vel_i);
        }
    }
    if (!isnan(drive_vel_limit) && !isnan(drive_vel_limit_idle))
    {
        setDriveRegisterFloat(CANFuocoRegisterMap::PID_VELOCITY_LIMIT_RW,
                              is_idle ? drive_vel_limit_idle : drive_vel_limit);
        if (is_verbose)
        {
            kinfo("Set PID_VELOCITY_LIMIT_RW: %f", drive_vel_limit);
        }
    }
}

void BldcsDriver::motorWrite(uint8_t motor_addr, uint8_t register_addr, uint8_t *data, uint8_t len)
{
    uint8_t stopped[8] = {};
    if (devices::robot_dev.control_locked()) {
        if (register_addr != MULTI_TARGET_W && register_addr != TARGET_RW) return;
        if (motor_addr == 7 && register_addr == TARGET_RW && len == sizeof(float)) {
            const float speed = devices::robot_dev.test_dribbler *
                                devices::robot_dev.dribbler_setting_to_vel;
            std::memcpy(stopped, &speed, sizeof(speed));
        }
        data = stopped;
        if (len > sizeof(stopped)) return;
    }
    uint32_t id = motor_addr << 8 | register_addr;
    can->write(id, data, len);
}

void BldcsDriver::setDriveRegisterFloat(uint8_t register_addr, float value)
{
    motorWrite(0, register_addr, (uint8_t *)&value, 4);
}

void BldcsDriver::setDriveVel(BldcsVec vel)
{
    if (devices::robot_dev.control_locked()) vel = {{0, 0, 0, 0}};
    uint8_t payload[8] = {0};

    for (size_t i = 0; i < 4; i++)
    {
        float value = -vel.vec[i];  // Positive speed should be CCW viewed from the top
        ushort value_fp16 = fp32_to_fp16(value);
        reinterpret_cast<uint16_t *>(payload)[i] = value_fp16;
    }

    motorWrite(0, MULTI_TARGET_W, payload, 8);

    kdebug("setDriveVel: %f %f %f %f", vel.vec[0], vel.vec[1], vel.vec[2], vel.vec[3]);
    current_vel = vel;
}

void BldcsDriver::getDriveVel(BldcsVec *vel)
{
    *vel = current_vel;
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
    motorWrite(7, TARGET_RW, target_speed.buffer, 4);
}

void BldcsDriver::getDribblerVel(float *vel)
{
    *vel = 0;

    kdebug("getDribblerVel: %f", *vel);
}

}  // namespace devices::bldc
