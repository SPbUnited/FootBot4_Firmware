#include "oled.hpp"

#include "drivers/driver_manager.hpp"

namespace devices::oled
{

OledDriver::OledDriver(oled_lib::OledConfig cfg, I2C_HandleTypeDef* hi2c)
    : oled_lib::OledConfig(cfg), display(hi2c)
{
}

void OledDriver::init()
{
    if (display.begin(oled_lib::OledConfig(*this)) != oled_lib::OledResult::Ok)
    {
        while (1)
        {
            // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            drivers::uart4.printf("OLED init error: %d %s\n", display.getLastResult(),
                                  display.getLastError());
            HAL_Delay(100);
        }
    }
    drivers::uart4.printf("OLED init ok\n");
    display.clear();
    display.print("Booting...");
    display.flush();
}

}  // namespace devices::oled
