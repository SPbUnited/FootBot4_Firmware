#include "uart_control.hpp"

#include <cstdio>
#include <cstring>

#include "kernel/kernel.hpp"
#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"

namespace devices::uart_control
{

uint32_t UartControl::m_prevTime_ms = 0;
uint32_t UartControl::m_packetsReceived = 0;
uint32_t UartControl::m_packetsReceived_1s = 0;

uint8_t UartControl::m_rx_acc_buf[UartControl::kRxBufSize];
uint32_t UartControl::m_rx_acc_len = 0;

UartControl::UartControl(UartControlConfig &uart_control_config) : UartControlConfig(uart_control_config) {}

static uint8_t compute_checksum(const uint8_t *data, size_t length)
{
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; ++i)
    {
        checksum ^= data[i];
    }
    return checksum;
}

void UartControl::recv_irq()
{
    while (m_rx_acc_len < kRxBufSize &&
           uart_instance.getc(reinterpret_cast<char *>(&m_rx_acc_buf[m_rx_acc_len])) == HAL_OK)
    {
        m_rx_acc_len++;
    }

    uint32_t parse_idx = 0;
    while (m_rx_acc_len - parse_idx >= kRxFrameLen)
    {
        const uint8_t *frame = &m_rx_acc_buf[parse_idx];
        if (frame[0] == kSettingsSyncByte)
        {
            if (m_rx_acc_len - parse_idx < kSettingsFrameLen)
            {
                break;  // wait for the rest of the frame
            }
            if (compute_checksum(frame, kSettingsFrameLen - 1) == frame[kSettingsFrameLen - 1])
            {
                memcpy(&m_settings_pkt, frame + 1, sizeof(SettingsPacket));
                m_has_settings = true;
                parse_idx += kSettingsFrameLen;
                continue;
            }
        }
        if (frame[0] != kSyncByte || compute_checksum(frame, kRxFrameLen - 1) != frame[kRxFrameLen - 1])
        {
            parse_idx++;
            continue;
        }

        memcpy(&m_last_pkt, frame + 1, sizeof(RxPacket));
        m_has_pkt = true;
        m_lastPacketTime = HAL_GetTick();

        m_packetsReceived++;
        if (m_packetsReceived % kLedTogglePackets == 0)
        {
            dts_led.toggle();
        }
        uint32_t time_ms = HAL_GetTick();
        if (time_ms - m_prevTime_ms > 1000)
        {
            m_prevTime_ms = time_ms;
            m_packetsReceived_1s = m_packetsReceived;
            m_packetsReceived = 0;
        }

        parse_idx += kRxFrameLen;
    }

    if (parse_idx > 0)
    {
        m_rx_acc_len -= parse_idx;
        if (m_rx_acc_len > 0)
        {
            memmove(m_rx_acc_buf, m_rx_acc_buf + parse_idx, m_rx_acc_len);
        }
    }
}

// KickerMode values on the wire follow robot/control.proto, not robot::KickerMode
static robot::KickerMode to_kicker_mode(uint8_t proto_mode)
{
    switch (proto_mode)
    {
        case 1:
            return robot::KICK_FRONT;
        case 2:
            return robot::KICK_UP;
        case 3:
            return robot::AUTOKICK_FRONT;
        case 4:
            return robot::AUTOKICK_UP;
        case 5:
            return robot::AUTOKICK_MOMENTUM;
        default:
            return robot::DISABLED;
    }
}

int UartControl::fetch()
{
    // The screen app changes robot_dev.robot_id when the id buttons are pressed
    if (robot_dev.robot_id != m_known_id)
    {
        m_id_from_buttons = true;
        m_known_id = robot_dev.robot_id;
    }

    if (m_has_settings)
    {
        m_has_settings = false;
        apply_settings(m_settings_pkt);
    }

    if (m_has_pkt)
    {
        m_has_pkt = false;
        m_link_active = true;
        sync_robot_id(m_last_pkt.robot_id);
        process_command(m_last_pkt);
    }
    else if (m_link_active && HAL_GetTick() - m_lastPacketTime > kLinkTimeout_ms)
    {
        m_link_active = false;
        stop_robot();
    }

    return 0;
}

// The id lives in the RPi hostname (fb4-NN); the STM32 starts without one and takes the RPi's.
// Both sides send their id in every frame. An id changed with the buttons wins: it is kept
// until the RPi takes it (the RPi adopts our id after it has differed for a while and renames
// itself). Otherwise the RPi's id wins.
void UartControl::sync_robot_id(uint8_t rpi_id)
{
    uint8_t own_id = robot_dev.robot_id;
    if (rpi_id > kMaxRobotId)
    {
        return;
    }
    if (rpi_id == own_id)
    {
        m_id_from_buttons = false;
        return;
    }
    if (m_id_from_buttons)
    {
        return;
    }
    kinfo("Robot id %d -> %d from the RPi", own_id, rpi_id);
    robot_dev.robot_id = rpi_id;
    m_known_id = rpi_id;
}

