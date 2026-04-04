#include "scheduler.hpp"

#include "apps/mainloop/mainloop.hpp"
#include "apps/screen/screen.hpp"
#include "devices/device_manager.hpp"

namespace kernel::scheduler
{

static bool is_init = false;

enum Apps
{
    app_mainloop,
    app_screen,
    app_shell,
    app_count,
};

TaskDescriptor tasks[app_count] = {
    {apps::mainloop::loop, 0, Ts_us, apps::mainloop::is_loop_pending, idle},
    {apps::screen::screen, 0, 1000000 / 10, ksperiodic, idle},
    {devices::shell::my_shellLoop, 0, 0, kspersistent, idle},
};

static void run(Apps app)
{
    if (app >= app_count)
    {
        return;
    }

    if (tasks[app].state == running)
    {
        return;
    }
    tasks[app].state = running;
    tasks[app].last_exec = drivers::system_clock::micros();
    tasks[app].function();
    tasks[app].state = idle;
}

void yield()
{
    if (!is_init)
    {
        return;
    }

    for (size_t app = 0; app < app_count; app++)
    {
        uint32_t elapsed_time = drivers::system_clock::micros() - tasks[app].last_exec;
        if (tasks[app].is_pending(elapsed_time, tasks[app].period))
        {
            run(static_cast<Apps>(app));
        }
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

}  // namespace kernel::scheduler
