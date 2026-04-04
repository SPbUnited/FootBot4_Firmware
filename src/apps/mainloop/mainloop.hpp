#pragma once

#include "inttypes.h"

namespace apps::mainloop
{

struct MainloopState
{
    uint32_t deltamin;
    uint32_t deltaavg;
    uint32_t deltamax;
    uint32_t counter;
};

bool is_loop_pending(uint32_t time_elapsed, uint32_t period);

void loop();

MainloopState get_state();

}  // namespace apps::mainloop
