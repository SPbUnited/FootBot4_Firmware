#ifdef BARE_METAL
#include "drivers/driver_manager.hpp"
#endif

#include "devices/device_manager.hpp"
#include "kernel/scheduler/scheduler.hpp"

#ifdef WEBOTS_INTEGRATION

#include "iostream"
#include "webots/Robot.hpp"

webots::Robot *wbrobot;

#endif

int main(void)
{
#ifdef BARE_METAL
    HAL_Init();
    drivers::init();
#endif

    devices::init();

#ifdef WEBOTS_INTEGRATION
    int timestep = wbrobot->getBasicTimeStep();
    // kinfo("Webots step: %d ms", timestep);
    std::cout << "Webots step: " << timestep << " ms" << std::endl;

    while (wbrobot->step(timestep) != -1)
    {
    }
#endif

    // kernel::scheduler::init();
}
