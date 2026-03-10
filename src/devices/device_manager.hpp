#pragma once

#include "bldc/bldc.hpp"
#include "chassis/chassis.hpp"
#include "logger/logger.hpp"
#include "oled/oled.hpp"
#include "shell/shell.hpp"

namespace devices
{

extern oled::OledDriver oled_drv;
extern bldc::BldcsDriver bldcs_drv;
extern chassis::Chassis chassis_drv;

void init();

}  // namespace devices
