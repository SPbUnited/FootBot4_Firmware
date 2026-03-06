#pragma once

#include "logger/logger.hpp"
#include "oled/oled.hpp"
#include "shell/shell.hpp"

namespace devices
{

extern oled::OledDriver oled_drv;

void init();

}  // namespace devices