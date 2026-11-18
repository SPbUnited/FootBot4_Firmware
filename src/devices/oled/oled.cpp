#include "oled.hpp"

// #include "drivers/driver_manager.hpp"
#include "kernel/kernel.hpp"

namespace devices::oled
{

OledDriver::OledDriver(oled_lib::OledConfig cfg, I2C_HandleTypeDef* hi2c)
    : oled_lib::OledConfig(cfg), display(hi2c)
{
}

bool OledDriver::init()
{
    if (display.begin(oled_lib::OledConfig(*this)) != oled_lib::OledResult::Ok)
    {
        kerror("OLED init error: %d %s\n", display.getLastResult(), display.getLastError());
        return false;
    }
    kinfo("OLED init ok\n");
    display.clear();
    display.print("Booting...");
    display.flush();
    return true;
}

}  // namespace devices::oled
