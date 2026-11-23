#include "nrf24_feedback.hpp"

#include <cstdio>
#include "devices/device_manager.hpp"
#include "drivers/system_clock/system_clock.hpp"

namespace devices::nrf24
{
namespace
{
constexpr uint8_t statusReg = 0x07, txDone = 0x20, maxRetries = 0x10;
constexpr uint8_t flushTx = 0xE1, writePayload = 0xA0;
}

bool Nrf24Feedback::writeReg(uint8_t reg, uint8_t value)
{
    return spi_instance.rawWrite(reg, &value, 1) == 0;
}

void Nrf24Feedback::init()
{
    stats.ready = false;
    sending = false;
    spi_instance.resetCe();
    spi_instance.setCS();

    auto matches = [this](uint8_t reg, uint8_t expected) {
        uint8_t value = 0;
        return spi_instance.rawRead(reg, &value, 1) == 0 && value == expected;
    };

    // Probe CONFIG, then configure the radio in power-down as in Nrf24Recv::init.
    if (!writeReg(0x00, 0x0E) || !matches(0x00, 0x0E) || !writeReg(0x00, 0x0C))
        return;

    uint8_t address[] = {0xAB, 0xAD, 0xAF};
    if (!writeReg(0x02, 0) || !writeReg(0x01, 0) ||  // RX pipes and auto-ack off
        !writeReg(0x04, 0) ||                        // No retransmission
        !writeReg(0x1D, 0x04) || !writeReg(0x1C, 0x01) ||  // Dynamic payload, TX pipe 0
        !writeReg(0x03, 1) || !matches(0x03, 1) ||   // Three-byte address
        spi_instance.rawWrite(0x10, address, sizeof(address)) ||
        !writeReg(0x05, 76) || !writeReg(0x06, 0x0E))  // 2 Mbps, 0 dBm
        return;

    if (spi_instance.rawWrite(0xE2, nullptr, 0) || spi_instance.rawWrite(flushTx, nullptr, 0) ||
        !writeReg(statusReg, 0x70) || !matches(0x00, 0x0C) || !matches(0x02, 0))
        return;

    // Leave CE low in powered PTX standby until send() loads a payload.
    if (!writeReg(0x00, 0x0E) || !matches(0x00, 0x0E))
        return;
    HAL_Delay(2);
    stats.ready = true;
}

void Nrf24Feedback::send()
{
    if (!stats.ready)
        return;
    if (sending)
    {
        uint8_t status = 0;
        if (spi_instance.rawRead(statusReg, &status, 1) || !(status & (txDone | maxRetries)))
            return;
        if ((status & maxRetries) && spi_instance.rawWrite(flushTx, nullptr, 0))
            return;
        if (!writeReg(statusReg, txDone | maxRetries))
            return;
        if ((status & txDone) && !(status & maxRetries))
        {
            ++stats.completed;
            stats.lastCompletedMs = HAL_GetTick();
        }
        sending = false;
    }

    const unsigned ball_sensors = devices::kicker_drv.read_front() +
                                  (devices::kicker_drv.read_deep() << 1);
    const float volts = can_battery_voltage;
    if (!(volts >= 0 && volts <= 1000))
        return;

    char packet[33];
    const int length = std::snprintf(packet, sizeof(packet), "id=%u,k=%u,mv=%u",
                                     static_cast<unsigned>(devices::robot_dev.robot_id),
                                     ball_sensors, static_cast<unsigned>(volts * 1000 + 0.5f));
    if (length <= 0 || length >= static_cast<int>(sizeof(packet)) ||
        spi_instance.rawWrite(writePayload, reinterpret_cast<uint8_t *>(packet), length))
        return;

    sending = true;
    spi_instance.setCe();
    const uint32_t start = drivers::system_clock::micros();
    while (drivers::system_clock::micros() - start < 12) {}
    spi_instance.resetCe();
}
}
