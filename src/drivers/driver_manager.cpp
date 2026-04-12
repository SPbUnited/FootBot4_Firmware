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
    .clockSpeed = 400000,
    .dutyCycle = I2C_DUTYCYCLE_2,
    .ownAddress1 = 0,
    .addressingMode = I2C_ADDRESSINGMODE_7BIT,
    .dualAddressMode = I2C_DUALADDRESS_DISABLE,
    .ownAddress2 = 0,
    .generalCallMode = I2C_GENERALCALL_DISABLE,
    .noStretchMode = I2C_NOSTRETCH_DISABLE,
};

can::CanConfig can1_config = {
    .instance = CAN1,
    .clk_enable =
        []()
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_CAN1_CLK_ENABLE();
    },
    .txPin = GPIO_PIN_1,
    .txPort = GPIOD,
    .txAlternate = GPIO_AF9_CAN1,
    .rxPin = GPIO_PIN_0,
    .rxPort = GPIOD,
    .rxAlternate = GPIO_AF9_CAN1,
    .prescaler = 12,  // Для 250 кбит/с при 45 МГц CAN clock: 15TQ * (12/45МГц) = 4мкс = 250кбит/с
    .mode = CAN_MODE_NORMAL,
    .sjw = CAN_SJW_1TQ,
    .timeSeg1 = CAN_BS1_6TQ,
    .timeSeg2 = CAN_BS2_8TQ,
    .timeTriggeredMode = DISABLE,
    .autoBusOff = DISABLE,
    .autoWakeUp = DISABLE,
    .autoRetrans = ENABLE,
    .receiveFifoLocked = DISABLE,
    .transmitFifoPriority = DISABLE,
};

i2c::I2cDriver i2c2(i2c2_config);

nrf24::NRF24Config nrf24_config = {
    .instance = SPI2,
    .clk_enable =
        []()
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_SPI2_CLK_ENABLE();
    },
    .sckPin = GPIO_PIN_13,
    .sckPort = GPIOB,
    .sckAlternate = GPIO_AF5_SPI2,
    .misoPin = GPIO_PIN_14,
    .misoPort = GPIOB,
    .misoAlternate = GPIO_AF5_SPI2,
    .mosiPin = GPIO_PIN_15,
    .mosiPort = GPIOB,
    .mosiAlternate = GPIO_AF5_SPI2,
    .cePin = GPIO_PIN_1,
    .cePort = GPIOB,
    .csPin = GPIO_PIN_12,
    .csPort = GPIOB,
    .irqPin = 0,
    .irqPort = GPIOA,
};

can::CanDriver can_drv(can1_config);
nrf24::NRF24Driver nrf24_drv(nrf24_config);

gpio::GPIOOutputDriver gpio_out;
gpio::GPIOInputDriver gpio_in;

buzzer::Buzzer buzzer_drv;

bootstrap::Bootstrap bootstrap_drv;
system_clock::SystemClock system_clock_drv;

void init()
{
    drivers::system_clock_drv.init();
    drivers::bootstrap_drv.init();

    // HAL_Delay(5000);

    drivers::uart4.init();
    drivers::i2c2.init();
    drivers::gpio_out.init();
    drivers::gpio_in.init();
    drivers::can_drv.init();
    drivers::nrf24_drv.init();
}

}  // namespace drivers
