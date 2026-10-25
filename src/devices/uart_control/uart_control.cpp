#include "uart_control.hpp"

#include <cstdio>
#include <cstring>

#include "kernel/kernel.hpp"
#include "devices/device_manager.hpp"

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

int UartControl::fetch()
{
    if (m_has_pkt)
    {
        m_has_pkt = false;
        process_command(m_last_pkt);
    }

    return 0;
}

void UartControl::process_command(const RxPacket &pkt)
{
    float vel_x = fp16_to_fp32(pkt.vel_x);
    float vel_y = fp16_to_fp32(pkt.vel_y);
    float w = fp16_to_fp32(pkt.w);
    float dribbler = fp16_to_fp32(pkt.dribbler);
    float kicker = fp16_to_fp32(pkt.kicker);

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
    robot_dev.set_kicker_setting(static_cast<uint8_t>(kicker));
    robot_dev.set_kicker_mode(static_cast<robot::KickerMode>(pkt.kicker_mode));
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
