#include "scheduler.hpp"

#include "apps/mainloop/mainloop.hpp"
#include "apps/screen/screen.hpp"
#include "apps/setting_broadcaster/setting_broadcaster.hpp"
#include "devices/logger/logger.hpp"
#include "devices/shell/shell.hpp"
#include "drivers/system_clock/system_clock.hpp"

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
    {"mainloop",
     apps::mainloop::loop,
     0,
     Ts_us,
     //  TPS(5),
     apps::mainloop::is_loop_pending,
     idle,
     {0},
     LOG_INFO},
    {"setting_broadcaster",
     apps::setting_broadcaster::broadcast_settings,
     0,
     TPS(1),
     ksperiodic,
     idle,
     {0},
     LOG_NONE},
    {"screen", apps::screen::screen, 0, TPS(10), ksperiodic, idle, {0}, LOG_NONE},
    {"monitor_reset", monitor_reset, 0, TPS(1), ksperiodic, idle, {0}, LOG_DEBUG},
    {"shell", devices::shell::my_shellLoop, 0, 0, kspersistent, idle, {0}, LOG_DEBUG},
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
    LogLevel old_log_level = logGetLevel(&devices::logger::uartLog);
    logSetLevel(&devices::logger::uartLog, tasks[app].log_level);
    tasks[app].function();
    logSetLevel(&devices::logger::uartLog, old_log_level);
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

void sleep_ms(uint32_t sleep_ms)
{
    sleep_us(sleep_ms * 1000);
}

void sleep_us(uint32_t sleep_us)
{
    uint32_t start = drivers::system_clock::micros();
    while (drivers::system_clock::micros() - start < sleep_us)
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
