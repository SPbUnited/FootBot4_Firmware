#include "stm32f4xx_hal.h"

#include "LedManager.h"

extern "C" void SysTick_Handler(void)
{
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

int main(void)
{
    HAL_Init();

    LEDMGR::init_leds();

    turn_on_robot();
    while (1)
    {
        for (size_t i = 0; i < LEDMGR::NUM_LEDS; i++)
        {
            LEDMGR::toggle_led(LEDMGR::Leds(i));
            HAL_Delay(100);
        }
    }
}
