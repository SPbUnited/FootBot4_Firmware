#include "system_clock.hpp"

#include "devices/device_manager.hpp"

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
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), micros, micros,
                 Robot uptime in microseconds);

}  // namespace drivers::system_clock
