#pragma once

#include <inttypes.h>
#include <cstring>

#include "drivers/gpio/gpio.hpp"
#include "drivers/uart/uart.hpp"
#include "half_float.h"
#include "math.hpp"

namespace devices::uart_control
{

struct UartControlConfig
{
    drivers::uart::UartDriver &uart_instance;
    drivers::gpio::GPIOOutputDriver &dts_led;
};

// Frame format (both directions): [SYNC 0xA5][payload][XOR checksum of SYNC+payload]
constexpr uint8_t kSyncByte = 0xA5;
constexpr uint8_t kNoRobotId = 0xFF;
constexpr uint8_t kMaxRobotId = 15;  // the id buttons wrap at 16

// RX payload: <5eB??B = 5 half-floats + 1 byte + 2 bools + 1 byte = 14 bytes (frame = 16 bytes)
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
    uint8_t beep;
    uint8_t robot_id;  // kNoRobotId: the RPi doesn't know its id yet
};
#pragma pack(pop)
static_assert(sizeof(RxPacket) == 14, "must match CMD_FMT in fb4-runner/src/protocol/uart.py");

// TX telemetry payload: <Be?? = 5 bytes (frame = 7 bytes)
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
    static constexpr uint32_t kRxFrameLen = 1 + sizeof(RxPacket) + 1;
    static constexpr uint32_t kTxFrameLen = 1 + sizeof(TxTelemetry) + 1;
    static constexpr uint32_t kRxBufSize = 256;
    // Toggling on every frame (100 Hz) just looks dimly lit, so blink at ~5 Hz instead
    static constexpr uint32_t kLedTogglePackets = 10;
    // Stop the robot if no valid command arrived for this long
    static constexpr uint32_t kLinkTimeout_ms = 200;

    static uint32_t m_prevTime_ms;
    static uint32_t m_packetsReceived;
    static uint32_t m_packetsReceived_1s;

    static uint8_t m_rx_acc_buf[kRxBufSize];
    static uint32_t m_rx_acc_len;

    // Only the latest command matters, so keep one slot instead of a queue
    RxPacket m_last_pkt{};
    bool m_has_pkt = false;
    bool m_link_active = false;

    // Robot id agreement with the RPi, see sync_robot_id()
    uint8_t m_known_id = kNoRobotId;  // robot_dev.robot_id as of the last check
    bool m_id_from_buttons = false;   // changed with the buttons, the RPi hasn't taken it yet
    bool m_id_save_pending = false;

    void process_command(const RxPacket &pkt);
    void sync_robot_id(uint8_t rpi_id);
    void stop_robot();

  public:
    volatile uint32_t m_lastPacketTime = 0;

    UartControl(UartControlConfig &uart_control_config);

    bool init();
    void recv_irq();
    int fetch();
    void send_telemetry();
};

}  // namespace devices::uart_control
