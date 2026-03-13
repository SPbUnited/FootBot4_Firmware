#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <cstdint>

#ifndef NATIVE

#include "stm32f4xx_hal.h"

namespace drivers::uart
{

struct UartConfig
{
    USART_TypeDef *instance;
    //
    void (*clk_enable)();
    //
    uint32_t txPin;
    GPIO_TypeDef *txPort;
    uint32_t txAlternate;
    uint32_t rxPin;
    GPIO_TypeDef *rxPort;
    uint32_t rxAlternate;
    //
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
  private:
    UART_HandleTypeDef handle;

  public:
    UartDriver(UartConfig config) : UartConfig(config) {}

    void init()
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
    }

    void putc(char c)
    {
        HAL_UART_Transmit(&handle, (uint8_t *)(&c), 1, 1000);
    }

    void write(const char *data, uint16_t size)
    {
        HAL_UART_Transmit(&handle, (uint8_t *)data, size, 1000);
    }

    void printf(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        char buffer[128];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        HAL_UART_Transmit(&handle, (uint8_t *)(&buffer), strlen(buffer), 1000);
    }

    void vprintf(const char *fmt, va_list args)
    {
        char buffer[128];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        HAL_UART_Transmit(&handle, (uint8_t *)(&buffer), strlen(buffer), 1000);
    }

    HAL_StatusTypeDef getc(char *c)
    {
        return HAL_UART_Receive(&handle, (uint8_t *)c, 1, 100);
    }
};

}  // namespace drivers::uart

#else

namespace drivers::uart
{

struct UartConfig
{
};

class UartDriver : public UartConfig
{
  private:
  public:
    UartDriver(UartConfig config) {}

    void init() {}

    void putc(char c)
    {
        ::printf("%c", c);
    }

    void write(const char *data, uint16_t size)
    {
        ::printf("%s", data);
    }

    void printf(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        char buffer[128];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        ::printf("%s", buffer);
    }

    void vprintf(const char *fmt, va_list args)
    {
        char buffer[128];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        ::printf("%s", buffer);
    }

    int getc(char *c)
    {
        *c = ::getchar();
        return 0;
    }
};

}  // namespace drivers::uart

#endif
