#pragma once

#include <inttypes.h>

#include "drivers/driver_manager.hpp"
#include "shell_cpp.h"

namespace devices::shell
{

extern Shell shell;

signed short shellWrite(char *data, unsigned short len);
signed short shellRead(char *data, unsigned short len);
void my_shellInit();
void my_shellLoop();

}  // namespace devices::shell
