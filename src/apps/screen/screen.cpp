#include "screen.hpp"

#include "apps/mainloop/mainloop.hpp"
#include "devices/device_manager.hpp"

#define display devices::oled_drv.display

namespace apps::screen
{

static uint32_t screen_timer = 0;
static uint32_t screen_refresh_period = 1000000.0 / 10.0;

bool is_screen_pending()
{
    return drivers::system_clock::micros() - screen_timer > screen_refresh_period;
}

void screen()
{
    screen_timer = drivers::system_clock::micros();

    apps::mainloop::MainloopState state = apps::mainloop::get_state();

    display.rectFill(0, 0, 126, 32, false);  // Очищаем область
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.printf("Version: %s\nUptime: %lu ticks\ndelta(avg|min|max):\n %lu | %lu | %lu",
                   FIRMWARE_VERSION, state.counter, state.deltaavg, state.deltamin,
                   state.deltamax);
    display.flush();
}

}  // namespace apps::screen