void UartControl::stop_robot()
{
    robot_dev.set_target_local_linear_vel(0, 0, true);
    robot_dev.set_target_angular_vel(0);
    robot_dev.set_dribbler_setting(0);
    robot_dev.set_kicker_mode(robot::DISABLED);
    drivers::buzzer_drv.set(false);
}

void UartControl::process_command(const RxPacket &pkt)
{
    float vel_x = fp16_to_fp32(pkt.vel_x);
    float vel_y = fp16_to_fp32(pkt.vel_y);
    float w = fp16_to_fp32(pkt.w);
    float dribbler = fp16_to_fp32(pkt.dribbler);
    float kicker = fp16_to_fp32(pkt.kicker);  // [V]

    robot_dev.set_target_local_linear_vel(vel_x, vel_y, true);

    if (pkt.angle_mode)
    {
        robot_dev.set_target_angular_dpos(w);
    }
    else
    {
        robot_dev.set_target_angular_vel(w);
    }

    robot_dev.set_dribbler_setting(static_cast<uint8_t>(dribbler));
    robot_dev.set_kicker_voltage(kicker);
    robot_dev.set_kicker_mode(to_kicker_mode(pkt.kicker_mode));
    drivers::buzzer_drv.set(pkt.beep);
}

SettingsPacket UartControl::current_settings()
{
    return {robot_dev.angle_k,          robot_dev.angle_b,         robot_dev.max_linear_vel,
            robot_dev.max_linear_accel, robot_dev.max_angular_vel, robot_dev.max_angular_accel,
            bldcs_drv.drive_vel_p,      bldcs_drv.drive_vel_i};
}

// NaN or negative: the field is not set, take the default. True if the value changed
static bool set_or_default(float &dst, float value, float default_value)
{
    if (isnan(value) || value < 0)
    {
        value = default_value;
    }
    if (value == dst)
    {
        return false;
    }
    dst = value;
    return true;
}

void UartControl::apply_settings(const SettingsPacket &pkt)
{
    if (!m_defaults_saved)
    {
        m_defaults = current_settings();
        m_defaults_saved = true;
    }
    const SettingsPacket &d = m_defaults;
    bool changed = set_or_default(robot_dev.angle_k, pkt.angle_k, d.angle_k);
    changed |= set_or_default(robot_dev.angle_b, pkt.angle_b, d.angle_b);
    changed |= set_or_default(robot_dev.max_linear_vel, pkt.max_linear_vel, d.max_linear_vel);
    changed |= set_or_default(robot_dev.max_linear_accel, pkt.max_linear_accel, d.max_linear_accel);
    changed |= set_or_default(robot_dev.max_angular_vel, pkt.max_angular_vel, d.max_angular_vel);
    changed |= set_or_default(robot_dev.max_angular_accel, pkt.max_angular_accel, d.max_angular_accel);
    bool wheel_pid = set_or_default(bldcs_drv.drive_vel_p, pkt.wheel_vel_p, d.wheel_vel_p);
    wheel_pid |= set_or_default(bldcs_drv.drive_vel_i, pkt.wheel_vel_i, d.wheel_vel_i);
    if (wheel_pid)
    {
        bldcs_drv.setDriveSettings();  // CAN to the motors, only when changed
    }
    if (changed || wheel_pid)
    {
        kinfo("Settings from the RPi: angle k %.2f b %.2f, max vel %.2f accel %.2f, max w %.2f accel %.1f, "
              "wheel p %.3f i %.2f",
              robot_dev.angle_k, robot_dev.angle_b, robot_dev.max_linear_vel, robot_dev.max_linear_accel,
              robot_dev.max_angular_vel, robot_dev.max_angular_accel, bldcs_drv.drive_vel_p,
              bldcs_drv.drive_vel_i);
    }
}

bool UartControl::init()
{
    uart_instance.init();
    return false;
}

void UartControl::send_telemetry()
{
    TxTelemetry tel = {};
    tel.robot_id = devices::robot_dev.robot_id;
    tel.voltage = fp32_to_fp16(devices::kicker_drv.get_voltage());
    tel.ball_deep = devices::kicker_drv.read_deep();
    tel.ball_front = devices::kicker_drv.read_front();

    uint8_t frame[kTxFrameLen];
    frame[0] = kSyncByte;
    memcpy(frame + 1, &tel, sizeof(tel));
    frame[kTxFrameLen - 1] = compute_checksum(frame, kTxFrameLen - 1);

    uart_instance.write(reinterpret_cast<const char *>(frame), sizeof(frame));
}

}  // namespace devices::uart_control
