#pragma once

#include <inttypes.h>

#include "drivers/uart/uart.hpp"
#include "math.hpp"

namespace devices::uart_control
{

struct UartControlConfig
{
    drivers::uart::UartDriver &uart_instance;
};

struct UartPacket
{
    uint8_t data[256];
    uint8_t len;
};

struct CommandData
{
    float xvel;
    float yvel;
    float wvel;
    uint8_t dribbler;
    uint8_t voltage;
    uint8_t kick_lower;
    uint8_t kick_upper;
    uint8_t autokick_lower;
    uint8_t autokick_upper;
    uint8_t autokick_momentum;
    uint8_t angle_mode;
};

class UartControl : public UartControlConfig
{
  private:
    static uint32_t m_prevTime_ms;
    static uint32_t m_packetsReceived;
    static uint32_t m_packetsReceived_1s;

    static const uint32_t m_incomePacketLen = 256;
    static uint8_t m_incomeArray[m_incomePacketLen];
    static uint8_t m_lenDbg;

    Queue<UartPacket, 1024> m_rx_queue;

    void process_command(const char *json);

  public:
    volatile uint32_t m_lastPacketTime;

    UartControl(UartControlConfig &uart_control_config);

    bool init();
    void recv_irq();
    int fetch();
    void send_telemetry();
};

}  // namespace devices::uart_control
