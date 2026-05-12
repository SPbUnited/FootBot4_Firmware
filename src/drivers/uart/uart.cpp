#include "uart.hpp"

#include "../system_clock/system_clock.hpp"
#include "kernel/scheduler/scheduler.hpp"

namespace drivers
{
extern uart::UartDriver uart4;
extern uart::UartDriver uart1;
}  // namespace drivers

extern "C" void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4)
    {
        drivers::uart4.hal_msp_init();
    }
    else if (huart->Instance == USART1)
    {
        drivers::uart1.hal_msp_init();
    }
}

extern "C" void UART4_IRQHandler(void)
{
    drivers::uart4.irq();
}

extern "C" void USART1_IRQHandler(void)
{
    drivers::uart1.irq();
}

extern "C" void DMA1_Stream2_IRQHandler(void)
{
    drivers::uart4.dma_rx_irq();
}

extern "C" void DMA1_Stream4_IRQHandler(void)
{
    drivers::uart4.dma_tx_irq();
}

extern "C" void DMA2_Stream5_IRQHandler(void)
{
    drivers::uart1.dma_rx_irq();
}

extern "C" void DMA2_Stream7_IRQHandler(void)
{
    drivers::uart1.dma_tx_irq();
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    drivers::uart4.hal_tx_cplt(huart);
    drivers::uart1.hal_tx_cplt(huart);
}

extern "C" void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    drivers::uart4.hal_error(huart);
    drivers::uart1.hal_error(huart);
}

