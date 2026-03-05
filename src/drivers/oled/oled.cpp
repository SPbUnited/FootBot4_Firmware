#include "oled.hpp"

#include "drivers/driver_manager.hpp"

namespace drivers::oled
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
            uart4.printf("OLED init error\n");
            HAL_Delay(100);
        }
    }
    uart4.printf("OLED init ok\n");
    display.clear();
}

}  // namespace drivers::oled
