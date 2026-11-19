#pragma once
#include <stdint.h>

namespace apps::screen
{

/** Hardware or subsystem reported on the boot screen. */
enum class BootComponent : uint8_t
{
    OLED,
    CAN,
    IMU_UART,
    LOG_UART,
    I2C,
    SPI,
    ANALOG_INPUT,
    RADIO,
    IMU,
    DRIVES,
    KICKER,
    CHASSIS,
    ODOMETER,
    GPIO,
    COUNT,
};

/** Result reported for one boot component. */
enum class BootState : uint8_t
{
    PENDING,
    OK,
    READY,
    WARNING,
    FAILED,
};

/** Records the initialization result for the boot_page menu. */
void set_boot_status(BootComponent component, BootState state, uint8_t detail = 0);
void init();
void update();
void suspendForShell();
}
