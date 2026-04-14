#include "nrf24.hpp"
// #include <math.h>
#include "devices/device_manager.hpp"
#include "kernel/kernel.hpp"

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
// Static member definitions
uint8_t Nrf24Recv::m_txAddress[6] = {0};
uint32_t Nrf24Recv::m_prevTime_ms = 0;
uint32_t Nrf24Recv::m_packetsReceived = 0;
uint32_t Nrf24Recv::m_packetsReceived_1s = 0;
uint32_t Nrf24Recv::m_ownPacketsReceived = 0;
uint32_t Nrf24Recv::m_ownPacketsReceived_1s = 0;

uint8_t Nrf24Recv::m_incomeArray[Nrf24Recv::m_incomePacketLen];
uint8_t Nrf24Recv::m_iArray[8];
uint8_t Nrf24Recv::m_lenDbg;

Nrf24Recv::Nrf24Recv(drivers::spi::SPIDriver &spi_instance) : spi_instance(spi_instance) {}

// Convert unsigned 8-bit to signed 8-bit
int8_t Nrf24Recv::u8Toi8(uint8_t x)
{
    if (x & 0x80)
        return -static_cast<int8_t>(0x100 - x);
    return static_cast<int8_t>(x);
}

// Main receive function
int Nrf24Recv::recv()
{
    for (size_t i = 0; i < 1; i++)
    {
        uint8_t reg = 1;
        uint32_t time_ms = 0;       // HAL_GetTick();
        int rc = readReg(7, &reg);  // 7 = NRF24_REG_FIFO_STATUS
        if (rc < 0)
        {
            // Error handling - could add logging here
        }

        // drivers::leds.toggle(led::DRV1);
        if (!(reg & 0x40))  // RX_DR - Data Not Ready
        {
            continue;
        }

        drivers::out_pins[drivers::LED_DATA_TRANSFER_STATUS_1].toggle();

        // drivers::leds.toggle(led::DATA_TRANSFER_STATUS_1);
        // Read payload length using R_RX_PL_WID command (0x60)
        uint32_t timeout_nrf_timer_recv = 0;            // HAL_GetTick();
        rc = spi_instance.rawRead(0x60, &m_lenDbg, 1);  // 0x60 = R_RX_PL_WID command
        if (rc < 0)
        {
            // Error handling
        }

        if (m_lenDbg < 6)
        {
            // Log error - could add logging here
        }

        if (time_ms - m_prevTime_ms > 1000)
        {
            m_prevTime_ms = time_ms;
            m_packetsReceived_1s = m_packetsReceived;
            m_packetsReceived = 0;
            m_ownPacketsReceived_1s = m_ownPacketsReceived;
            m_ownPacketsReceived = 0;
        }
        m_packetsReceived++;

        // if (m_lenDbg == 6)
        // {
        //     spi_instance.rawRead(0x61, m_incomeArray, m_lenDbg);  // 0x61 = R_RX_PAYLOAD
        // }
        // else if (m_lenDbg == 8)
        // {
        //     spi_instance.rawRead(0x61, m_iArray, m_lenDbg);
        // }

        spi_instance.rawRead(0x61, m_incomeArray, m_lenDbg);

        writeReg(0x07, 0x40);  // Clear RX_DR interrupt

        m_lastPacketTime = HAL_GetTick();

        devices::nrfm_decoder::nrfm_rx_callback(m_incomeArray, m_lenDbg);

        // m_address = m_display->adrAndCh[0];

        // if (m_lenDbg == 8)
        // {
        //     if (m_address + 0xA0 == m_iArray[m_lenDbg - 1])
        //     {
        //         uint8_t t_test_arr[4];
        //         // memcpy(t_test_arr, m_iArray, 4);
        //         // m_cannabus->sendDebugOverride(t_test_arr, m_iArray[m_lenDbg - 4] +
        //         // (m_iArray[m_lenDbg - 3] << 8), m_lenDbg);
        //     }
        // }
        // // drivers::
        // uint8_t m_address = 6;
        // if ((m_address != (m_incomeArray[5] & 0x0F)) ||
        //     (((m_address + 0xF0) == (m_incomeArray[5])) &&
        //      ((m_address + 0xF0) == (m_incomeArray[4])) &&
        //      ((m_address + 0xF0) == (m_incomeArray[3])) &&
        //      ((m_address + 0xF0) == (m_incomeArray[2])) &&
        //      ((m_address + 0xF0) == (m_incomeArray[1])) &&
        //      ((m_address + 0xF0) == (m_incomeArray[0]))))
        // {
        //     return 0;  // Not for this device
        // }
        // drivers::leds.toggle(led::DRV5);
        // drivers::out_pins[drivers::LED_DATA_TRANSFER_STATUS_2].toggle();
        // Parse received data
        // uint8_t flags = m_incomeArray[0];
        // uint8_t kvlSpd = m_incomeArray[1];
        // uint8_t kvlVal = ((kvlSpd & m_kvlMask) >> 4);
        // uint8_t radioBarrier = (flags & 0x80) > 0 ? 1 : 0;

        // uint8_t speedDribler = 0;
        // speedDribler = (kvlSpd & m_spdMask);

        // // Motion control - standard format
        // if (0x00 == (m_incomeArray[m_lenDbg - 1] & 0xF0))
        // {
        //     int8_t iVal = u8Toi8(m_incomeArray[2]);
        //     iVal = u8Toi8(m_incomeArray[4]);
        //     iVal = u8Toi8(m_incomeArray[3]);
        // }
        // // Motion control - minifloat format
        // else if (0xF0 == (m_incomeArray[m_lenDbg - 1] & 0xF0))
        // {
        //     float iVal = minif_to_float3(m_incomeArray[2]);

        //     iVal = minif_to_float3(m_incomeArray[4]);

        //     iVal = minif_to_float3(m_incomeArray[3]);
        // }
    }

    spi_instance.flushRx();

    // HAL_Delay(1);
    return 0;
}

