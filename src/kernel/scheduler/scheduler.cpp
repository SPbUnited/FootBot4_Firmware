#include "scheduler.hpp"

#include "apps/mainloop/mainloop.hpp"
#include "apps/screen/screen.hpp"
#include "apps/setting_broadcaster/setting_broadcaster.hpp"
#include "devices/device_manager.hpp"

namespace kernel::scheduler
{

static bool is_init = false;

enum Apps
{
    app_mainloop,
    app_screen,
    app_setting_broadcaster,
    app_shell,
    app_monitor_reset,
    app_count,
};

#define TPS(x) (static_cast<uint32_t>(1e6 / x))  // Ticks per second
#define SPT(x) (static_cast<uint32_t>(1e6 * x))  // Seconds per tick

#define ksperiodic [](uint32_t time_elapsed, uint32_t period) { return time_elapsed > period; }
#define kspersistent [](uint32_t time_elapsed, uint32_t period) { return true; }

void monitor_reset();

TaskDescriptor tasks[app_count] = {
    {"mainloop", apps::mainloop::loop, 0, Ts_us, apps::mainloop::is_loop_pending, idle, {0}},
    {"setting_broadcaster",
     apps::setting_broadcaster::broadcast_settings,
     0,
     TPS(1),
     ksperiodic,
     idle,
     {0}},
    {"screen", apps::screen::screen, 0, TPS(10), ksperiodic, idle, {0}},
    {"monitor_reset", monitor_reset, 0, TPS(1), ksperiodic, idle, {0}},
    {"shell", devices::shell::my_shellLoop, 0, 0, kspersistent, idle, {0}},
};

void monitor_reset()
{
    for (size_t app = 0; app < app_count; app++)
    {
        if (tasks[app].monitor.counter > 100)
        {
            tasks[app].monitor.exec_time = {0};
            tasks[app].monitor.period_time = {0};
            tasks[app].monitor.counter = 0;
        }
    }
}

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
    tasks[app].monitor.period_time.update(drivers::system_clock::micros() - tasks[app].last_exec);
    tasks[app].last_exec = drivers::system_clock::micros();
    tasks[app].function();
    tasks[app].monitor.exec_time.update(drivers::system_clock::micros() - tasks[app].last_exec);
    tasks[app].monitor.counter++;
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

void delay_ms(uint32_t delay_ms)
{
    delay_us(delay_ms * 1000);
}

void delay_us(uint32_t delay_us)
{
    uint32_t start = drivers::system_clock::micros();
    while (drivers::system_clock::micros() - start < delay_us)
    {
        yield();
    }
}

TaskDescriptor *get_tasks()
{
    return tasks;
}

size_t get_task_count()
{
    return app_count;
}

}  // namespace kernel::scheduler
