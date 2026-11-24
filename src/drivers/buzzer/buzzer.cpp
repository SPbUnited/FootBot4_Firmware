#include "buzzer.hpp"

#include "drivers/system_clock/system_clock.hpp"

namespace drivers::buzzer
{
void Buzzer::set_drive_alarm(bool active)
{
    static bool playing = false;
    static uint32_t last_fault_ms = 0;
    constexpr uint32_t recovery_ms = 3000;
    const uint32_t now = HAL_GetTick();
    if (active)
        last_fault_ms = now;
    else if (playing && now - last_fault_ms < recovery_ms)
        return;

    if (active == playing)
        return;
    playing = active;

    if (active)
    {
        __HAL_RCC_TIM7_CLK_ENABLE();
        // APB1 prescaler is 4, so TIM7 runs at twice PCLK1.
        TIM7->PSC = 2U * HAL_RCC_GetPCLK1Freq() / 1000000U - 1U;
        TIM7->ARR = 902U - 1U;  // Half-period of D-flat 5 (554.37 Hz).
        TIM7->EGR = TIM_EGR_UG;
        TIM7->SR = 0;
        TIM7->DIER = TIM_DIER_UIE;
        HAL_NVIC_SetPriority(TIM7_IRQn, 10, 0);
        HAL_NVIC_ClearPendingIRQ(TIM7_IRQn);
        HAL_NVIC_EnableIRQ(TIM7_IRQn);
        TIM7->CR1 = TIM_CR1_CEN;
    }
    else
    {
        HAL_NVIC_DisableIRQ(TIM7_IRQn);
        TIM7->CR1 = 0;
        TIM7->DIER = 0;
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
    }
}

extern "C" void TIM7_IRQHandler()
{
    TIM7->SR = 0;
    HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_13);
}

void Buzzer::play_boot_melody()
{
    struct Note
    {
        uint16_t frequency_hz;
        uint16_t duration_ms;
    };
    constexpr Note melody[] = {
        {196, 100}, {494, 100}, {294, 100}, {370, 100}, {392, 100}, {784, 200},
    };

    for (const Note &note : melody)
    {
        const uint32_t half_period = 500000U / note.frequency_hz;
        const uint32_t start = system_clock::micros();
        uint32_t edge = start;
        while (system_clock::micros() - start < note.duration_ms * 1000U)
        {
            HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_13);
            while (system_clock::micros() - edge < half_period) {}
            edge += half_period;
        }
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
        if (&note != &melody[sizeof(melody) / sizeof(melody[0]) - 1])
            HAL_Delay(20);
    }
}
}
