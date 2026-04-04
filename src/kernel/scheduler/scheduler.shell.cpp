#include "scheduler.hpp"

#include "devices/device_manager.hpp"
#include "devices/shell/shell.main.hpp"

int schedulerTop()
{
    getshell();

    kernel::scheduler::TaskDescriptor *tasks = kernel::scheduler::get_tasks();

    printf("\033[2J");  // Clear screen

    while (true)
    {
        char c = getchar();
        if (c == -1)
        {
            printf("\nError! Cannot read keyboard input!");
            break;
        }
        if (c == 'q')
        {
            break;
        }
        else if (c == 'r')
        {
            for (size_t app = 0; app < kernel::scheduler::get_task_count(); app++)
            {
                tasks[app].monitor.exec_time = {0};
                tasks[app].monitor.period_time = {0};
                tasks[app].monitor.counter = 0;
            }
        }
        // printf("\033[2J\033[H\n");  // Clear screen and move cursor to home position
        printf("\033[H\n");
        printf(
            "app |                name  |   exec_time (min|avg|max)  |  period_time (min|avg|max) "
            "| "
            "counter \n");
        for (size_t app = 0; app < kernel::scheduler::get_task_count(); app++)
        {
            printf(" %2d | %20s | %8lu|%8lu|%8lu | %8lu|%8lu|%8lu | %6lu \n", app, tasks[app].name,
                   tasks[app].monitor.exec_time.min, tasks[app].monitor.exec_time.avg,
                   tasks[app].monitor.exec_time.max, tasks[app].monitor.period_time.min,
                   tasks[app].monitor.period_time.avg, tasks[app].monitor.period_time.max,
                   tasks[app].monitor.counter);
        }

        printf("\nPress 'q' to quit, 'r' to reset monitor.\n");

        kernel::scheduler::delay_ms(250);
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), top, schedulerTop,
                 show scheduler info);
