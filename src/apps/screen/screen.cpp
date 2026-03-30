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

    display.rectFill(1, 8, 126, 24, false);  // Очищаем область
    display.setCursor(4, 8);
    display.setTextSize(1);
    display.printf("Uptime:\n  %lu ticks\n  delta: %lu", state.counter, state.delta);
    display.flush();
}

}  // namespace apps::screen
