#include "buzzer.hpp"

extern "C" void TIM7_IRQHandler(void)
{
    TIM7->SR = ~TIM_SR_UIF;
    // BSRR instead of ODR ^=, so the kicker pins on GPIOF never get a read-modify-write
    GPIOF->BSRR = (GPIOF->ODR & GPIO_PIN_13) ? (uint32_t)GPIO_PIN_13 << 16 : GPIO_PIN_13;
}

namespace drivers::buzzer
{

void Buzzer::init()
{
    __HAL_RCC_GPIOF_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);

    // APB1 is divided, so its timers run at twice PCLK1
    __HAL_RCC_TIM7_CLK_ENABLE();
    TIM7->PSC = 2 * HAL_RCC_GetPCLK1Freq() / 1000000 - 1;  // 1 MHz
    TIM7->ARR = 1000000 / (2 * kToneFreq_Hz) - 1;            // update on every half-period
    TIM7->EGR = TIM_EGR_UG;
    TIM7->SR = 0;
    TIM7->DIER = TIM_DIER_UIE;
    HAL_NVIC_SetPriority(TIM7_IRQn, 15, 0);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);
}

void Buzzer::set(bool on)
{
    if (on == m_on)
    {
        return;
    }
    m_on = on;
    if (on)
    {
        TIM7->CR1 |= TIM_CR1_CEN;
    }
    else
    {
        TIM7->CR1 &= ~TIM_CR1_CEN;
        TIM7->SR = 0;
        HAL_NVIC_ClearPendingIRQ(TIM7_IRQn);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
    }
}

}  // namespace drivers::buzzer
