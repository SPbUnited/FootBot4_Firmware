#include "mainloop.hpp"

#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"
#include "inttypes.h"

namespace apps::mainloop
{

bool is_loop_pending(uint32_t time_elapsed, uint32_t period)
{
    return time_elapsed >= period;
}

void loop()
{
    // Sense
    devices::robot_dev.sense();
    devices::nrf24_recv.recv_irq();
    devices::nrf24_recv.fetch();

    // Plan
    devices::robot_dev.plan();

    // Act
    devices::robot_dev.act();
}

}  // namespace apps::mainloop
