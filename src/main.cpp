#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"

int main(void)
{
    HAL_Init();
    drivers::system_clock_drv.init();
    bool can_initialized = false;
    devices::bldc::BldcsVec zero_velocity{};

    if (drivers::can_drv.init())
    {
        can_initialized = true;
        // Sends monitoring and configured PID settings over CAN.
        devices::bldcs_drv.init();
        devices::bldcs_drv.setDriveVel(zero_velocity);
    }

    uint32_t last_command_us = drivers::system_clock::micros();
    while (true)
    {
        devices::bldcs_drv.pollFeedback();
        drivers::can_drv.updateMonitorWindow();

        const uint32_t now_us = drivers::system_clock::micros();
        if (can_initialized && now_us - last_command_us >= Ts_us)
        {
            devices::bldcs_drv.setDriveVel(zero_velocity);
            last_command_us = now_us;
        }
    }
}
