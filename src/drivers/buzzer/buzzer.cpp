#include "buzzer.hpp"

#include "drivers/system_clock/system_clock.hpp"

namespace drivers::buzzer
{
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
