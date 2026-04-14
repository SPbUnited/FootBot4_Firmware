#pragma once

#include "inttypes.h"

namespace apps::mainloop
{

bool is_loop_pending(uint32_t time_elapsed, uint32_t period);

void loop();

void nrf24_recv();

}  // namespace apps::mainloop
