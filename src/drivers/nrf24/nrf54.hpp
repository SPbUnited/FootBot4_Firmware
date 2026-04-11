#pragma once
// #include <stdio.h>
// #include <stdint.h>
// #include <string.h>
#include <inttypes.h>
#include "stm32f4xx_hal.h"

#include "nrf24.hpp"
#include "minifloat3.h"

namespace drivers::nrf24
{

class Nrf24Recv : public NRF24Config
{
private:
    // SPI handle
    SPI_HandleTypeDef *m_spi_handle;

    // Static data members
    static uint8_t m_txAddress[6];
    static uint32_t m_prevTime_ms;
    static uint32_t m_packetsReceived;
    static uint32_t m_packetsReceived_1s;
    static uint32_t m_ownPacketsReceived;
    static uint32_t m_ownPacketsReceived_1s;
    static uint8_t m_lenDbg;


    // Packet data
    static const uint32_t m_incomePacketLen = 6;
    static uint8_t m_incomeArray[m_incomePacketLen];
    static uint8_t m_iArray[8];

    // Bit masks
    static const uint32_t m_wpOpCodePos = 5;
    static const uint32_t m_wpXSpeedPos = 0;
    static const uint32_t m_wpYSpeedPos = 1;
    static const uint32_t m_wpRSpeedPos = 2;
    static const uint32_t m_wpKvlSpdPos = 3;
    static const uint32_t m_wpFlagsPos = 4;

    static const uint8_t m_wpLed = 0x80;
    static const uint8_t m_wpFrcKickForward = 0x40;
    static const uint8_t m_wpFrcKickUp = 0x20;
    static const uint8_t m_wpKupMask = 0x10;
    static const uint8_t m_wpKforMask = 0x08;
    static const uint8_t m_wpBspMask = 0x04;
    static const uint8_t m_wpDrbenMask = 0x02;
    static const uint8_t m_wpKceMask = 0x01;

    static const uint8_t m_kvlMask = 0xF0;
    static const uint8_t m_spdMask = 0x0F;

    // Internal register operations
    int spiUpdate_reg(uint8_t reg_addr, uint8_t mask, uint8_t value);
    int rawRead(uint8_t reg_addr, uint8_t *value, uint8_t len);
    int readReg(uint8_t reg_addr, uint8_t *value);
    int writeReg(uint8_t reg_addr, uint8_t value);
    int rawWrite(uint8_t reg_addr, uint8_t *value, uint8_t len);

    // CE control
    void setCe();
    void resetCe();

    // NRF24L01+ Register Map
    static constexpr uint8_t NRF24_REG_CONFIG = 0x00;
    static constexpr uint8_t NRF24_REG_EN_AA = 0x01;
    static constexpr uint8_t NRF24_REG_EN_RXADDR = 0x02;
    static constexpr uint8_t NRF24_REG_SETUP_AW = 0x03;
    static constexpr uint8_t NRF24_REG_SETUP_RETR = 0x04;
    static constexpr uint8_t NRF24_REG_DYNPD = 0x1C;
    static constexpr uint8_t NRF24_REG_FEATURE = 0x1D;
    static constexpr uint8_t NRF24_REG_RF_SETUP = 0x06;

    // NRF24 Commands
    static constexpr uint8_t NRF24_CMD_R_REGISTER = 0x00;
    static constexpr uint8_t NRF24_CMD_W_REGISTER = 0x20;
    static constexpr uint8_t NRF24_CMD_R_RX_PAYLOAD = 0x61;
    static constexpr uint8_t NRF24_CMD_W_TX_PAYLOAD = 0xA0;
    static constexpr uint8_t NRF24_CMD_FLUSH_TX = 0xE1;
    static constexpr uint8_t NRF24_CMD_FLUSH_RX = 0xE2;
    static constexpr uint8_t NRF24_CMD_REUSE_TX_PL = 0xE3;
    static constexpr uint8_t NRF24_CMD_NOP = 0xFF;

public:
    volatile uint32_t m_lastPacketTime;
    uint8_t m_address;

    Nrf24Recv(drivers::nrf24::NRF24Config config);

    int recv();
    void send(uint8_t checker, uint8_t id);

    static int8_t u8Toi8(uint8_t x);

    void flushTx(void);
    void flushRx(void);
};
}