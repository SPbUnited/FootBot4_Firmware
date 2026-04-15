#include "system_clock.hpp"

extern "C" void SysTick_Handler(void)
{
    HAL_IncTick();
}

namespace drivers::system_clock
{

uint32_t micros()
{
    return TIM5->CNT;
}

}  // namespace drivers::system_clock
