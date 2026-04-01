#include "scheduler.hpp"

#include "apps/mainloop/mainloop.hpp"
#include "apps/screen/screen.hpp"
#include "devices/device_manager.hpp"

namespace apps
{

static bool is_init = false;

enum Apps
{
    app_mainloop,
    app_shell,
    app_screen,
    app_count,
};

enum State
{
    idle,
    running,
};

static State state[app_count] = {idle};

static void run(Apps app)
{
    if (app >= app_count)
    {
        return;
    }

    if (state[app] == running)
    {
        return;
    }
    state[app] = running;
    switch (app)
    {
        case app_mainloop:
            mainloop::loop();
            break;
        // case app_shell:
        //     devices::shell::my_shellLoop();
        //     break;
        case app_screen:
            screen::screen();
        default:
            break;
    }
    state[app] = idle;
}

void yield()
{
    if (!is_init)
    {
        return;
    }

    if (mainloop::is_loop_pending())
    {
        run(app_mainloop);
    }
    else if (screen::is_screen_pending())
    {
        run(app_screen);
    }
    else
    {
        run(app_shell);
    }
}

void init()
{
    is_init = true;

    while (true)
    {
        yield();
    }
}

}  // namespace apps
