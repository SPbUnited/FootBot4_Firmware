#pragma once

#include "inttypes.h"
#include "log.h"
#include "stddef.h"

namespace kernel::scheduler
{

enum TaskState
{
    idle,
    running,
};

struct MinAvgMax
{
    uint32_t min;
    uint32_t avg;
    uint32_t max;

    void update(uint32_t value)
    {
        if (min == 0 || value < min)
        {
            min = value;
        }
        if (value > max)
        {
            max = value;
        }
        if (avg < min)
        {
            avg = min;
        }
        if (avg > max)
        {
            avg = max;
        }
        avg = (avg * 0.9 + value * 0.1);
    }
};

struct TaskDescriptor
{
    char name[20];
    void (*function)();
    uint32_t last_exec;
    uint32_t period;
    bool (*is_pending)(uint32_t time_elapsed, uint32_t period);
    TaskState state;
    struct TaskMonitor
    {
        MinAvgMax exec_time;
        MinAvgMax period_time;
        uint32_t counter;
    } monitor;
    LogLevel log_level;
};

void yield();

void init();

void delay_ms(uint32_t delay_ms);
void delay_us(uint32_t delay_us);

TaskDescriptor *get_tasks();
size_t get_task_count();

}  // namespace kernel::scheduler
