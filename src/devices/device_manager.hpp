#pragma once

#include "bldc/bldc.hpp"
#include "chassis/chassis.hpp"
#include "logger/logger.hpp"
#include "odom/odom.hpp"
#include "oled/oled.hpp"
#include "shell/shell.hpp"

namespace devices
{

extern oled::OledDriver oled_drv;
extern bldc::BldcsDriver bldcs_drv;
extern chassis::Chassis chassis_drv;
extern odom::Odometer odom_dev;

void init();

}  // namespace devices
