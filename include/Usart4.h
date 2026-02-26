#include "stm32f4xx_hal.h"

#define USARTx UART4

#define USARTx_TX_PIN GPIO_PIN_0
#define USARTx_TX_GPIO_PORT GPIOA
#define USARTx_TX_AF GPIO_AF8_UART4
#define USARTx_RX_PIN GPIO_PIN_1
#define USARTx_RX_GPIO_PORT GPIOA
#define USARTx_RX_AF GPIO_AF8_UART4

#define USARTx_CLK_ENABLE() __HAL_RCC_UART4_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

static UART_HandleTypeDef Uart4Handle;

void initUsart4(){
    GPIO_InitTypeDef GPIO_InitStruct;

    USARTx_TX_GPIO_CLK_ENABLE();
    USARTx_RX_GPIO_CLK_ENABLE();

    USARTx_CLK_ENABLE();

    GPIO_InitStruct.Pin = USARTx_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = USARTx_TX_AF;

    HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = USARTx_RX_PIN;
    GPIO_InitStruct.Alternate = USARTx_RX_AF;

    HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
    Uart4Handle.Instance = USARTx;

    Uart4Handle.Init.BaudRate = 115200;
    Uart4Handle.Init.WordLength = UART_WORDLENGTH_8B;
    Uart4Handle.Init.StopBits = UART_STOPBITS_1;
    Uart4Handle.Init.Parity = UART_PARITY_NONE;
    Uart4Handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    Uart4Handle.Init.Mode = UART_MODE_TX_RX;
    Uart4Handle.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&Uart4Handle);
}

void Uart4OutputChar(char c)
{
  HAL_UART_Transmit(&Uart4Handle, (uint8_t *)(&c), 1, 1000);
}

