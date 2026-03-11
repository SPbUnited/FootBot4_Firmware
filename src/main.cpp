#include "devices/device_manager.hpp"
#include "drivers/driver_manager.hpp"

#define display devices::oled_drv.display

int main(void)
{
    HAL_Init();
    drivers::init();
    devices::init();

    // Главный цикл
    uint32_t counter = 0;
    UNUSED(counter);

    logSetLevel(&devices::logger::uartLog, LOG_DEBUG);

    while (1)
    {
        static uint32_t last = 0;
        while (drivers::system_clock::micros() - last < Ts_us - 2000)
            devices::shell::my_shellLoop();
        while (drivers::system_clock::micros() - last < Ts_us)
            ;
        uint32_t delta = drivers::system_clock::micros() - last;
        last += Ts_us;

        logVerbose("Uptime: %lu ticks, delta: %lu", counter, delta);

        // // Обновляем счётчик
        display.rectFill(1, 8, 126, 24, false);  // Очищаем область
        display.setCursor(4, 8);
        display.setTextSize(1);
        display.printf("Uptime:\n  %lu ticks\n  delta: %lu", counter, delta);
        display.flush();
        drivers::leds.display_number(counter);

        counter++;
    }
}
