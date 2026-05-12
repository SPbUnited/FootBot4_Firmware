#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <cstdint>

#include "stm32f4xx_hal.h"

namespace drivers::uart
{

/** STM32F429: UART4 RX DMA1 Stream2 Ch4, TX Stream4 Ch4; USART1 RX DMA2 Stream5 Ch4, TX Stream7 Ch4. */
struct UartConfig
{
    USART_TypeDef *instance;
    void (*clk_enable)();

    DMA_Stream_TypeDef *dma_rx_stream;
    IRQn_Type dma_rx_irqn;
    uint32_t dma_rx_channel;

    DMA_Stream_TypeDef *dma_tx_stream;
    IRQn_Type dma_tx_irqn;
    uint32_t dma_tx_channel;

    uint32_t txPin;
    GPIO_TypeDef *txPort;
    uint32_t txAlternate;
    uint32_t rxPin;
    GPIO_TypeDef *rxPort;
    uint32_t rxAlternate;

    uint32_t baudrate;
    uint32_t wordLength;
    uint32_t stopBits;
    uint32_t parity;
    uint32_t hwFlowControl;
    uint32_t mode;
    uint32_t overSampling;
};

class UartDriver : public UartConfig
{
  public:
    static constexpr size_t kTxRingSize = 512;
    static constexpr size_t kTxDmaChunk = 64;
    static constexpr size_t kRxDmaBufSize = 512;

  private:
    UART_HandleTypeDef handle{};
    DMA_HandleTypeDef hdma_rx_{};
    DMA_HandleTypeDef hdma_tx_{};

    uint8_t rx_dma_buf_[kRxDmaBufSize]{};
    volatile uint32_t rx_dma_tail_{0};

    uint8_t tx_ring_[kTxRingSize]{};
    volatile uint32_t tx_head_{0};
    volatile uint32_t tx_tail_{0};
    uint8_t tx_dma_chunk_[kTxDmaChunk]{};
    volatile bool tx_dma_busy_{false};

    void tx_try_start_();
    void tx_push_bytes_(const uint8_t *data, uint16_t size);
    uint32_t rx_dma_head_() const;
    HAL_StatusTypeDef read_dma_(uint8_t *out, uint16_t size, uint32_t timeout_us);
    void restart_rx_dma_();

  public:
    explicit UartDriver(UartConfig config) : UartConfig(config) {}

    void init();

    void hal_msp_init();

    void flush_tx();

    void irq();
    void dma_rx_irq();
    void dma_tx_irq();

    void putc(char c);
    void write(const char *data, uint16_t size);
    void printf(const char *fmt, ...);
    void vprintf(const char *fmt, va_list args);

    HAL_StatusTypeDef getc(char *c);
    HAL_StatusTypeDef read(char *data, uint16_t size);

    void hal_tx_cplt(UART_HandleTypeDef *huart);
    void hal_error(UART_HandleTypeDef *huart);
};

}  // namespace drivers::uart
