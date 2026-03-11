#include "system_clock.hpp"

#ifndef NATIVE

extern "C" void SysTick_Handler(void)
{
    HAL_IncTick();
}

#else

#endif
