#pragma once

#include "bldc/bldc.hpp"
#include "chassis/chassis.hpp"
#include "eeprom/eeprom.hpp"
#include "imu/imu.hpp"
#include "logger/logger.hpp"
#include "nrf24/nrf24.hpp"
#include "nrfm_decoder/nrfm_decoder.hpp"
#include "odom/odom.hpp"
#include "oled/oled.hpp"
#include "robot/robot.hpp"
#include "shell/shell.hpp"

namespace devices
{

extern oled::OledDriver oled_drv;
extern bldc::BldcsDriver bldcs_drv;
extern chassis::Chassis chassis_drv;
extern odom::Odometer odom_dev;
extern robot::Robot robot_dev;
extern nrf24::Nrf24Recv nrf24_recv;
extern imu::BNO055 bno055_drv;
extern nrfm_decoder::NRFMDecoder nrfm_decoder_dev;

void init();

}  // namespace devices
