#include "bldc.hpp"
#include <cstring>
#include "devices/device_manager.hpp"

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
    volatile uint32_t bldc_monitor_target_tx_attempts = 0;
    volatile uint32_t bldc_monitor_target_tx_successes = 0;
    volatile uint32_t bldc_monitor_target_tx_failures = 0;
    volatile float bldc_monitor_target_velocity[4] = {};
    volatile uint8_t bldc_monitor_feedback_mask = 0;
    volatile uint8_t bldc_monitor_drive_feedback[4] = {};
}

namespace devices::bldc
{

uint8_t BldcsDriver::init()
{
    if (!setMonitorSettings())
    {
        kerror("BLDC monitor setup could not be queued on CAN");
    }

    kprogress_start("Awaiting BLDC initialization...");
    const uint32_t start_ms = HAL_GetTick();
    uint32_t last_progress_ms = start_ms;
    while (position_feedback_mask != 0x0F && HAL_GetTick() - start_ms < 5000U)
    {
        broadcastNextSetting(true);
        pollFeedback();
        const uint32_t now_ms = HAL_GetTick();
        if (now_ms - last_progress_ms >= 100U)
        {
            kprogress_tick(2.0);
            last_progress_ms = now_ms;
        }
        HAL_Delay(100);
    }
    if (position_feedback_mask == 0x0F)
    {
        kprogress_finish("BLDC encoder feedback received from all drives");
    }
    else
    {
        kprogress_finish("BLDC feedback incomplete");
        kwarning("BLDC encoder feedback mask: 0x%02X (expected 0x0F)", position_feedback_mask);
    }

    setDriveSettings(true);
    bldc_monitor_feedback_mask = position_feedback_mask;
    return position_feedback_mask;
}

bool BldcsDriver::setMonitorSettings()
{
    const uint8_t monitor_registers[4] = {SHAFT_FEEDBACK_R, SUPPLY_VOLTAGE_R, PCB_TEMPERATURE_R,
                                          MOTOR_TEMPERATURE_R};
    return motorWrite(0, MONITOR_SETTINGS_RW, monitor_registers, sizeof(monitor_registers));
}

void BldcsDriver::broadcastNextSetting(bool is_idle)
{
    switch (settings_broadcast_step)
    {
        case 0:
            setMonitorSettings();
            break;
        case 1:
            if (!isnan(drive_vel_p))
            {
                setDriveRegisterFloat(CANFuocoRegisterMap::PID_VELOCITY_P_RW, drive_vel_p);
            }
            break;
        case 2:
            if (!isnan(drive_vel_i))
            {
                setDriveRegisterFloat(CANFuocoRegisterMap::PID_VELOCITY_I_RW, drive_vel_i);
            }
            break;
        case 3:
            if (!isnan(drive_vel_limit) && !isnan(drive_vel_limit_idle))
            {
                setDriveRegisterFloat(CANFuocoRegisterMap::PID_VELOCITY_LIMIT_RW,
                                      is_idle ? drive_vel_limit_idle : drive_vel_limit);
            }
            break;
        default:
            settings_broadcast_step = 0;
            return;
    }

    settings_broadcast_step = (settings_broadcast_step + 1U) % 4U;
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
    motorWrite(0, register_addr, reinterpret_cast<const uint8_t *>(&value), 4);
}

void BldcsDriver::setDriveVel(BldcsVec vel, bool direction_test)
{
    if (devices::robot_dev.control_locked() && !direction_test) vel = {{0, 0, 0, 0}};
    uint8_t payload[8] = {0};

    for (size_t i = 0; i < 4; i++)
    {
        bldc_monitor_target_velocity[i] = vel.vec[i];
        float value = -vel.vec[i];  // Positive speed should be CCW viewed from the top
        ushort value_fp16 = fp32_to_fp16(value);
        reinterpret_cast<uint16_t *>(payload)[i] = value_fp16;
    }

    ++bldc_monitor_target_tx_attempts;
    if (motorWrite(0, MULTI_TARGET_W, payload, 8))
    {
        ++bldc_monitor_target_tx_successes;
    }
    else
    {
        ++bldc_monitor_target_tx_failures;
    }

    kdebug("setDriveVel: %f %f %f %f", vel.vec[0], vel.vec[1], vel.vec[2], vel.vec[3]);
    target_vel = vel;
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
        drivers::out_pins[feedback_leds[motor_addr - 1]].toggle();
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
            position_feedback_mask |= 1U << offset;
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
    bldc_monitor_feedback_mask = position_feedback_mask;
    for (uint8_t drive = 0; drive < 4; ++drive)
    {
        bldc_monitor_drive_feedback[drive] = (position_feedback_mask >> drive) & 1U;
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

bool BldcsDriver::getDrivePosition(BldcsVec *position, uint8_t *received_mask)
{
    if (position == nullptr || received_mask == nullptr)
    {
        return false;
    }

    *position = current_pos;
    *received_mask = position_feedback_mask;
    if (*received_mask != 0x0F)
    {
        return false;
    }

    position_feedback_mask = 0;
    return true;
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
