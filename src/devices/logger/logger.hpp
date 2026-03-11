#pragma once

#include "drivers/driver_manager.hpp"
#include "log.h"

namespace devices::logger
{

extern Log uartLog;

void init();

}  // namespace devices::logger
