#include "device_manager.hpp"

#include "FlashStorage_STM32.h"
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
    .drive_vel_p = 0.25,
    .drive_vel_i = 40.0,
    // .drive_vel_p = 0.1,
    // .drive_vel_i = 1.0,
    .drive_vel_limit = NAN,
};

bldc::BldcsDriver bldcs_drv(bldcs_config);

chassis::ChassisConfig chassis_config = {
    .bldcs = &bldcs_drv,
    .wheel_angles = {1.0472, 2.4433, -2.4433, -1.0472},
    .wheel_radius = 0.034f * 4 / 5,
    .robot_radius = 0.0765f,
};

chassis::Chassis chassis_drv(chassis_config);

odom::OdometerConfig odom_config = {
    .Ts = Ts_s,
};

odom::Odometer odom_dev(odom_config);

robot::RobotSettings robot_settings = {.dribbler_setting_to_vel = 250.0 / 16,
                                       .kicker_setting_to_voltage = 200.0 / 16,
                                       .angle_kp = 6.0,
                                       .max_linear_vel = 4.8,
                                       .max_linear_accel = NAN,
                                       .max_angular_vel = 6.0,
                                       .max_angular_accel = NAN,
                                       .robot_id = 15,
                                       .signature = 0};

robot::RobotConfig robot_config = {
    .odom_dev = odom_dev, .chassis_drv = chassis_drv, .bldcs_drv = bldcs_drv};

robot::Robot robot_dev(robot_settings, robot_config);

nrfm_decoder::NRFMDecoderConfig nrfm_decoder_config = {
    .robot_dev = robot_dev, .dts_led = drivers::out_pins[drivers::LED_DATA_TRANSFER_STATUS_2]};

nrfm_decoder::NRFMDecoder nrfm_decoder_dev(nrfm_decoder_config);

nrf24::Nrf24RecvConfig nrf24_recv_config = {
    .spi_instance = drivers::spi2,
    .dts_led = drivers::out_pins[drivers::LED_DATA_TRANSFER_STATUS_1],
    .nrfm_decoder = nrfm_decoder_dev,
};

nrf24::Nrf24Recv nrf24_recv(nrf24_recv_config);

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

    odom_dev.init();
    kinfo("Odometer initialized");

    robot_settings.signature = robot::calculate_signature(robot_settings);

    robot::RobotSettings eeprom_robot_settings = {0};

    eeprom::get(0, eeprom_robot_settings);

    if (eeprom_robot_settings.signature != robot_settings.signature)
    {
        kwarning("EEPROM signature mismatch, saving settings to EEPROM");
        eeprom::put(0, robot_settings);
    }
    else
    {
        kinfo("EEPROM signature match");
        kinfo("Updating robot settings from EEPROM");
        kinfo("  id: %d", eeprom_robot_settings.robot_id);
        robot_dev.init(eeprom_robot_settings);
    }

    kinfo("Robot initialized");

    nrf24_recv.init();
    kinfo("NRF24 initialized");

    kinfo("Devices initialized");
}

}  // namespace devices
