#include "apps/scheduler.hpp"
#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"

int main(void)
{
    HAL_Init();
    drivers::init();
    devices::init();

    logSetLevel(&devices::logger::uartLog, LOG_DEBUG);

    apps::init();
}
