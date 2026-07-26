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

void UartControl::recv_irq()
{
    while (m_rx_acc_len < kRxBufSize &&
           uart_instance.getc(reinterpret_cast<char *>(&m_rx_acc_buf[m_rx_acc_len])) == HAL_OK)
    {
        m_rx_acc_len++;
    }

    if (m_rx_acc_len == 0)
    {
        return;
    }

    m_lastPacketTime = HAL_GetTick();
    m_packetsReceived++;

    uint32_t time_ms = HAL_GetTick();
    if (time_ms - m_prevTime_ms > 1000)
    {
        m_prevTime_ms = time_ms;
        m_packetsReceived_1s = m_packetsReceived;
        m_packetsReceived = 0;
    }

    while (m_rx_acc_len >= kPacketLen)
    {
        RxPacket pkt;
        memcpy(&pkt, m_rx_acc_buf, kPacketLen);
        m_rx_queue.push_back(pkt);

        m_rx_acc_len -= kPacketLen;
        if (m_rx_acc_len > 0)
        {
            memmove(m_rx_acc_buf, m_rx_acc_buf + kPacketLen, m_rx_acc_len);
        }
    }
}

int UartControl::fetch()
{
    while (!m_rx_queue.isEmpty())
    {
        RxPacket pkt = m_rx_queue.pop_front();
        process_command(pkt);
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

    uart_instance.write(reinterpret_cast<const char *>(&tel), sizeof(tel));
}

}  // namespace devices::uart_control
