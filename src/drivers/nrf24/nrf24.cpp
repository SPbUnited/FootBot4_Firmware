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

    // Initialize NRF24 module
    init_nrf24();
}

void NRF24Driver::init_nrf24()
{
    // Wait for module to stabilize
    HAL_Delay(5);

    // Configure basic settings
    write_register(NRF24_REG_EN_AA, NRF24_DEFAULT_EN_AA);
    write_register(NRF24_REG_EN_RXADDR, NRF24_DEFAULT_EN_RXADDR);
    write_register(NRF24_REG_SETUP_AW, NRF24_DEFAULT_SETUP_AW);
    write_register(NRF24_REG_SETUP_RETR, NRF24_DEFAULT_SETUP_RETR);
    write_register(NRF24_REG_DYNPD, NRF24_DEFAULT_DYNPD);
    write_register(NRF24_REG_FEATURE, NRF24_DEFAULT_FEATURE);

    // Set default data rate (250kbps) and power (0dBm)
    set_data_rate(DataRate::DATARATE_250K);
    set_power(Power::POWER_0DBM);

    // Enable 2-byte CRC
    set_crc(2);

    // Power up the module
    power_up();
}

void NRF24Driver::write(uint32_t id, const uint8_t *data, uint8_t len)
{
    // TODO: Implement write with ID filtering
    // For now, just send data through SPI
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&handle, (uint8_t *)data, len, HAL_MAX_DELAY);
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

void NRF24Driver::set_channel(uint8_t channel)
{
    write_register(0x05, channel);  // RF_CH register
}

// Private methods
void NRF24Driver::spi_select()
{
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);
}

void NRF24Driver::spi_deselect()
{
    HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);
}

uint8_t NRF24Driver::spi_transmit_receive(uint8_t data)
{
    uint8_t rx_data;
    spi_select();
    write(0, &data, 1);
    read(0, &rx_data, 1);
    spi_deselect();
    return rx_data;
}

void NRF24Driver::write_register(uint8_t reg, uint8_t value)
{
    uint8_t cmd[2] = {
        NRF24_CMD_W_REGISTER | (reg & 0x1F),
        value
    };
    spi_select();
    write(0, cmd, 2);
    spi_deselect();
}

uint8_t NRF24Driver::read_register(uint8_t reg)
{
    uint8_t cmd = NRF24_CMD_R_REGISTER | (reg & 0x1F);
    uint8_t result;
    spi_select();
    write(0, &cmd, 1);
    read(0, &result, 1);
    spi_deselect();
    return result;
}

void NRF24Driver::set_address_width(uint8_t width)
{
    if (width < 2 || width > 5) return;
    write_register(NRF24_REG_SETUP_AW, width - 2);
}

void NRF24Driver::set_data_rate(DataRate rate)
{
    uint8_t value = read_register(NRF24_REG_RF_SETUP);
    value &= ~0x28;  // Clear DR1 and DR0 bits
    value |= (static_cast<uint8_t>(rate) << 3) & 0x28;
    write_register(NRF24_REG_RF_SETUP, value);
}

void NRF24Driver::set_power(Power power)
{
    uint8_t value = read_register(NRF24_REG_RF_SETUP);
    value &= ~0x06;  // Clear RF_PWR1 and RF_PWR0 bits
    value |= (static_cast<uint8_t>(power) << 1) & 0x06;
    write_register(NRF24_REG_RF_SETUP, value);
}

void NRF24Driver::set_crc(uint8_t length)
{
    uint8_t config = read_register(NRF24_REG_CONFIG);
    config &= ~(NRF24_CONFIG_EN_CRC | NRF24_CONFIG_CRCO);
    if (length > 0) {
        config |= NRF24_CONFIG_EN_CRC;
        if (length > 1) {
            config |= NRF24_CONFIG_CRCO;
        }
    }
    write_register(NRF24_REG_CONFIG, config);
}

void NRF24Driver::enable_auto_ack(uint8_t pipe)
{
    uint8_t value = read_register(NRF24_REG_EN_AA);
    value |= (1 << pipe);
    write_register(NRF24_REG_EN_AA, value);
}

void NRF24Driver::disable_auto_ack(uint8_t pipe)
{
    uint8_t value = read_register(NRF24_REG_EN_AA);
    value &= ~(1 << pipe);
    write_register(NRF24_REG_EN_AA, value);
}

void NRF24Driver::enable_dynamic_payload(uint8_t pipe)
{
    uint8_t value = read_register(NRF24_REG_DYNPD);
    value |= (1 << pipe);
    write_register(NRF24_REG_DYNPD, value);
}

void NRF24Driver::set_retransmit(uint8_t count, uint8_t delay)
{
    uint8_t value = ((count & 0x0F) << 4) | (delay & 0x0F);
    write_register(NRF24_REG_SETUP_RETR, value);
}

void NRF24Driver::tx_payload(const uint8_t *data, uint8_t len)
{
    spi_select();
    uint8_t cmd = NRF24_CMD_W_TX_PAYLOAD;
    write(0, &cmd, 1);
    write(0, data, len);
    spi_deselect();
}

void NRF24Driver::flush_tx()
{
    uint8_t cmd = NRF24_CMD_FLUSH_TX;
    spi_select();
    write(0, &cmd, 1);
    spi_deselect();
}

void NRF24Driver::flush_rx()
{
    uint8_t cmd = NRF24_CMD_FLUSH_RX;
    spi_select();
    write(0, &cmd, 1);
    spi_deselect();
}

uint8_t NRF24Driver::rx_payload(uint8_t *data, uint8_t len)
{
    spi_select();
    uint8_t cmd = NRF24_CMD_R_RX_PAYLOAD;
    write(0, &cmd, 1);
    read(0, data, len);
    spi_deselect();
    return len;
}

uint8_t NRF24Driver::get_status()
{
    uint8_t cmd = NRF24_CMD_NOP;
    uint8_t status;
    spi_select();
    write(0, &cmd, 1);
    read(0, &status, 1);
    spi_deselect();
    return status;
}

uint8_t NRF24Driver::get_fifo_status()
{
    return read_register(0x17);  // FIFO_STATUS register
}

uint8_t NRF24Driver::get_observe_tx()
{
    return read_register(0x08);  // OBSERVE_TX register
}

uint8_t NRF24Driver::get_cd()
{
    return read_register(0x09);  // CD register
}

void NRF24Driver::power_up()
{
    uint8_t config = read_register(NRF24_REG_CONFIG);
    config |= NRF24_CONFIG_PWR_UP;
    write_register(NRF24_REG_CONFIG, config);
    HAL_Delay(2);  // Wait for PLL to lock
}

void NRF24Driver::power_down()
{
    uint8_t config = read_register(NRF24_REG_CONFIG);
    config &= ~NRF24_CONFIG_PWR_UP;
    write_register(NRF24_REG_CONFIG, config);
}

}  // namespace drivers::nrf24
