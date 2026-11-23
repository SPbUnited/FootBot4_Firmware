#include "nrf24_feedback.hpp"

#include <cstdio>
#include "devices/device_manager.hpp"
#include "drivers/system_clock/system_clock.hpp"

#define NRF24_REG_RX_ADDR_P0 (uint8_t)0x0A  // Receive address data pipe 0
#define NRF24_REG_RX_ADDR_P1 (uint8_t)0x0B  // Receive address data pipe 1
#define NRF24_REG_RX_ADDR_P2 (uint8_t)0x0C  // Receive address data pipe 2
#define NRF24_REG_RX_ADDR_P3 (uint8_t)0x0D  // Receive address data pipe 3
#define NRF24_REG_RX_ADDR_P4 (uint8_t)0x0E  // Receive address data pipe 4
#define NRF24_REG_RX_ADDR_P5 (uint8_t)0x0F  // Receive address data pipe 5
#define NRF24_REG_TX_ADDR (uint8_t)0x10     // Transmit address
#define NRF24_REG_RX_PW_P0 (uint8_t)0x11    // Number of bytes in RX payload in data pipe 0
#define NRF24_REG_RX_PW_P1 (uint8_t)0x12    // Number of bytes in RX payload in data pipe 1
#define NRF24_REG_RX_PW_P2 (uint8_t)0x13    // Number of bytes in RX payload in data pipe 2
#define NRF24_REG_RX_PW_P3 (uint8_t)0x14    // Number of bytes in RX payload in data pipe 3
#define NRF24_REG_RX_PW_P4 \
    (uint8_t)0x15  // Number of bytes inNRF24_CMD_NOP RX payload in data pipe 4
#define NRF24_REG_RX_PW_P5 (uint8_t)0x16  // Number of bytes in RX payload in data pipe 5

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

void Nrf24Feedback::start()
{
    stats.ready = false;
    sending = false;
    spi_instance.setCS();
    stats.ready = !init();
}

bool Nrf24Feedback::init()
{
    bool mismatch = false;

    // Set CE high to enable the module
    spi_instance.setCe();
    HAL_Delay(5);
    spi_instance.resetCe();

    // Initialize NRF24 module
    auto w = [this](uint8_t reg, uint8_t val) { writeReg(reg, val); };
    auto r = [this](uint8_t reg)
    {
        uint8_t data = 2;
        readReg(reg, &data);
        return data;
    };

    // Configure NRF24
    w(0x00, 0x0E);  // CONFIG: PWR_UP, CRCO=1 (2-byte CRC)
    uint8_t a = r(0x00);
    if (a != 0x0E)
    {
        mismatch = true;
    }

    w(0x00, 0x0C);  // CONFIG: PWR_UP, CRCEN, PRIM_RX=0 (TX mode)
    w(0x06, 0x06);  // RF_SETUP: 250kbps, 0dBm
    w(0x02, 0x00);  // EN_RXADDR: All pipes disabled
    w(0x1C, 0x3F);  // DYNPD: Enable dynamic payload
    w(0x01, 0x3F);  // EN_AA: Enable auto-ack on all pipes
    w(0x1D, 0x05);  // FEATURE: Enable features
    w(0x04, 0x53);  // SETUP_RETR: 5 retransmit, 500us delay

    uint8_t default_addr_0[]{0xe7, 0xe7, 0xe7, 0xe7, 0xe7};
    spi_instance.rawWrite(0x0A, default_addr_0, 5);

    w(NRF24_REG_RX_PW_P0, 0x00);

    uint8_t default_addr_1[]{0xc2, 0xc2, 0xc2, 0xc2, 0xc2};
    spi_instance.rawWrite(0x0B, default_addr_1, 5);

    w(NRF24_REG_RX_PW_P1, 0x00);
    w(NRF24_REG_RX_ADDR_P2, 0xC3);
    w(NRF24_REG_RX_PW_P2, 0x00);
    w(NRF24_REG_RX_ADDR_P3, 0xC4);
    w(NRF24_REG_RX_PW_P3, 0x00);
    w(NRF24_REG_RX_ADDR_P4, 0xC5);
    w(NRF24_REG_RX_PW_P4, 0x00);
    w(NRF24_REG_RX_ADDR_P5, 0xC6);
    w(NRF24_REG_RX_PW_P5, 0x00);

    spi_instance.rawWrite(0x10, default_addr_0, 5);

    w(0x03, 0x03);
    if (devices::robot_dev.robot_id <= 7)
    {
        w(0x05, 76);  // channel set
    }
    else
    {
        w(0x05, 76);  // channel set
    }

    w(0x00, 0x0D);
    spi_instance.flushRx();
    w(0x00, 0x0C);
    spi_instance.flushTx();
    w(0x07, 0x70);

    HAL_Delay(10);
    w(0x06, 0x0E);

    if (r(0x00) != 0x0C)
    {
        mismatch = true;
    }
    w(0x03, 0x01);
    w(0x06, 0x0E);
    // w(0x01, 0x40);
    w(0x01, 0x00);

    if (r(0x03) != 0x01)
    {
        mismatch = true;
    }

    uint8_t self_addr[]{0xAB, 0xAD, 0xAF};
    // rawWrite(0x0A, self_addr, 3);
    // uint8_t self_addr[]{0xAF, 0xAD, 0xAB};
    // if (send_or_recieve)
    //     rawWrite(0x10, self_addr, 3);
    // else
    spi_instance.rawWrite(0x10, self_addr, 3);

    if (r(0x02) != 0x00)
    {
        mismatch = true;
    }
    // if (send_or_recieve)
    //     w(0x02, 0x06);
    // else
    w(0x02, 0x06);
    w(0x11, 0x20);

    spi_instance.rawWrite(0x0A, self_addr, 3);
    // if (send_or_recieve)
    //     w(0x00, 0x0E);
    // else
    w(0x00, 0x0E);

    spi_instance.flushRx();

    w(0x07, 0x40);

    spi_instance.setCe();
    HAL_Delay(1);
    spi_instance.resetCe();
    HAL_Delay(1);
    spi_instance.setCe();
    HAL_Delay(10);
    spi_instance.resetCS();
    HAL_Delay(10);

    if (mismatch)
        kwarning("NRF feedback register readback mismatch");
    return false;
}

void Nrf24Feedback::send()
{
    if (!stats.ready)
        return;
    spi_instance.resetCe();
    spi_instance.setCS();
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
