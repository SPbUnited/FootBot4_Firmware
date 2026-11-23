#include "device_manager.hpp"

#include "FlashStorage_STM32.h"
#include "apps/screen/screen.hpp"
#include "drivers/driver_manager.hpp"
#include "apps/screen/screen.hpp"

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

imu::BNO055Config bno055_config = {
    .uart_drv = drivers::uart1,
};

imu::BNO055 bno055_drv(bno055_config);

bldc::BldcsConfig bldcs_config = {
    .can = &drivers::can_drv,
    // .drive_vel_p = 0.25,
    // .drive_vel_i = 40.0,
    // // .drive_vel_p = 0.1,
    // // .drive_vel_i = 1.0,
    // .drive_vel_limit = 8.0,
    // .drive_vel_limit_idle = 1.0,
    .drive_vel_p = NAN,
    .drive_vel_i = NAN,
    .drive_vel_limit = NAN,
    .drive_vel_limit_idle = NAN,
    .feedback_leds = {drivers::GPIO_OUT::LED_DRV1, drivers::GPIO_OUT::LED_DRV2, drivers::GPIO_OUT::LED_DRV3, drivers::GPIO_OUT::LED_DRV4},
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

robot::RobotSettings robot_settings = {
    .dribbler_setting_to_vel = 600.0 / 16,
    .kicker_setting_to_voltage = 200.0 / 16,
    .angle_k = 0.3,
    .angle_b = 5.5,  // щас тут динамический коэф
    .max_linear_vel = 3.2,
    .max_linear_accel = 100.0,  // убрали ограничение по ускорению локальному
    .max_angular_vel = 10.0,
    .max_angular_accel = 100.0,
    .robot_id = 15,
    .signature = 0};

kicker::KickerConfig kicker_config = {
    .adc_pin = &drivers::analog_in_pin,
    .charge_pin = &drivers::out_pins[drivers::CHARGE],
    .discharge_pin = &drivers::out_pins[drivers::DISCHARGE],
    .straight_pin = &drivers::out_pins[drivers::STRAIGHT],
    .chip_pin = &drivers::out_pins[drivers::CHIP],
    .ball_checker_deep = &drivers::in_pins[drivers::CHECKER_DEEP],
    .ball_checker_front = &drivers::in_pins[drivers::CHECKER_FRONT]};

kicker::Kicker kicker_drv(kicker_config);

robot::RobotConfig robot_config = {.odom_dev = odom_dev,
                                   .chassis_drv = chassis_drv,
                                   .bldcs_drv = bldcs_drv,
                                   .bno055_drv = bno055_drv,
                                   .kicker_drv = kicker_drv};

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

nrf24::Nrf24FeedbackConfig nrf_feedback_config = {
    .spi_instance = drivers::spi4,
};
nrf24::Nrf24Feedback nrf_feedback(nrf_feedback_config);

void init(drivers::InitStatus driver_status)
{
    shell::my_shellInit();
    if (driver_status.uart4)
    {
        logger::init();
    }
    // char buffer[128];
    // int rem = sizeof(init_message) % sizeof(buffer);
    // snprintf(buffer, sizeof(buffer), "%s", init_message);
    // logPrintln("%s", buffer);
    if (driver_status.uart4)
    {
        shellWriteString(&shell::shell, init_message);
    }
    kinfo("Shell initialized");
    kinfo("Logger initialized");
    bool oled_initialized = driver_status.i2c && oled_drv.init();
    apps::screen::set_boot_status(apps::screen::BootComponent::OLED,
        oled_initialized ? apps::screen::BootState::OK : apps::screen::BootState::FAILED);
    apps::screen::set_boot_status(
        apps::screen::BootComponent::CAN,
        driver_status.can ? apps::screen::BootState::OK : apps::screen::BootState::FAILED);
    apps::screen::set_boot_status(
        apps::screen::BootComponent::IMU_UART,
        driver_status.uart1 ? apps::screen::BootState::OK : apps::screen::BootState::FAILED);
    apps::screen::set_boot_status(
        apps::screen::BootComponent::LOG_UART,
        driver_status.uart4 ? apps::screen::BootState::OK : apps::screen::BootState::FAILED);
    apps::screen::set_boot_status(
        apps::screen::BootComponent::I2C,
        driver_status.i2c ? apps::screen::BootState::OK : apps::screen::BootState::FAILED);
    apps::screen::set_boot_status(
        apps::screen::BootComponent::SPI,
        driver_status.spi ? apps::screen::BootState::OK : apps::screen::BootState::FAILED);
    apps::screen::set_boot_status(
        apps::screen::BootComponent::ANALOG_INPUT,
        driver_status.adc ? apps::screen::BootState::OK : apps::screen::BootState::FAILED);
    apps::screen::set_boot_status(apps::screen::BootComponent::GPIO,
                                  apps::screen::BootState::READY);
    if (oled_initialized)
    {
        kinfo("OLED initialized");
    }

    bool radio_initialized = driver_status.spi && !nrf24_recv.init();
    apps::screen::set_boot_status(
        apps::screen::BootComponent::RADIO,
        radio_initialized ? apps::screen::BootState::OK : apps::screen::BootState::FAILED);
    if (!radio_initialized)
    {
        kerror("NRF24 initialization failed");
    }
    else
    {
        kinfo("NRF24 initialized");
    }

    bool bno055_initialized = false;
    uint8_t imu_failure_detail = driver_status.uart1 ? 2 : 1;
    for (size_t i = 0; driver_status.uart1 && i < 5; i++)
    {
        bool bno055_init = bno055_drv.check();
        if (!bno055_init)
        {
            kerror("BNO055 offline");
            if (!bno055_drv.reset())
            {
                HAL_Delay(100);
            }
        }
        else
        {
            kinfo("BNO055 online");
            bool config_mode_ok = bno055_drv.setmode(OPERATION_MODE_CONFIG);
            bool angle_units_ok = config_mode_ok && bno055_drv.set_angle_units(RADIANS);
            bool rate_units_ok = angle_units_ok && bno055_drv.set_anglerate_units(RAD_PER_SEC);
            bool ndof_mode_ok = rate_units_ok && bno055_drv.setmode(OPERATION_MODE_NDOF);
            if (ndof_mode_ok)
            {
                kinfo("BNO055 initialized");
                bno055_initialized = true;
                break;
            }

            imu_failure_detail = 3;
            kerror("BNO055 configuration failed");
            if (!bno055_drv.reset())
            {
                HAL_Delay(100);
            }
        }
    }
    if (!bno055_initialized)
    {
        kerror("BNO055 initialization failed after retries");
    }
    apps::screen::set_boot_status(
        apps::screen::BootComponent::IMU,
        bno055_initialized ? apps::screen::BootState::OK : apps::screen::BootState::FAILED,
        bno055_initialized ? 0 : imu_failure_detail);

    uint8_t drive_feedback_mask = bldcs_drv.init();
    uint8_t drive_count = 0;
    for (uint8_t i = 0; i < 4; ++i)
    {
        drive_count += (drive_feedback_mask >> i) & 1U;
    }
    apps::screen::set_boot_status(apps::screen::BootComponent::DRIVES,
                                  drive_count == 4
                                      ? apps::screen::BootState::OK
                                      : (drive_count == 0 ? apps::screen::BootState::FAILED
                                                          : apps::screen::BootState::WARNING),
                                  drive_feedback_mask);
    if (drive_count == 4)
    {
        kinfo("BLDC initialized");
    }

    chassis_drv.init();
    apps::screen::set_boot_status(apps::screen::BootComponent::CHASSIS,
                                  apps::screen::BootState::READY);
    kinfo("Chassis initialized");

    odom_dev.init();
    apps::screen::set_boot_status(apps::screen::BootComponent::ODOMETER,
                                  apps::screen::BootState::READY);
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

    kicker_drv.init();
    apps::screen::set_boot_status(apps::screen::BootComponent::KICKER,
                                  apps::screen::BootState::READY);
    kinfo("Kicker initialized");
    // kinfo("AUAUSDUASDUDevices initialized");
    radio_initialized = driver_status.spi && !nrf24_recv.init();
    nrf24_recv.recv_irq();
    if (driver_status.spi4)
        nrf_feedback.init();
    kinfo("Devices initialized");
    apps::screen::init();
}

}  // namespace devices
