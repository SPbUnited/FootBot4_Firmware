#include "spi.hpp"

#include <cstring>

#include "kernel/kernel.hpp"

namespace drivers::spi
{

SPIDriver::SPIDriver(SPIConfig config) : SPIConfig(config) {}

void SPIDriver::init()
{
    // Initialize SPI clock
    if (clk_enable)
    {
        clk_enable();
    }

    // Configure CE pin as output
    GPIO_InitTypeDef GPIO_InitStruct = {};
    GPIO_InitStruct.Pin = cePin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(cePort, &GPIO_InitStruct);

    // Configure CS pin as output
    GPIO_InitStruct.Pin = csPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(csPort, &GPIO_InitStruct);

    // Configure SPI pins (SCK, MISO, MOSI)
    GPIO_InitStruct.Pin = sckPin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = sckAlternate;
    HAL_GPIO_Init(sckPort, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = misoPin;
    GPIO_InitStruct.Alternate = misoAlternate;
    HAL_GPIO_Init(misoPort, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = mosiPin;
    GPIO_InitStruct.Alternate = mosiAlternate;
    HAL_GPIO_Init(mosiPort, &GPIO_InitStruct);
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);
    // Initialize SPI handle
    handle.Instance = instance;
    handle.Init.Mode = SPI_MODE_MASTER;
    handle.Init.Direction = SPI_DIRECTION_2LINES;
    handle.Init.DataSize = SPI_DATASIZE_8BIT;
    handle.Init.CLKPolarity = SPI_POLARITY_LOW;
    handle.Init.CLKPhase = SPI_PHASE_1EDGE;
    handle.Init.NSS = SPI_NSS_SOFT;
    handle.Init.BaudRatePrescaler =
        SPI_BAUDRATEPRESCALER_32;  // 180MHz / 2 = 90 MHz SPI clock (closest to 100 MHz)
    handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
    handle.Init.TIMode = SPI_TIMODE_DISABLE;
    handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    handle.Init.CRCPolynomial = 7;

    HAL_StatusTypeDef status = HAL_SPI_Init(&handle);
    if (status != HAL_OK)
    {
        // Error - SPI init failed
    }
}

int SPIDriver::rawWrite(uint8_t reg_addr, uint8_t *value, uint8_t len)
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
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);

    // Transmit data
    HAL_SPI_TransmitReceive(&handle, buffer_tx, buffer_rx, len + 1, 100);

    // Deselect chip
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);

    return 0;
}

int SPIDriver::rawRead(uint8_t reg_addr, uint8_t *value, uint8_t len)
{
    uint8_t buffer_tx[2] = {reg_addr, 0xFF};  // 0xFF is NOP command
    uint8_t buffer_rx[32] = {0};

    if (len > 64)
    {
        return -1;
    }

    // Select chip
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);

    // Send register address
    HAL_SPI_TransmitReceive(&handle, buffer_tx, buffer_rx, len + 1, 100);

    // Deselect chip
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);

    // Copy received data (skip first byte which is the NOP response)
    // memcpy(value, buffer_rx, len);
    for (size_t i = 0; i < len; i++)
    {
        value[i] = buffer_rx[i + 1];
    }
    // uint8_t *value_p = value;
    // for (uint8_t i = 0; i < len; ++i)
    // {
    //     *value_p = buffer_rx[len - i];
    //     value_p++;
    // }

    return 0;
}

void SPIDriver::setCe()
{
    HAL_GPIO_WritePin(cePort, cePin, GPIO_PIN_SET);
}

void SPIDriver::resetCe()
{
    HAL_GPIO_WritePin(cePort, cePin, GPIO_PIN_RESET);
}

void SPIDriver::setCS()
{
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);
}

void SPIDriver::resetCS()
{
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);
}

// Flush the TX FIFO
void SPIDriver::flushTx(void)
{
    uint8_t cmd = 0xE1;  // NRF24_CMD_FLUSH_TX
    uint8_t dummy;
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&handle, &cmd, 1, 100);
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);
}

// Flush the RX FIFO
void SPIDriver::flushRx(void)
{
    uint8_t cmd = 0xE2;  // NRF24_CMD_FLUSH_RX
    uint8_t dummy;
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&handle, &cmd, 1, 100);
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);
}

}  // namespace drivers::spi