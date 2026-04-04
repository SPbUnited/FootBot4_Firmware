#pragma once

#include "inttypes.h"

namespace kernel::scheduler
{

enum TaskState
{
    idle,
    running,
};

struct TaskDescriptor
{
    void (*function)();
    uint32_t last_exec;
    uint32_t period;
    bool (*is_pending)(uint32_t time_elapsed, uint32_t period);
    TaskState state;
};

#define ksperiodic [](uint32_t time_elapsed, uint32_t period) { return time_elapsed > period; }
#define kspersistent [](uint32_t time_elapsed, uint32_t period) { return true; }

void yield();

void init();

}  // namespace kernel::scheduler
