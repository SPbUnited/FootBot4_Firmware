// #pragma once
// #include <stdio.h>

// #include <logging/log.h>
// #include <zephyr/device.h>
// #include <zephyr/drivers/gpio.h>
// #include <zephyr/drivers/spi.h>
// #include <zephyr/sys/util.h>
// #include <zephyr/zephyr.h>

// // #include <limits>
// #include "../nrf24/nrf24.hpp"
// #include "../kicker/kicker.h"
// #include "../led_display/led_display.h"
// #include "../motion_logic/motion_logic.hpp"
// #include "../nrf24/nrf24Reg.hpp"
// #include "../cannabus/cannabus.hpp"

// #define SPI_OP SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_LINES_SINGLE | SPI_TRANSFER_MSB

// // static const struct gpio_dt_spec cePin = GPIO_DT_SPEC_GET(DT_N_ALIAS_ce, gpios);

// // const struct spi_cs_control cs_pin = {
// //     .gpio = SPI_CS_GPIOS_DT_SPEC_GET(DT_NODELABEL(nrf24l01)),
// //     .delay = (0),
// // };

// // const struct spi_cs_control cs_pin_recv = {
// //     .gpio = SPI_CS_GPIOS_DT_SPEC_GET(DT_NODELABEL(nrf24l01_recv)),
// //     .delay = (0),
// // };

// // const struct spi_dt_spec nrf24 = {
// //     .bus = DEVICE_DT_GET(DT_NODELABEL(spi2)),
// //     .config =
// //         {
// //             .frequency = DT_PROP(DT_NODELABEL(nrf24l01), spi_max_frequency),
// //             .operation = SPI_OP,
// //             .cs = &cs_pin,
// //         },
// // };

// // static const struct gpio_dt_spec cePin_recv = GPIO_DT_SPEC_GET(DT_N_ALIAS_ce_recv, gpios);

// // const struct spi_dt_spec nrf24_recv = {
// //     .bus = DEVICE_DT_GET(DT_NODELABEL(spi2)),
// //     .config =
// //         {
// //             .frequency = DT_PROP(DT_NODELABEL(nrf24l01_recv), spi_max_frequency),
// //             .operation = SPI_OP,
// //             .cs = &cs_pin_recv,
// //         },
// // };

// class Nrf24Recv
// {
// private:
//     static int spiUpdate_reg(uint8_t reg_addr, uint8_t mask, uint8_t value)
//     {
//         uint8_t tmp_val;

//         rawRead(reg_addr, &tmp_val, 1);
//         tmp_val = (tmp_val & ~mask) | (value & mask);

//         return rawWrite(reg_addr, &tmp_val, 1);
//     }
//     static int rawRead(uint8_t reg_addr, uint8_t *value, uint8_t len)
//     {
//         uint8_t buffer_tx[2] = {reg_addr, NRF24_CMD_NOP};
//         uint8_t buffer_rx[10] = {0};

//         const struct spi_buf tx_buf[2] = {
//             {
//                 .buf = buffer_tx,
//                 .len = sizeof(buffer_tx),
//             }};

//         const struct spi_buf rx_buf[2] = {
//             {
//                 .buf = buffer_rx,
//                 .len = len + 1,
//             }};

//         const struct spi_buf_set tx = {
//             .buffers = tx_buf,
//             .count = 1};

//         const struct spi_buf_set rx = {
//             .buffers = rx_buf,
//             .count = 1};

//         if (len > 64)
//         {
//             return -EIO;
//         }
//         int res = spi_transceive_dt(spec, &tx, &rx);
//         if (res < 0)
//         {
//             return -EIO;
//         }

//         uint8_t *value_p = value;
//         for (uint8_t i = 0; i < len; ++i)
//         {
//             *value_p = buffer_rx[len - i];
//             value_p++;
//         }

//         // memcpy(value, buffer_rx + 1, len);
//         return 0;
//     }
//     static int readReg(uint8_t reg_addr, uint8_t *value)
//     {
//         return rawRead(reg_addr, value, 1);
//     }

//     static int writeReg(uint8_t reg_addr, uint8_t value)
//     {
//         return rawWrite(reg_addr, &value, 1);
//     }
//     static int rawWrite(uint8_t reg_addr, uint8_t *value, uint8_t len)
//     {
//         uint8_t buffer_tx;
//         if (reg_addr < NRF24_CMD_W_REGISTER)
//         {
//             buffer_tx = NRF24_CMD_W_REGISTER | (reg_addr & NRF24_MASK_REG_MAP);
//         }
//         else if (reg_addr != NRF24_CMD_W_TX_PAYLOAD)
//         {
//             buffer_tx = reg_addr;
//             if ((reg_addr != NRF24_CMD_FLUSH_TX) &&
//                 (reg_addr != NRF24_CMD_FLUSH_RX) &&
//                 (reg_addr != NRF24_CMD_REUSE_TX_PL) &&
//                 (reg_addr != NRF24_CMD_NOP))
//             {
//                 value = NULL;
//             }
//         }
//         else
//         {
//             buffer_tx = reg_addr;
//         }
//         const struct spi_buf tx_buf[2] = {
//             {
//                 .buf = &buffer_tx,
//                 .len = 1,
//             },
//             {
//                 .buf = value, // value, //&arra, // value,
//                 .len = len,   // len,
//             }};
//         const struct spi_buf_set tx = {
//             .buffers = tx_buf,
//             .count = 2};

