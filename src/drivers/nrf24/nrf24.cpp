#include "nrf24.hpp"

#include "drivers/driver_manager.hpp"
#include "kernel/kernel.hpp"

namespace drivers::nrf24
{

NRF24Driver::NRF24Driver(NRF24Config config) : NRF24Config(config) {}

void NRF24Driver::init()
{
    clk_enable();

    // Configure SPI pins
    GPIO_InitTypeDef GPIO_InitStruct;

    // SCK pin configuration
    GPIO_InitStruct.Pin = sckPin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = sckAlternate;

    HAL_GPIO_Init(sckPort, &GPIO_InitStruct);

    // MISO pin configuration
    GPIO_InitStruct.Pin = misoPin;
    GPIO_InitStruct.Alternate = misoAlternate;

    HAL_GPIO_Init(misoPort, &GPIO_InitStruct);

    // MOSI pin configuration
    GPIO_InitStruct.Pin = mosiPin;
    GPIO_InitStruct.Alternate = mosiAlternate;

    HAL_GPIO_Init(mosiPort, &GPIO_InitStruct);

    // Configure CE pin as output
    GPIO_InitStruct.Pin = cePin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(cePort, &GPIO_InitStruct);

    // Configure CS pin as output
    GPIO_InitStruct.Pin = csPin;
    GPIO_InitStruct.Alternate = 0;  // No alternate function for CS
    HAL_GPIO_Init(csPort, &GPIO_InitStruct);

    // Configure IRQ pin as input (optional)
    if (irqPin != 0)
    {
        GPIO_InitStruct.Pin = irqPin;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(irqPort, &GPIO_InitStruct);
    }

    // Initialize SPI
    handle.Instance = instance;

    handle.Init.Mode = SPI_MODE_MASTER;
    handle.Init.Direction = SPI_DIRECTION_2LINES;
    handle.Init.DataSize = SPI_DATASIZE_8BIT;
    handle.Init.CLKPolarity = SPI_POLARITY_LOW;
    handle.Init.CLKPhase = SPI_PHASE_1EDGE;
    handle.Init.NSS = SPI_NSS_SOFT;
    handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;  // 180MHz / 32 = 5.625MHz SPI clock
    handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
    handle.Init.TIMode = SPI_TIMODE_DISABLE;
    handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    handle.Init.CRCPolynomial = 10;

    HAL_StatusTypeDef status = HAL_SPI_Init(&handle);
    if (status != HAL_OK)
    {
        kerror("NRF24 SPI init error: %d\n", int(status));
    }

    // Set CE high to enable the module
    HAL_GPIO_WritePin(cePort, cePin, GPIO_PIN_SET);

    // Set CS high (deselect)
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);
}

void NRF24Driver::write(uint32_t id, uint8_t *data, uint8_t len)
{
    // TODO: Implement write with ID filtering
    // For now, just send data through SPI
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&handle, data, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);
}

void NRF24Driver::read(uint32_t id, uint8_t *data, uint8_t len)
{
    // TODO: Implement read with ID filtering
    // For now, just receive data through SPI
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);
    HAL_SPI_Receive(&handle, data, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);
}

void NRF24Driver::setReg(uint32_t reg, uint32_t value)
{
    uint8_t cmd[2] = {
        static_cast<uint8_t>(reg & 0x1F),  // Register address (5 bits)
        static_cast<uint8_t>(value & 0xFF)  // Value
    };
    write(0, cmd, 2);
}

}  // namespace drivers::nrf24
