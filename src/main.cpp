#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"
#include "kernel/scheduler/scheduler.hpp"

int main(void)
{
    HAL_Init();
    drivers::InitStatus driver_status = drivers::init();
    devices::init(driver_status);

    kernel::scheduler::init();
}
