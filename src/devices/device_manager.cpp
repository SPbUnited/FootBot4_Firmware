#include "device_manager.hpp"

#include "drivers/driver_manager.hpp"

namespace devices
{

oled_lib::OledConfig oled_config = {
    .i2cAddr7 = 0x3C,  // 7-битный адрес
    .width = 128,
    .height = 32,
    .i2cFreq = 100000,
    .vccMode = oled_lib::VccMode::InternalChargePump,
    .flip180 = false,
    // .resetCallback = &oledResetCallback, // Если есть RST пин
};

oled::OledDriver oled_drv(oled_config, &drivers::i2c2.handle);

bldc::BldcsConfig bldcs_config = {
    .can = &drivers::can_drv,
};

bldc::BldcsDriver bldcs_drv(bldcs_config);

chassis::ChassisConfig chassis_config = {
    .bldcs = &bldcs_drv,
    .wheel_angles = {1.0472, 2.4433, -2.4433, -1.0472},
    .wheel_radius = 0.034f,
    .robot_radius = 0.0765f,
};

chassis::Chassis chassis_drv(chassis_config);

void init()
{
    shell::my_shellInit();
    logInfo("Shell initialized");
    logger::init();
    logInfo("Logger initialized");
    oled_drv.init();
    logInfo("OLED initialized");

    bldcs_drv.init();
    logInfo("BLDC initialized");

    chassis_drv.init();
    logInfo("Chassis initialized");

    logInfo("Devices initialized");
}

}  // namespace devices