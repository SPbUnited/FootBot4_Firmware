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

/** Starts boot reporting and records whether the OLED is available. */
void begin_boot_report(bool display_ready);
/** Updates one component and refreshes the current boot page. */
void set_boot_status(BootComponent component, BootState state, uint8_t detail = 0);
/** Completes the report and keeps it visible while pages rotate. */
void finish_boot_report();
void screen();

void init();
void update();
void suspendForShell();
}  // namespace apps::screen
