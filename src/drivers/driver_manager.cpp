#include "driver_manager.hpp"

namespace drivers
{

uart::UartConfig uart4_config = {
    .instance = UART4,
    .clk_enable =
        []()
    {
        __HAL_RCC_UART4_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
    },
    .txPin = GPIO_PIN_0,
    .txPort = GPIOA,
    .txAlternate = GPIO_AF8_UART4,
    .rxPin = GPIO_PIN_1,
    .rxPort = GPIOA,
    .rxAlternate = GPIO_AF8_UART4,
    .baudrate = 115200,
    .wordLength = UART_WORDLENGTH_8B,
    .stopBits = UART_STOPBITS_1,
    .parity = UART_PARITY_NONE,
    .hwFlowControl = UART_HWCONTROL_NONE,
    .mode = UART_MODE_TX_RX,
    .overSampling = UART_OVERSAMPLING_16,
};

uart::UartDriver uart4(uart4_config);

uart::UartConfig uart1_config = {
    .instance = USART1,
    .clk_enable =
        []()
    {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
    },
    .txPin = GPIO_PIN_9,
    .txPort = GPIOA,
    .txAlternate = GPIO_AF7_USART1,
    .rxPin = GPIO_PIN_10,
    .rxPort = GPIOA,
    .rxAlternate = GPIO_AF7_USART1,
    .baudrate = 115200,
    .wordLength = UART_WORDLENGTH_8B,
    .stopBits = UART_STOPBITS_1,
    .parity = UART_PARITY_NONE,
    .hwFlowControl = UART_HWCONTROL_NONE,
    .mode = UART_MODE_TX_RX,
    .overSampling = UART_OVERSAMPLING_16,
};

uart::UartDriver uart1(uart1_config);

i2c::I2cConfig i2c2_config = {
    .instance = I2C2,
    .clk_enable =
        []()
    {
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_I2C2_CLK_ENABLE();
    },
    .sclPin = GPIO_PIN_0,
    .sclPort = GPIOF,
    .sclAlternate = GPIO_AF4_I2C2,
    .sdaPin = GPIO_PIN_1,
    .sdaPort = GPIOF,
    .sdaAlternate = GPIO_AF4_I2C2,
    .clockSpeed = 100000,
    .dutyCycle = I2C_DUTYCYCLE_2,
    .ownAddress1 = 0,
    .addressingMode = I2C_ADDRESSINGMODE_7BIT,
    .dualAddressMode = I2C_DUALADDRESS_DISABLE,
    .ownAddress2 = 0,
    .generalCallMode = I2C_GENERALCALL_DISABLE,
    .noStretchMode = I2C_NOSTRETCH_DISABLE,
};

i2c::I2cDriver i2c2(i2c2_config);

led::LedDriver leds;

oled_lib::OledConfig oled_config = {
    .i2cAddr7 = 0x3C,  // 7-битный адрес
    .width = 128,
    .height = 32,
    .i2cFreq = 100000,
    .vccMode = oled_lib::VccMode::InternalChargePump,
    .flip180 = false,
    // .resetCallback = &oledResetCallback, // Если есть RST пин
};

oled::OledDriver oled_drv(oled_config, &i2c2.handle);

buzzer::Buzzer buzzer_drv;

bootstrap::Bootstrap bootstrap_drv;
system_clock::SystemClock system_clock_drv;

}  // namespace drivers