// Send function
void Nrf24Recv::send(uint8_t checker, uint8_t id)
{
    uint8_t default_addr[]{2, checker, id};
    spi_instance.rawWrite(0xA0, default_addr, 3);  // NRF24_CMD_W_TX_PAYLOAD
}

// Constructor
void Nrf24Recv::init()
{
    // spi.init()

    // Set CE high to enable the module
    spi_instance.setCe();
    for (int i = 0; i < 5000; i++)
        ;
    // HAL_Delay(5);
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
        // Error - NRF24 not responding
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

    w(0x05, 0x34);  // channel set
    w(0x00, 0x0D);
    spi_instance.flushRx();
    w(0x00, 0x0C);
    spi_instance.flushTx();
    w(0x07, 0x70);

    // k_sleep(K_MSEC(10));
    for (int i = 0; i < 10000; i++)
        ;
    w(0x06, 0x0E);

    if (r(0x00) != 0x0C)
    {
        volatile int a = 0;
        a++;
        // fail();
    }
    w(0x03, 0x01);
    w(0x06, 0x0E);
    // w(0x01, 0x40);
    w(0x01, 0x00);

    if (r(0x03) != 0x01)
    {
        volatile int a = 0;
        a++;
        // fail();
    }

    uint8_t self_addr[]{0xAB, 0xAD, 0xAF};
    // rawWrite(0x0A, self_addr, 3);
    // uint8_t self_addr[]{0xAF, 0xAD, 0xAB};
    // if (send_or_recieve)
    //     rawWrite(0x10, self_addr, 3);
    // else
    spi_instance.rawWrite(0x0A, self_addr, 3);

    if (r(0x02) != 0x00)
    {
        volatile int a = 0;
        a++;
        // fail();
    }
    // if (send_or_recieve)
    //     w(0x02, 0x06);
    // else
    w(0x02, 0x01);
    w(0x11, 0x20);

    spi_instance.rawWrite(0x0A, self_addr, 3);
    // if (send_or_recieve)
    //     w(0x00, 0x0E);
    // else
    w(0x00, 0x0F);

    spi_instance.flushRx();

    w(0x07, 0x40);

    spi_instance.setCe();
    // k_sleep(K_MSEC(1));
    for (int i = 0; i < 1000; i++)
        ;
    spi_instance.resetCe();
    // k_sleep(K_MSEC(1));
    for (int i = 0; i < 1000; i++)
        ;
    spi_instance.setCe();
    for (int i = 0; i < 5000; i++)
        ;
    spi_instance.resetCS();
}

// Private methods implementation

// SPI register update with mask TODO: rewrite to this
int Nrf24Recv::spiUpdate_reg(uint8_t reg_addr, uint8_t mask, uint8_t value)
{
    uint8_t tmp_val;

    spi_instance.rawRead(reg_addr, &tmp_val, 1);
    tmp_val = (tmp_val & ~mask) | (value & mask);

    return spi_instance.rawWrite(reg_addr, &tmp_val, 1);
}

// Read single register
int Nrf24Recv::readReg(uint8_t reg_addr, uint8_t *value)
{
    return spi_instance.rawRead(reg_addr, value, 1);
}

// Write single register
int Nrf24Recv::writeReg(uint8_t reg_addr, uint8_t value)
{
    return spi_instance.rawWrite(reg_addr, &value, 1);
}

}  // namespace devices::nrf24