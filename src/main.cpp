#include "stm32f4xx_hal.h"

#define LED_PIN         GPIO_PIN_4
#define LED_GPIO_PORT   GPIOG
#define LED_GPIO_CLK_ENABLE() __HAL_RCC_GPIOG_CLK_ENABLE()

extern "C" void SysTick_Handler(void) {
    HAL_IncTick();
}

void turn_on_robot()
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
}

void init_led(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    LED_GPIO_CLK_ENABLE(); //Скорее всего надо сделать switch case для всех GPIO
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);

}

int main(void) {
    HAL_Init();
    

    init_led(LED_GPIO_PORT, LED_PIN);
    init_led(LED_GPIO_PORT, GPIO_PIN_3);

    HAL_GPIO_WritePin(LED_GPIO_PORT, GPIO_PIN_3, GPIO_PIN_SET);
    turn_on_robot();
    while (1) {

        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
        HAL_GPIO_TogglePin(LED_GPIO_PORT, GPIO_PIN_3);
    
        HAL_Delay(500);
    }
}
