#include "nrf24_lib.h"
#include "nrf24.hpp"

// External references to driver instance
extern drivers::nrf24::NRF24Driver nrf24;

// Low-level SPI operations using the driver
uint8_t nrf24_spi_transmit_receive(uint8_t data)
{
    uint8_t rx_data;
    nrf24_spi_select();
    nrf24.write(0, &data, 1);
    nrf24.read(0, &rx_data, 1);
    nrf24_spi_deselect();
    return rx_data;
}

void nrf24_spi_select(void)
{
    HAL_GPIO_WritePin(nrf24.csPort, nrf24.csPin, GPIO_PIN_RESET);
}

void nrf24_spi_deselect(void)
{
    HAL_GPIO_WritePin(nrf24.csPort, nrf24.csPin, GPIO_PIN_SET);
}

void nrf24_write_register(uint8_t reg, uint8_t value)
{
    uint8_t cmd[2] = {
        NRF24_CMD_W_REGISTER | (reg & 0x1F),
        value
    };
    nrf24_spi_select();
    nrf24.write(0, cmd, 2);
    nrf24_spi_deselect();
}

uint8_t nrf24_read_register(uint8_t reg)
{
    uint8_t cmd = NRF24_CMD_R_REGISTER | (reg & 0x1F);
    uint8_t result;
    nrf24_spi_select();
    nrf24.write(0, &cmd, 1);
    nrf24.read(0, &result, 1);
    nrf24_spi_deselect();
    return result;
}

void nrf24_write_register_multi(uint8_t reg, const uint8_t *data, uint8_t len)
{
    uint8_t *cmd = (uint8_t *)malloc(len + 1);
    cmd[0] = NRF24_CMD_W_REGISTER | (reg & 0x1F);
    memcpy(cmd + 1, data, len);
    nrf24_spi_select();
    nrf24.write(0, cmd, len + 1);
    nrf24_spi_deselect();
    free(cmd);
}

void nrf24_read_register_multi(uint8_t reg, uint8_t *data, uint8_t len)
{
    uint8_t cmd = NRF24_CMD_R_REGISTER | (reg & 0x1F);
    nrf24_spi_select();
    nrf24.write(0, &cmd, 1);
    nrf24.read(0, data, len);
    nrf24_spi_deselect();
}

void nrf24_init(void)
{
    // Wait for module to stabilize
    HAL_Delay(5);

    // Configure basic settings
    nrf24_write_register(NRF24_REG_EN_AA, NRF24_DEFAULT_EN_AA);
    nrf24_write_register(NRF24_REG_EN_RXADDR, NRF24_DEFAULT_EN_RXADDR);
    nrf24_write_register(NRF24_REG_SETUP_AW, NRF24_DEFAULT_SETUP_AW);
    nrf24_write_register(NRF24_REG_SETUP_RETR, NRF24_DEFAULT_SETUP_RETR);
    nrf24_write_register(NRF24_REG_DYNPD, NRF24_DEFAULT_DYNPD);
    nrf24_write_register(NRF24_REG_FEATURE, NRF24_DEFAULT_FEATURE);

    // Set default data rate (250kbps) and power (0dBm)
    nrf24_set_data_rate(NRF24_DATARATE_250K);
    nrf24_set_power(NRF24_POWER_0DBM);

    // Enable 2-byte CRC
    nrf24_set_crc(2);

    // Power up the module
    nrf24_power_up();
}

void nrf24_set_address_width(uint8_t width)
{
    if (width < 2 || width > 5) return;
    nrf24_write_register(NRF24_REG_SETUP_AW, width - 2);
}

void nrf24_set_data_rate(nrf24_datarate_t rate)
{
    uint8_t value = nrf24_read_register(NRF24_REG_RF_SETUP);
    value &= ~0x28;  // Clear DR1 and DR0 bits
    value |= (rate << 3) & 0x28;
    nrf24_write_register(NRF24_REG_RF_SETUP, value);
}

void nrf24_set_power(nrf24_power_t power)
{
    uint8_t value = nrf24_read_register(NRF24_REG_RF_SETUP);
    value &= ~0x06;  // Clear RF_PWR1 and RF_PWR0 bits
    value |= (power << 1) & 0x06;
    nrf24_write_register(NRF24_REG_RF_SETUP, value);
}

