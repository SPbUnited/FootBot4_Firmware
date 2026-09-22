// STM32 composition and scheduler entry point. Host entry point: sim/webots/main.cpp.
#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"
#include "kernel/scheduler/scheduler.hpp"
int main()
{
    HAL_Init();
    drivers::init();
    devices::init();
    kernel::scheduler::init();
}
