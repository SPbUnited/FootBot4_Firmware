#pragma once

#include "inttypes.h"

namespace apps::mainloop
{

struct MainloopState
{
    uint32_t delta;
    uint32_t counter;
};

bool is_loop_pending();

void loop();

MainloopState get_state();

}  // namespace apps::mainloop