//         if (len > 64)
//         {
//             return -EIO;
//         }

//         if (spi_write_dt(spec, &tx))
//         {
//             printk("error to transmit");
//             return -EIO;
//         }

//         return 0;
//     }

//     void
//     setCe()
//     {
//         gpio_pin_set_dt(m_chip_enable_pin, 1);
//     };
//     void resetCe() { gpio_pin_set_dt(m_chip_enable_pin, 0); };

//     static const struct spi_dt_spec *spec;

//     K_KERNEL_STACK_MEMBER(thread_stack, 8192);
//     struct k_thread thread;

//     static uint8_t m_address;
//     static bool m_isNewPacketRec;
//     uint8_t m_debugByte = 0;
//     static volatile uint32_t m_lastPacketTime;

// public:
//     static const uint32_t m_incomePacketLen = 6;
//     static uint8_t m_incomeArray[m_incomePacketLen];
//     static uint8_t m_iArray[8];
//     uint8_t barrier = 0;

// private:
//     const struct gpio_dt_spec *m_chip_enable_pin;

//     uint8_t m_txAddressWidth{3};
//     uint8_t m_txPayLoadWidth{2};

//     static uint8_t m_txAddress[6];
//     static uint32_t m_prevTime_ms;
//     static uint32_t m_packetsReceived;
//     static uint32_t m_packetsReceived_1s;
//     static uint32_t m_ownPacketsReceived;
//     static uint32_t m_ownPacketsReceived_1s;
//     static uint8_t m_lenDbg;

//     static motion::Control *m_motionControl;
//     static kicker::Kicker *m_kicker;
//     static LedDisplay *m_display;
//     static canbroadcast::CanBroadcast *m_cannabus;
//     // static uint8_t *m_barrier;

//     static const uint32_t m_wpOpCodePos = 5;
//     static const uint32_t m_wpXSpeedPos = 0;
//     static const uint32_t m_wpYSpeedPos = 1;
//     static const uint32_t m_wpRSpeedPos = 2;
//     static const uint32_t m_wpKvlSpdPos = 3;
//     static const uint32_t m_wpFlagsPos = 4;

//     static const uint8_t m_wpLed = 0x80;
//     static const uint8_t m_wpFrcKickForward = 0x40;
//     static const uint8_t m_wpFrcKickUp = 0x20;
//     static const uint8_t m_wpKupMask = 0x10;
//     static const uint8_t m_wpKforMask = 0x08;
//     static const uint8_t m_wpBspMask = 0x04;
//     static const uint8_t m_wpDrbenMask = 0x02;
//     static const uint8_t m_wpKceMask = 0x01;

//     static const uint8_t m_kvlMask = 0xF0;
//     static const uint8_t m_spdMask = 0x0F;

// public:
//     Nrf24Recv(const struct spi_dt_spec *_spec, motion::Control *_motionControl, kicker::Kicker *_kicker, canbroadcast::CanBroadcast *_cannabus, const struct gpio_dt_spec *_chip_enable_pin, bool send_or_recieve); //, LedDisplay *_display = NULL);//, uint8_t *barrier);

//     int recv();
//     void send(uint8_t checker, uint8_t id);
//     void init();

//     void toggleFeatures();

//     bool isConnected(void);
//     // Function prototypes
//     void setPowerMode(Nrf24Power mode);
//     void setOperationMode(Nrf24OperationMode mode);
//     void setCrcScheme(Nrf24Crc scheme);

//     void setRfChannel(uint8_t channel);
//     void setAutoRetr(Nrf24SetupRetry delay, uint8_t count);
//     void setAddrWidth(Nrf24SetupAddressWidth addrWidth);
//     void setAddr(Nrf24RxpipeAddresses pipe, const uint8_t *addr);
//     void setTxPower(Nrf24RfPower txPwr);
//     void setDataRate(Nrf24DataRate dataRate);

//     void setRxPipe(Nrf24RxpipeAddresses pipe, uint8_t aaState, uint8_t payloadLen);
//     void closePipe(Nrf24RxpipeAddresses pipe);
//     void enableAa(Nrf24RxpipeAddresses pipe);
//     void disableAa(Nrf24RxpipeAddresses pipe);

//     static int8_t u8Toi8(uint8_t x);

//     uint8_t getStatus(void);
//     Nrf24IrqStatus getIrqStatus(void);
//     uint8_t getStatusRxFIFO(void);
//     uint8_t getStatusTxFIFO(void);
//     uint8_t getRxSource(void);
//     uint8_t getRetransmitCounters(void);
//     uint8_t getRpd(void);

//     void resetPLOS(void);
//     void flushTx(void);
//     static void flushRx(void);
//     void clearIrqFlags(void);

//     void writePayload(uint8_t *pBuf, uint8_t length);
//     Nrf24RxResult readPayload(uint8_t *pBuf, uint8_t *length);
// };