#include "nrf.hpp"
#include <math.h>

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

// Convert unsigned 8-bit to signed 8-bit
int8_t Nrf24Recv::u8Toi8(uint8_t x)
{
    if (x & 0x80)
        return -static_cast<int8_t>(0x100 - x);
    return static_cast<int8_t>(x);
}

// Flush the TX FIFO
void Nrf24Recv::flushTx(void)
{
    uint8_t cmd = 0xE1;  // NRF24_CMD_FLUSH_TX
    uint8_t dummy;
    HAL_GPIO_WritePin(m_chip_select_port, m_chip_select_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(m_spi_handle, &cmd, 1, 100);
    HAL_GPIO_WritePin(m_chip_select_port, m_chip_select_pin, GPIO_PIN_SET);
}

// Flush the RX FIFO
void Nrf24Recv::flushRx(void)
{
    uint8_t cmd = 0xE2;  // NRF24_CMD_FLUSH_RX
    uint8_t dummy;
    HAL_GPIO_WritePin(m_chip_select_port, m_chip_select_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(m_spi_handle, &cmd, 1, 100);
    HAL_GPIO_WritePin(m_chip_select_port, m_chip_select_pin, GPIO_PIN_SET);
}

// Main receive function
int Nrf24Recv::recv()
{
    uint8_t reg = 1;
    uint32_t time_ms = HAL_GetTick();
    int rc = readReg(7, &reg);  // 7 = NRF24_REG_FIFO_STATUS
    if (rc < 0)
    {
        // Error handling - could add logging here
    }

    if (reg & 0x40)  // RX_DR - Data Ready
    {
        // Read payload length
        uint32_t timeout_nrf_timer_recv = HAL_GetTick();
        rc = readReg(0x60, &m_lenDbg);  // 0x60 = RX_PAYLOAD_WIDTH0
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

        if (m_lenDbg == 6)
        {
            rawRead(0x61, m_incomeArray, m_lenDbg);  // 0x61 = R_RX_PAYLOAD
        }
        else if (m_lenDbg == 8)
        {
            rawRead(0x61, m_iArray, m_lenDbg);
        }
        writeReg(0x07, 0x40);  // Clear RX_DR interrupt

        m_lastPacketTime = HAL_GetTick();

        flushRx();
        // m_address = m_display->adrAndCh[0];

        if (m_lenDbg == 8)
        {
            if (m_address + 0xA0 == m_iArray[m_lenDbg - 1])
            {
                uint8_t t_test_arr[4];
                memcpy(t_test_arr, m_iArray, 4);
                // m_cannabus->sendDebugOverride(t_test_arr, m_iArray[m_lenDbg - 4] + (m_iArray[m_lenDbg - 3] << 8), m_lenDbg);
            }
        }

        if ((m_address != (m_incomeArray[5] & 0x0F)) ||
            (((m_address + 0xF0) == (m_incomeArray[5])) &&
             ((m_address + 0xF0) == (m_incomeArray[4])) &&
             ((m_address + 0xF0) == (m_incomeArray[3])) &&
             ((m_address + 0xF0) == (m_incomeArray[2])) &&
             ((m_address + 0xF0) == (m_incomeArray[1])) &&
             ((m_address + 0xF0) == (m_incomeArray[0]))))
        {
            return 0;  // Not for this device
        }

        // Parse received data
        uint8_t flags = m_incomeArray[0];
        uint8_t kvlSpd = m_incomeArray[1];
        uint8_t kvlVal = ((kvlSpd & m_kvlMask) >> 4);
        uint8_t radioBarrier = (flags & 0x80) > 0 ? 1 : 0;

        uint8_t speedDribler = 0;
        speedDribler = (kvlSpd & m_spdMask);

        // Motion control - standard format
        if (0x00 == (m_incomeArray[m_lenDbg - 1] & 0xF0))
        {
            int8_t iVal = u8Toi8(m_incomeArray[2]);
            iVal = u8Toi8(m_incomeArray[4]);
            iVal = u8Toi8(m_incomeArray[3]);
        }
        // Motion control - minifloat format
        else if (0xF0 == (m_incomeArray[m_lenDbg - 1] & 0xF0))
        {
            float iVal = minif_to_float3(m_incomeArray[2]);

            iVal = minif_to_float3(m_incomeArray[4]);

            iVal = minif_to_float3(m_incomeArray[3]);
        }
    }

    HAL_Delay(1);
    return 0;
}

// Send function
void Nrf24Recv::send(uint8_t checker, uint8_t id)
{
    uint8_t default_addr[]{2, checker, id};
    rawWrite(0xA0, default_addr, 3);  // NRF24_CMD_W_TX_PAYLOAD
}

// Constructor
Nrf24Recv::Nrf24Recv(SPI_HandleTypeDef *spi_handle,
                     uint32_t chip_enable_pin, GPIO_TypeDef *chip_enable_port,
                     uint32_t chip_select_pin, GPIO_TypeDef *chip_select_port)
{
    m_spi_handle = spi_handle;
    m_chip_enable_pin = chip_enable_pin;
    m_chip_enable_port = chip_enable_port;
    m_chip_select_pin = chip_select_pin;
    m_chip_select_port = chip_select_port;


    // Configure CE pin as output
    GPIO_InitTypeDef GPIO_InitStruct = {};
    GPIO_InitStruct.Pin = m_chip_enable_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(m_chip_enable_port, &GPIO_InitStruct);

    // Configure CS pin as output
    GPIO_InitStruct.Pin = m_chip_select_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(m_chip_select_port, &GPIO_InitStruct);

    // Set CE high to enable the module
    setCe();
    HAL_Delay(5);
    resetCe();

    // Initialize NRF24 module
    auto w = [this](uint8_t reg, uint8_t val)
    { writeReg(reg, val); };
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

    // Set addresses
    uint8_t default_addr_0[]{0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
    uint8_t default_addr_1[]{0xC2, 0xC2, 0xC2, 0xC2, 0xC2};

    // Write TX address
    uint8_t cmd[6];
    cmd[0] = 0x10;  // W_TX_ADDR
    memcpy(&cmd[1], default_addr_0, 5);
    HAL_GPIO_WritePin(m_chip_select_port, m_chip_select_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(m_spi_handle, cmd, 6, 100);
    HAL_GPIO_WritePin(m_chip_select_port, m_chip_select_pin, GPIO_PIN_SET);

    // Write RX address pipe 1
    cmd[0] = 0x0B;  // RX_ADDR_P1
    memcpy(&cmd[1], default_addr_1, 5);
    HAL_GPIO_WritePin(m_chip_select_port, m_chip_select_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(m_spi_handle, cmd, 6, 100);
    HAL_GPIO_WritePin(m_chip_select_port, m_chip_select_pin, GPIO_PIN_SET);

    // Set RX pipe 1 address width
    w(0x03, 0x03);  // SETUP_AW: 5 bytes

    // Set channel
    w(0x05, 0x4C);  // RF_CH: Channel 76 (2.476 GHz)

    // Power up and flush
    w(0x00, 0x0D);  // CONFIG: PWR_UP, CRCEN, PRIM_RX=1 (RX mode)
    flushRx();
    w(0x00, 0x0C);  // CONFIG: PWR_UP, CRCEN, PRIM_RX=0 (TX mode)
    flushTx();
    w(0x07, 0x70);  // FIFO_STATUS: Clear status flags

    HAL_Delay(10);

    // Final configuration
    w(0x06, 0x0E);  // CONFIG: PWR_UP, CRCEN, PRIM_RX=0

    if (r(0x00) != 0x0C)
    {
        // Error
    }

    w(0x03, 0x01);  // RX_ADDR_P1
    w(0x06, 0x0E);

    if (r(0x03) != 0x01)
    {
        // Error
    }

    // Set self address
    uint8_t self_addr[]{0xAB, 0xAD, 0xAF};
    cmd[0] = 0x10;  // W_TX_ADDR
    memcpy(&cmd[1], self_addr, 3);
    HAL_GPIO_WritePin(m_chip_select_port, m_chip_select_pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(m_spi_handle, cmd, 4, 100);
    HAL_GPIO_WritePin(m_chip_select_port, m_chip_select_pin, GPIO_PIN_SET);

    // Set TX/RX mode
    w(0x02, 0x06);  // EN_RXADDR: Enable pipe 1 and 2

    // Final setup
    w(0x11, 0x20);  // RX_PW_P1: Payload width 32 bytes
    w(0x00, 0x0E);  // CONFIG: PWR_UP, CRCEN, PRIM_RX=1 (RX mode)

    flushRx();
    w(0x07, 0x40);  // Clear RX_DR

    setCe();
    HAL_Delay(1);
    resetCe();
    HAL_Delay(1);
    setCe();
    HAL_Delay(10);
}

// Private methods implementation

// SPI register update with mask
int Nrf24Recv::spiUpdate_reg(uint8_t reg_addr, uint8_t mask, uint8_t value)
{
    uint8_t tmp_val;

    rawRead(reg_addr, &tmp_val, 1);
    tmp_val = (tmp_val & ~mask) | (value & mask);

    return rawWrite(reg_addr, &tmp_val, 1);
}

// Raw read from NRF24 register
int Nrf24Recv::rawRead(uint8_t reg_addr, uint8_t *value, uint8_t len)
{
    uint8_t buffer_tx[2] = {reg_addr, 0xFF};  // 0xFF is NOP command
    uint8_t buffer_rx[10] = {0};

    if (len > 64)
    {
        return -1;
    }

    // Select chip
    HAL_GPIO_WritePin(m_chip_select_port, m_chip_select_pin, GPIO_PIN_RESET);

    // Send register address
    HAL_SPI_Transmit(m_spi_handle, buffer_tx, 1, 100);

    // Receive data
    HAL_SPI_Receive(m_spi_handle, buffer_rx, len + 1, 100);

    // Deselect chip
    HAL_GPIO_WritePin(m_chip_select_port, m_chip_select_pin, GPIO_PIN_SET);

    // Copy received data (skip first byte which is the NOP response)
    uint8_t *value_p = value;
    for (uint8_t i = 0; i < len; ++i)
    {
        *value_p = buffer_rx[len - i];
        value_p++;
    }

    return 0;
}

// Read single register
int Nrf24Recv::readReg(uint8_t reg_addr, uint8_t *value)
{
    return rawRead(reg_addr, value, 1);
}

// Write single register
int Nrf24Recv::writeReg(uint8_t reg_addr, uint8_t value)
{
    return rawWrite(reg_addr, &value, 1);
}

// Raw write to NRF24 register
int Nrf24Recv::rawWrite(uint8_t reg_addr, uint8_t *value, uint8_t len)
{
    uint8_t buffer_tx[66];  // 1 byte address + 64 bytes data + 1 for safety
    uint8_t buffer_rx[66];

    buffer_tx[0] = reg_addr;
    if (reg_addr < 0x20)  // NRF24_CMD_W_REGISTER
    {
        buffer_tx[0] = 0x20 | (reg_addr & 0x1F);
    }

    if (value != NULL && len > 0)
    {
        memcpy(&buffer_tx[1], value, len);
    }

    // Select chip
    HAL_GPIO_WritePin(m_chip_select_port, m_chip_select_pin, GPIO_PIN_RESET);

    // Transmit data
    HAL_SPI_TransmitReceive(m_spi_handle, buffer_tx, buffer_rx, len + 1, 100);

    // Deselect chip
    HAL_GPIO_WritePin(m_chip_select_port, m_chip_select_pin, GPIO_PIN_SET);

    return 0;
}

// Set CE pin high
void Nrf24Recv::setCe()
{
    HAL_GPIO_WritePin(m_chip_enable_port, m_chip_enable_pin, GPIO_PIN_SET);
}

// Set CE pin low
void Nrf24Recv::resetCe()
{
    HAL_GPIO_WritePin(m_chip_enable_port, m_chip_enable_pin, GPIO_PIN_RESET);
}
