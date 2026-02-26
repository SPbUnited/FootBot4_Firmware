#include "stm32f4xx_hal.h"

#define USARTx USART1

#define USARTx_TX_PIN GPIO_PIN_9
#define USARTx_TX_GPIO_PORT GPIOA
#define USARTx_TX_AF GPIO_AF7_USART1
#define USARTx_RX_PIN GPIO_PIN_10
#define USARTx_RX_GPIO_PORT GPIOA
#define USARTx_RX_AF GPIO_AF7_USART1

#define USARTx_CLK_ENABLE() __HAL_RCC_USART1_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

static UART_HandleTypeDef Usart1Handle;

void initUart1(){
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
    Usart1Handle.Instance = USARTx;

    Usart1Handle.Init.BaudRate = 115200;
    Usart1Handle.Init.WordLength = UART_WORDLENGTH_8B;
    Usart1Handle.Init.StopBits = UART_STOPBITS_1;
    Usart1Handle.Init.Parity = UART_PARITY_NONE;
    Usart1Handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    Usart1Handle.Init.Mode = UART_MODE_TX_RX;
    Usart1Handle.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&Usart1Handle);
}

void Uart1OutputChar(char c)
{
  HAL_UART_Transmit(&Usart1Handle, (uint8_t *)(&c), 1, 1000);
}

