#pragma once

#include <inttypes.h>

#include "stm32f4xx_hal.h"

namespace drivers::nrf24
{

struct NRF24Config
{
    SPI_TypeDef *instance;
    //
    void (*clk_enable)();

    // SPI pins
    uint32_t sckPin;
    GPIO_TypeDef *sckPort;
    uint32_t sckAlternate;
    uint32_t misoPin;
    GPIO_TypeDef *misoPort;
    uint32_t misoAlternate;
    uint32_t mosiPin;
    GPIO_TypeDef *mosiPort;
    uint32_t mosiAlternate;

    // Chip Enable pin
    uint32_t cePin;
    GPIO_TypeDef *cePort;

    // Chip Select pin
    uint32_t csPin;
    GPIO_TypeDef *csPort;

    // IRQ pin (optional)
    uint32_t irqPin;
    GPIO_TypeDef *irqPort;
};

class NRF24Driver : public NRF24Config
{
  private:
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

    // Configuration bits
    static constexpr uint8_t NRF24_CONFIG_PRIM_RX = 0x01;
    static constexpr uint8_t NRF24_CONFIG_PWR_UP = 0x02;
    static constexpr uint8_t NRF24_CONFIG_CRCO = 0x04;
    static constexpr uint8_t NRF24_CONFIG_EN_CRC = 0x08;
    static constexpr uint8_t NRF24_CONFIG_MAX_RT = 0x10;
    static constexpr uint8_t NRF24_CONFIG_TX_DS = 0x20;
    static constexpr uint8_t NRF24_CONFIG_RX_DR = 0x40;

    // Default configuration values
    static constexpr uint8_t NRF24_DEFAULT_EN_AA = 0x3F;
    static constexpr uint8_t NRF24_DEFAULT_EN_RXADDR = 0x03;
    static constexpr uint8_t NRF24_DEFAULT_SETUP_AW = 0x03;
    static constexpr uint8_t NRF24_DEFAULT_SETUP_RETR = 0x03;
    static constexpr uint8_t NRF24_DEFAULT_DYNPD = 0x3F;
    static constexpr uint8_t NRF24_DEFAULT_FEATURE = 0x06;

    // Data rates
    enum class DataRate : uint8_t
    {
        DATARATE_250K = 0x00,
        DATARATE_1M = 0x01,
        DATARATE_2M = 0x02
    };

    // Power levels
    enum class Power : uint8_t
    {
        POWER_M18DBM = 0x00,
        POWER_M12DBM = 0x01,
        POWER_M6DBM = 0x02,
        POWER_0DBM = 0x03
    };

    // Low-level SPI operations
    void spi_select();
    void spi_deselect();
    uint8_t spi_transmit_receive(uint8_t data);

    // Register operations
    void write_register(uint8_t reg, uint8_t value);
    uint8_t read_register(uint8_t reg);

  public:
    SPI_HandleTypeDef handle;

    NRF24Driver(NRF24Config config);

    void init();
    void init_nrf24();
    void set_channel(uint8_t channel);
    void write(uint32_t id, const uint8_t *data, uint8_t len);
    void read(uint32_t id, uint8_t *data, uint8_t len);
    void setReg(uint32_t reg, uint32_t value);

    // Configuration methods
    void set_address_width(uint8_t width);
    void set_data_rate(DataRate rate);
    void set_power(Power power);
    void set_crc(uint8_t length);
    void enable_auto_ack(uint8_t pipe);
    void disable_auto_ack(uint8_t pipe);
    void enable_dynamic_payload(uint8_t pipe);
    void set_retransmit(uint8_t count, uint8_t delay);

    // TX/RX methods
    void tx_payload(const uint8_t *data, uint8_t len);
    void flush_tx();
    void flush_rx();
    uint8_t rx_payload(uint8_t *data, uint8_t len);
    uint8_t get_status();
    uint8_t get_fifo_status();
    uint8_t get_observe_tx();
    uint8_t get_cd();

    // Power management
    void power_up();
    void power_down();
};

}  // namespace drivers::nrf24
