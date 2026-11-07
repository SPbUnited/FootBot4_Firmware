#include "bldc.hpp"

#include <cstring>

#include "half_float.h"
#include "kernel/kernel.hpp"
#include "vt_linalg"

extern "C"
{
    volatile float can_battery_voltage = 0.0f;
    volatile float can_drive_voltage[4] = {};
    volatile float can_drive_velocity[4] = {};
    volatile float can_drive_position[4] = {};
    volatile float can_driver_pcb_temperature[4] = {};
    volatile float can_motor_temperature[4] = {};
}

namespace devices::bldc
{

void BldcsDriver::init()
{
    // const uint8_t monitor_registers[4] = {SHAFT_FEEDBACK_R, SUPPLY_VOLTAGE_R, PCB_TEMPERATURE_R,
    //                                      MOTOR_TEMPERATURE_R};
    const uint8_t monitor_registers[4] = {SHAFT_FEEDBACK_R, 0, 0, 0};
    motorWrite(0, MONITOR_SETTINGS_RW, monitor_registers, sizeof(monitor_registers));

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

bool BldcsDriver::motorWrite(uint8_t motor_addr, uint8_t register_addr, const uint8_t *data,
                             uint8_t len)
{
    uint32_t id = (static_cast<uint32_t>(motor_addr & 0x0F) << 7) | (register_addr & 0x7F);
    return can->write(id, data, len);
}

void BldcsDriver::setDriveRegisterFloat(uint8_t register_addr, float value)
{
    motorWrite(0, register_addr, reinterpret_cast<const uint8_t *>(&value), 4);
}

void BldcsDriver::setDriveVel(BldcsVec vel)
{
    uint8_t payload[8] = {0};

    for (size_t i = 0; i < 4; i++)
    {
        float value = -vel.vec[i];  // Positive speed should be CCW viewed from the top
        ushort value_fp16 = fp32_to_fp16(value);
        reinterpret_cast<uint16_t *>(payload)[i] = value_fp16;
    }

    received_feedback_mask = 0;
    motorWrite(0, MULTI_TARGET_W, payload, 8);

    kdebug("setDriveVel: %f %f %f %f", vel.vec[0], vel.vec[1], vel.vec[2], vel.vec[3]);
    current_vel = vel;
}

void BldcsDriver::pollFeedback()
{
    uint32_t response_id;
    uint8_t response[8];
    uint8_t response_len;

    for (uint8_t frames_read = 0;
         frames_read < 8 && can->read(&response_id, response, &response_len); frames_read++)
    {
        uint8_t motor_addr = (response_id >> 7) & 0x0F;
        uint8_t register_addr = response_id & 0x7F;
        if (motor_addr < 1 || motor_addr > 4)
        {
            continue;
        }

        const uint8_t offset = motor_addr - 1;
        if (register_addr == SUPPLY_VOLTAGE_R && response_len == sizeof(float))
        {
            float voltage;
            std::memcpy(&voltage, response, sizeof(voltage));
            can_battery_voltage = voltage;
            can_drive_voltage[offset] = voltage;
            received_feedback_mask |= 1U << offset;
        }
        else if (register_addr == SHAFT_FEEDBACK_R && response_len == 2 * sizeof(float))
        {
            std::memcpy(&current_vel.vec[offset], &response[0], sizeof(float));
            std::memcpy(&current_pos.vec[offset], &response[4], sizeof(float));
            can_drive_velocity[offset] = current_vel.vec[offset];
            can_drive_position[offset] = current_pos.vec[offset];
            received_feedback_mask |= 1U << offset;
        }
        else if ((register_addr == PCB_TEMPERATURE_R || register_addr == MOTOR_TEMPERATURE_R) &&
                 response_len == sizeof(float))
        {
            float temperature;
            std::memcpy(&temperature, response, sizeof(temperature));
            if (register_addr == PCB_TEMPERATURE_R)
            {
                can_driver_pcb_temperature[offset] = temperature;
            }
            else
            {
                can_motor_temperature[offset] = temperature;
            }
        }
    }
}

bool BldcsDriver::getFeedback(BldcsVec *velocity, BldcsVec *angle, uint8_t *received_mask)
{
    if (velocity == nullptr || angle == nullptr || received_mask == nullptr)
    {
        return false;
    }

    *velocity = current_vel;
    *angle = current_pos;
    *received_mask = received_feedback_mask;
    return true;
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