void nrf24_set_crc(uint8_t length)
{
    uint8_t config = nrf24_read_register(NRF24_REG_CONFIG);
    config &= ~(NRF24_CONFIG_EN_CRC | NRF24_CONFIG_CRCO);
    if (length > 0) {
        config |= NRF24_CONFIG_EN_CRC;
        if (length > 1) {
            config |= NRF24_CONFIG_CRCO;
        }
    }
    nrf24_write_register(NRF24_REG_CONFIG, config);
}

void nrf24_enable_auto_ack(uint8_t pipe)
{
    uint8_t value = nrf24_read_register(NRF24_REG_EN_AA);
    value |= (1 << pipe);
    nrf24_write_register(NRF24_REG_EN_AA, value);
}

void nrf24_disable_auto_ack(uint8_t pipe)
{
    uint8_t value = nrf24_read_register(NRF24_REG_EN_AA);
    value &= ~(1 << pipe);
    nrf24_write_register(NRF24_REG_EN_AA, value);
}

void nrf24_enable_dynamic_payload(uint8_t pipe)
{
    uint8_t value = nrf24_read_register(NRF24_REG_DYNPD);
    value |= (1 << pipe);
    nrf24_write_register(NRF24_REG_DYNPD, value);
}

void nrf24_set_retransmit(uint8_t count, uint8_t delay)
{
    uint8_t value = ((count & 0x0F) << 4) | (delay & 0x0F);
    nrf24_write_register(NRF24_REG_SETUP_RETR, value);
}

void nrf24_tx_payload(const uint8_t *data, uint8_t len)
{
    nrf24_spi_select();
    uint8_t cmd = NRF24_CMD_W_TX_PAYLOAD;
    nrf24.write(0, &cmd, 1);
    nrf24.write(0, data, len);
    nrf24_spi_deselect();
}

void nrf24_flush_tx(void)
{
    uint8_t cmd = NRF24_CMD_FLUSH_TX;
    nrf24_spi_select();
    nrf24.write(0, &cmd, 1);
    nrf24_spi_deselect();
}

void nrf24_flush_rx(void)
{
    uint8_t cmd = NRF24_CMD_FLUSH_RX;
    nrf24_spi_select();
    nrf24.write(0, &cmd, 1);
    nrf24_spi_deselect();
}

uint8_t nrf24_rx_payload(uint8_t *data, uint8_t len)
{
    nrf24_spi_select();
    uint8_t cmd = NRF24_CMD_R_RX_PAYLOAD;
    nrf24.write(0, &cmd, 1);
    nrf24.read(0, data, len);
    nrf24_spi_deselect();
    return len;
}

uint8_t nrf24_get_status(void)
{
    uint8_t cmd = NRF24_CMD_NOP;
    uint8_t status;
    nrf24_spi_select();
    nrf24.write(0, &cmd, 1);
    nrf24.read(0, &status, 1);
    nrf24_spi_deselect();
    return status;
}

uint8_t nrf24_get_fifo_status(void)
{
    return nrf24_read_register(0x17);  // FIFO_STATUS register
}

uint8_t nrf24_get_observe_tx(void)
{
    return nrf24_read_register(0x08);  // OBSERVE_TX register
}

uint8_t nrf24_get_cd(void)
{
    return nrf24_read_register(0x09);  // CD register
}

void nrf24_power_up(void)
{
    uint8_t config = nrf24_read_register(NRF24_REG_CONFIG);
    config |= NRF24_CONFIG_PWR_UP;
    nrf24_write_register(NRF24_REG_CONFIG, config);
    HAL_Delay(2);  // Wait for PLL to lock
}

void nrf24_power_down(void)
{
    uint8_t config = nrf24_read_register(NRF24_REG_CONFIG);
    config &= ~NRF24_CONFIG_PWR_UP;
    nrf24_write_register(NRF24_REG_CONFIG, config);
}

void nrf24_set_channel(uint8_t channel)
{
    nrf24_write_register(0x05, channel);  // RF_CH register
}
