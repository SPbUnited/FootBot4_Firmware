#include <oled/OledSsd1315.hpp>

#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"
#include "stm32f4xx_hal.h"

#define display devices::oled_drv.display

int main(void)
{
    HAL_Init();
    drivers::system_clock_drv.init();
    drivers::bootstrap_drv.init();

    HAL_Delay(200);

    drivers::uart4.init();
    drivers::i2c2.init();
    drivers::leds.init();
    devices::oled_drv.init();

    HAL_Delay(200);

    drivers::uart4.printf("OLED init ok\n");

    display.clear();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("OLED SSD1315");

    display.setCursor(0, 8);
    display.print("STM32 HAL Demo");

    display.setCursor(0, 16);
    display.setTextSize(1);
    display.print("Привет!");

    // Рисуем рамку
    display.rect(0, 0, 128, 32, true);

    // Отправляем на дисплей
    display.flush();

    devices::shell::my_shellInit();

    // Главный цикл
    uint32_t counter = 0;
    while (1)
    {
        // char c;
        // drivers::uart4.getc(&c);
        // drivers::uart4.putc(c);
        // HAL_Delay(1000);

        // // Обновляем счётчик
        // display.rectFill(1, 24, 126, 7, false);  // Очищаем область
        // display.setCursor(4, 24);
        // display.setTextSize(1);
        // display.printf("Uptime: %lu sec", counter);
        // display.flush();
        // drivers::leds.display_number(counter);
        // drivers::uart4.printf("Uptime: %lu sec\n", counter);

        // counter++;
        devices::shell::my_shellLoop();
    }
}