namespace drivers::uart
{

void UartDriver::hal_msp_init()
{
    if (instance == UART4)
    {
        HAL_NVIC_SetPriority(UART4_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(UART4_IRQn);
    }
    else if (instance == USART1)
    {
        HAL_NVIC_SetPriority(USART1_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }

    hdma_rx_.Instance = dma_rx_stream;
    hdma_rx_.Init.Channel = dma_rx_channel;
    hdma_rx_.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_rx_.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_rx_.Init.MemInc = DMA_MINC_ENABLE;
    hdma_rx_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx_.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_rx_.Init.Mode = DMA_CIRCULAR;
    hdma_rx_.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_rx_.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    hdma_tx_.Instance = dma_tx_stream;
    hdma_tx_.Init.Channel = dma_tx_channel;
    hdma_tx_.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tx_.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tx_.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tx_.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx_.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_tx_.Init.Mode = DMA_NORMAL;
    hdma_tx_.Init.Priority = DMA_PRIORITY_MEDIUM;
    hdma_tx_.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    __HAL_LINKDMA(&handle, hdmarx, hdma_rx_);
    __HAL_LINKDMA(&handle, hdmatx, hdma_tx_);

    HAL_DMA_Init(&hdma_rx_);
    HAL_DMA_Init(&hdma_tx_);

    HAL_NVIC_SetPriority(dma_rx_irqn, 6, 0);
    HAL_NVIC_EnableIRQ(dma_rx_irqn);
    HAL_NVIC_SetPriority(dma_tx_irqn, 6, 0);
    HAL_NVIC_EnableIRQ(dma_tx_irqn);
}

void UartDriver::init()
{
    clk_enable();

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = txPin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = txAlternate;

    HAL_GPIO_Init(txPort, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = rxPin;
    GPIO_InitStruct.Alternate = rxAlternate;

    HAL_GPIO_Init(rxPort, &GPIO_InitStruct);

    handle.Instance = instance;

    handle.Init.BaudRate = baudrate;
    handle.Init.WordLength = wordLength;
    handle.Init.StopBits = stopBits;
    handle.Init.Parity = parity;
    handle.Init.HwFlowCtl = hwFlowControl;
    handle.Init.Mode = mode;
    handle.Init.OverSampling = overSampling;

    HAL_UART_Init(&handle);

    rx_dma_tail_ = 0;
    (void)HAL_UART_Receive_DMA(&handle, rx_dma_buf_, kRxDmaBufSize);
}

void UartDriver::irq()
{
    HAL_UART_IRQHandler(&handle);
}

void UartDriver::dma_rx_irq()
{
    HAL_DMA_IRQHandler(&hdma_rx_);
}

void UartDriver::dma_tx_irq()
{
    HAL_DMA_IRQHandler(&hdma_tx_);
}

void UartDriver::hal_tx_cplt(UART_HandleTypeDef *huart)
{
    if (huart != &handle)
    {
        return;
    }
    tx_dma_busy_ = false;
    tx_try_start_();
}

void UartDriver::hal_error(UART_HandleTypeDef *huart)
{
    if (huart != &handle)
    {
        return;
    }
    tx_dma_busy_ = false;
    HAL_UART_DMAStop(&handle);
    __HAL_UART_CLEAR_PEFLAG(&handle);
    __HAL_UART_CLEAR_FEFLAG(&handle);
    __HAL_UART_CLEAR_NEFLAG(&handle);
    __HAL_UART_CLEAR_OREFLAG(&handle);
    restart_rx_dma_();
    tx_try_start_();
}

void UartDriver::restart_rx_dma_()
{
    rx_dma_tail_ = 0;
    (void)HAL_UART_Receive_DMA(&handle, rx_dma_buf_, kRxDmaBufSize);
}

uint32_t UartDriver::rx_dma_head_() const
{
    return kRxDmaBufSize - __HAL_DMA_GET_COUNTER(&hdma_rx_);
}

HAL_StatusTypeDef UartDriver::read_dma_(uint8_t *out, uint16_t size, uint32_t timeout_us)
{
    const uint32_t deadline = drivers::system_clock::micros() + timeout_us;
    uint16_t got = 0;

    while (got < size)
    {
        uint16_t chunk = 0;
        __disable_irq();
        const uint32_t head = rx_dma_head_();
        uint32_t avail = (head + kRxDmaBufSize - rx_dma_tail_) % kRxDmaBufSize;
        if (avail > 0U)
        {
            chunk = static_cast<uint16_t>((avail < static_cast<uint32_t>(size - got)) ? avail : static_cast<uint32_t>(size - got));
            for (uint16_t i = 0; i < chunk; ++i)
            {
                out[got++] = rx_dma_buf_[rx_dma_tail_];
                rx_dma_tail_ = (rx_dma_tail_ + 1U) % kRxDmaBufSize;
            }
        }
        __enable_irq();

        if (chunk > 0U)
        {
            continue;
        }

        if (static_cast<int32_t>(deadline - drivers::system_clock::micros()) <= 0)
        {
            return HAL_TIMEOUT;
        }
        kernel::scheduler::yield();
    }
    return HAL_OK;
}

void UartDriver::tx_try_start_()
{
    if (tx_dma_busy_)
    {
        return;
    }
    if (handle.gState != HAL_UART_STATE_READY)
    {
        return;
    }
    if (tx_tail_ == tx_head_)
    {
        return;
    }

    uint16_t n = 0;
    uint32_t t = tx_tail_;
    while (n < kTxDmaChunk && t != tx_head_)
    {
        tx_dma_chunk_[n++] = tx_ring_[t];
        t = (t + 1U) % kTxRingSize;
    }
    if (n == 0)
    {
        return;
    }

    if (HAL_UART_Transmit_DMA(&handle, tx_dma_chunk_, n) != HAL_OK)
    {
        return;
    }
    tx_dma_busy_ = true;
    tx_tail_ = t;
}

void UartDriver::tx_push_bytes_(const uint8_t *data, uint16_t size)
{
    for (uint16_t i = 0; i < size; ++i)
    {
        for (;;)
        {
            uint32_t primask = __get_PRIMASK();
            __disable_irq();
            uint32_t next = (tx_head_ + 1U) % kTxRingSize;
            if (next != tx_tail_)
            {
                tx_ring_[tx_head_] = data[i];
                tx_head_ = next;
                __set_PRIMASK(primask);
                break;
            }
            __set_PRIMASK(primask);
            kernel::scheduler::yield();
        }
    }
}

void UartDriver::flush_tx()
{
    for (;;)
    {
        tx_try_start_();
        const bool queue_empty = (tx_head_ == tx_tail_);
        const bool hw_idle = !tx_dma_busy_ && (handle.gState == HAL_UART_STATE_READY);
        if (queue_empty && hw_idle)
        {
            break;
        }
        kernel::scheduler::yield();
    }
}

void UartDriver::putc(char c)
{
    tx_push_bytes_(reinterpret_cast<const uint8_t *>(&c), 1);
    tx_try_start_();
}

void UartDriver::write(const char *data, uint16_t size)
{
    tx_push_bytes_(reinterpret_cast<const uint8_t *>(data), size);
    tx_try_start_();
    flush_tx();
}

void UartDriver::printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

void UartDriver::vprintf(const char *fmt, va_list args)
{
    char buffer[128];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    write(buffer, static_cast<uint16_t>(strlen(buffer)));
}

HAL_StatusTypeDef UartDriver::getc(char *c)
{
    return read_dma_(reinterpret_cast<uint8_t *>(c), 1, 1000U);
}

HAL_StatusTypeDef UartDriver::read(char *data, uint16_t size)
{
    return read_dma_(reinterpret_cast<uint8_t *>(data), size, 250000U);
}

}  // namespace drivers::uart
