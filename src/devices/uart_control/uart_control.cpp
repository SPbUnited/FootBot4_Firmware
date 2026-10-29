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
        m_id_from_buttons = m_known_id != kNoRobotId;  // not on the first call
        m_known_id = robot_dev.robot_id;
    }

    if (m_id_save_pending)
    {
        // Erasing the EEPROM flash sector stalls the main loop for a second or two; the robot
        // got a stop on the previous cycle
        m_id_save_pending = false;
        kinfo("Saving robot id %d to EEPROM", robot_dev.robot_id);
        eeprom::put(0, static_cast<robot::RobotSettings>(robot_dev));
        m_has_pkt = false;  // stale by now
        return 0;
    }

    if (m_has_pkt)
    {
        m_has_pkt = false;
        m_link_active = true;
        sync_robot_id(m_last_pkt.robot_id);
        if (m_id_save_pending)
        {
            stop_robot();
        }
        else
        {
            process_command(m_last_pkt);
        }
    }
    else if (m_link_active && HAL_GetTick() - m_lastPacketTime > kLinkTimeout_ms)
    {
        m_link_active = false;
        stop_robot();
    }

    return 0;
}

// Both sides send their id in every frame. An id changed with the buttons wins: it is kept
// until the RPi takes it (the RPi adopts our id after it has differed for a while). Otherwise
// the RPi's id wins, so a robot with a new RPi or a renamed RPi follows it. An id agreed with
// the RPi is saved to the EEPROM.
void UartControl::sync_robot_id(uint8_t rpi_id)
{
    uint8_t own_id = robot_dev.robot_id;
    if (rpi_id > kMaxRobotId)
    {
        return;
    }
    if (rpi_id == own_id)
    {
        if (m_id_from_buttons)
        {
            m_id_from_buttons = false;
            m_id_save_pending = true;
        }
        return;
    }
    if (m_id_from_buttons)
    {
        return;
    }
    kinfo("Robot id %d -> %d from the RPi", own_id, rpi_id);
    robot_dev.robot_id = rpi_id;
    m_known_id = rpi_id;
    m_id_save_pending = true;
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
