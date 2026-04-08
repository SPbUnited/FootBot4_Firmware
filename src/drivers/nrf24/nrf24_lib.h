#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

// NRF24L01+ Register Map
#define NRF24_REG_RF_SETUP    0x06
#define NRF24_REG_CONFIG      0x00
#define NRF24_REG_EN_AA       0x01
#define NRF24_REG_EN_RXADDR   0x02
#define NRF24_REG_SETUP_AW    0x03
#define NRF24_REG_SETUP_RETR  0x04
#define NRF24_REG_DYNPD       0x1C
#define NRF24_REG_FEATURE     0x1D

// NRF24 Commands
#define NRF24_CMD_R_REGISTER    0x00
#define NRF24_CMD_W_REGISTER    0x20
#define NRF24_CMD_R_RX_PAYLOAD  0x61
#define NRF24_CMD_W_TX_PAYLOAD  0xA0
#define NRF24_CMD_FLUSH_TX      0xE1
#define NRF24_CMD_FLUSH_RX      0xE2
#define NRF24_CMD_REUSE_TX_PL   0xE3
#define NRF24_CMD_NOP           0xFF

// Configuration bits
#define NRF24_CONFIG_PRIM_RX    0x01
#define NRF24_CONFIG_PWR_UP     0x02
#define NRF24_CONFIG_CRCO       0x04
#define NRF24_CONFIG_EN_CRC     0x08
#define NRF24_CONFIG_MAX_RT     0x10
#define NRF24_CONFIG_TX_DS      0x20
#define NRF24_CONFIG_RX_DR      0x40

// Default configuration values
#define NRF24_DEFAULT_EN_AA     0x3F  // Auto ACK on all pipes
#define NRF24_DEFAULT_EN_RXADDR 0x03  // Enable pipes 0 and 1
#define NRF24_DEFAULT_SETUP_AW  0x03  // 5-byte addresses
#define NRF24_DEFAULT_SETUP_RETR 0x03  // 3 retransmits, 250us delay
#define NRF24_DEFAULT_DYNPD     0x3F  // Dynamic payload on all pipes
#define NRF24_DEFAULT_FEATURE   0x06  // EN_DPL, EN_ACK_PAY

// Data rates
typedef enum {
    NRF24_DATARATE_250K = 0x00,
    NRF24_DATARATE_1M = 0x01,
    NRF24_DATARATE_2M = 0x02
} nrf24_datarate_t;

// Power levels
typedef enum {
    NRF24_POWER_M18DBM = 0x00,
    NRF24_POWER_M12DBM = 0x01,
    NRF24_POWER_M6DBM = 0x02,
    NRF24_POWER_0DBM = 0x03
} nrf24_power_t;

#ifdef __cplusplus
extern "C" {
#endif

// Low-level SPI operations (to be implemented by driver)
uint8_t nrf24_spi_transmit_receive(uint8_t data);
void nrf24_spi_select(void);
void nrf24_spi_deselect(void);

// High-level NRF24 functions
void nrf24_init(void);
void nrf24_write_register(uint8_t reg, uint8_t value);
uint8_t nrf24_read_register(uint8_t reg);
void nrf24_write_register_multi(uint8_t reg, const uint8_t *data, uint8_t len);
void nrf24_read_register_multi(uint8_t reg, uint8_t *data, uint8_t len);

// Configuration functions
void nrf24_set_address_width(uint8_t width);
void nrf24_set_data_rate(nrf24_datarate_t rate);
void nrf24_set_power(nrf24_power_t power);
void nrf24_set_crc(uint8_t length);
void nrf24_enable_auto_ack(uint8_t pipe);
void nrf24_disable_auto_ack(uint8_t pipe);
void nrf24_enable_dynamic_payload(uint8_t pipe);
void nrf24_set_retransmit(uint8_t count, uint8_t delay);
void nrf24_set_channel(uint8_t channel);

// TX/RX functions
void nrf24_tx_payload(const uint8_t *data, uint8_t len);
void nrf24_flush_tx(void);
void nrf24_flush_rx(void);
uint8_t nrf24_rx_payload(uint8_t *data, uint8_t len);
uint8_t nrf24_get_status(void);
uint8_t nrf24_get_fifo_status(void);
uint8_t nrf24_get_observe_tx(void);
uint8_t nrf24_get_cd(void);

// Power management
void nrf24_power_up(void);
void nrf24_power_down(void);

#ifdef __cplusplus
}
#endif
