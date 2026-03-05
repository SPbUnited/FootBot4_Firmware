#include "system_clock.hpp"

extern "C" void SysTick_Handler(void)
{
    HAL_IncTick();
}
