#include "device_manager.hpp"

#include "drivers/driver_manager.hpp"

namespace devices
{

const char init_message[] =
    "\n"
    "    ______            __  ____        __  __ __\n"
    "   / ____/___  ____  / /_/ __ )____  / /_/ // /\n"
    "  / /_  / __ \\/ __ \\/ __/ __  / __ \\/ __/ // /_\n"
    " / __/ / /_/ / /_/ / /_/ /_/ / /_/ / /_/__  __/\n"
    "/_/    \\____/\\____/\\__/_____/\\____/\\__/  /_/   \n"
    "\n"
    "Build:              " __DATE__ " " __TIME__
    "\n"
    "Firmware version:   " FIRMWARE_VERSION
    "\n"
    "Copyright:          (c) 2025 SPbUnited\n\n";

oled_lib::OledConfig oled_config = {
    .i2cAddr7 = 0x3C,  // 7-битный адрес
    .width = 128,
    .height = 32,
    .i2cFreq = 400000,
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
    logger::init();
    // char buffer[128];
    // int rem = sizeof(init_message) % sizeof(buffer);
    // snprintf(buffer, sizeof(buffer), "%s", init_message);
    // logPrintln("%s", buffer);
    shellWriteString(&shell::shell, init_message);
    kinfo("Shell initialized");
    kinfo("Logger initialized");
    oled_drv.init();
    kinfo("OLED initialized");

    bldcs_drv.init();
    kinfo("BLDC initialized");

    chassis_drv.init();
    kinfo("Chassis initialized");

    kinfo("Devices initialized");
}

}  // namespace devices
