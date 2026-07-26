#pragma once

#include <inttypes.h>
#include <cstring>

#include "drivers/uart/uart.hpp"
#include "half_float.h"
#include "math.hpp"

namespace devices::uart_control
{

struct UartControlConfig
{
    drivers::uart::UartDriver &uart_instance;
};

// RX packet: <5eB? = 5 half-floats + 1 byte + 1 bool = 12 bytes
#pragma pack(push, 1)
struct RxPacket
{
    uint16_t vel_x;
    uint16_t vel_y;
    uint16_t w;
    uint16_t dribbler;
    uint16_t kicker;
    uint8_t kicker_mode;
    uint8_t angle_mode;
};
#pragma pack(pop)

// TX telemetry packet: <Be?? = 5 bytes
#pragma pack(push, 1)
struct TxTelemetry
{
    uint8_t robot_id;
    uint16_t voltage;
    uint8_t ball_deep;
    uint8_t ball_front;
};
#pragma pack(pop)

class UartControl : public UartControlConfig
{
  private:
    static constexpr uint32_t kPacketLen = sizeof(RxPacket);
    static constexpr uint32_t kRxBufSize = 256;

    static uint32_t m_prevTime_ms;
    static uint32_t m_packetsReceived;
    static uint32_t m_packetsReceived_1s;

    static uint8_t m_rx_acc_buf[kRxBufSize];
    static uint32_t m_rx_acc_len;

    Queue<RxPacket, 8> m_rx_queue;

    void process_command(const RxPacket &pkt);

  public:
    volatile uint32_t m_lastPacketTime;

    UartControl(UartControlConfig &uart_control_config);

    bool init();
    void recv_irq();
    int fetch();
    void send_telemetry();
};

}  // namespace devices::uart_control
