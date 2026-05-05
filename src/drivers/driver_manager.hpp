#pragma once

#include "bootstrap/bootstrap.hpp"
#include "buzzer/buzzer.hpp"
#include "can/can.hpp"
#include "gpio/gpio.hpp"
#include "i2c/i2c.hpp"
// #include "led/led.hpp"

#include "stm32f4xx_hal.h"
#include "system_clock/system_clock.hpp"
#include "uart/uart.hpp"
#include "spi/spi.hpp"

namespace drivers
{

extern bootstrap::Bootstrap bootstrap_drv;
extern system_clock::SystemClock system_clock_drv;
extern uart::UartDriver uart4;
extern uart::UartDriver uart1;
extern i2c::I2cDriver i2c2;
extern buzzer::Buzzer buzzer_drv;
extern can::CanDriver can_drv;
extern spi::SPIDriver spi2;

enum GPIO_OUT
{
    LED_STM32,
    LED_DRV1,
    LED_DRV2,
    LED_DRV3,
    LED_DRV4,
    LED_DRV5,
    LED_DATA_TRANSFER_STATUS_1,
    LED_DATA_TRANSFER_STATUS_2,
    CHARGE,
    DISCHARGE,
    STRAIGHT,
    CHIP,
    DEEP_LED,
    FRONT_LED,
    OUT_COUNT,
};

enum GPIO_INPUT
{
    BUTTON_ADDR_UP,
    BUTTON_ADDR_DOWN,
    BUTTON_SELECT,
    CHECKER_DEEP,
    CHECKER_FRONT,
    BUTTON_TURN_OFF,
    INPUT_COUNT,
};

enum ANALOG_INPUT
{
    KICKER_VOLTAGE,
    // ANALOG_COUNT,
};

extern gpio::GPIOInputDriver in_pins[INPUT_COUNT];
extern gpio::GPIOOutputDriver out_pins[OUT_COUNT];

// extern gpio::GPIOAnalogInputDriver analog_in_pins[1];

extern gpio::GPIOAnalogInputDriver analog_in_pin;

void init();

}  // namespace drivers
