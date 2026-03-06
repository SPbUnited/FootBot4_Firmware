#pragma once

#include <inttypes.h>

#include "drivers/driver_manager.hpp"

namespace devices::shell
{

signed short shellWrite(char *data, unsigned short len);
signed short shellRead(char *data, unsigned short len);
void my_shellInit();
void my_shellLoop();

}  // namespace devices::shell
