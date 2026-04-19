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

spi::SPIConfig nrf24_recv_config = {
    .instance = SPI2,
    .clk_enable =
        []()
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_SPI2_CLK_ENABLE();
    },
    .sckPin = GPIO_PIN_10,
    .sckPort = GPIOB,
    .sckAlternate = GPIO_AF5_SPI2,
    .misoPin = GPIO_PIN_2,
    .misoPort = GPIOC,
    .misoAlternate = GPIO_AF5_SPI2,
    .mosiPin = GPIO_PIN_3,
    .mosiPort = GPIOC,
    .mosiAlternate = GPIO_AF5_SPI2,
    .cePin = GPIO_PIN_8,
    .cePort = GPIOD,
    .csPin = GPIO_PIN_12,
    .csPort = GPIOB,
    .irqPin = 0,
    .irqPort = GPIOA,
};

can::CanDriver can_drv(can1_config);
// nrf24::NRF24Driver nrf24_drv(nrf24_config);

gpio::GPIOOutputDriver gpio_out;
gpio::GPIOInputDriver gpio_in;

buzzer::Buzzer buzzer_drv;

bootstrap::Bootstrap bootstrap_drv;
system_clock::SystemClock system_clock_drv;

// nrf24::Nrf24Recv nrf24_recv(nrf24_config);
spi::SPIDriver spi2(nrf24_recv_config);
gpio::GPIOOutputDriver out_pins[OUT_COUNT];
gpio::GPIOInputDriver in_pins[INPUT_COUNT];
gpio::GPIOAnalogInputDriver analog_in_pins[1];

gpio::GPIODescriptor out_pins_desc[] = {
    [LED_STM32] = {GPIOD, GPIO_PIN_15},
    [LED_DRV1] = {GPIOG, GPIO_PIN_2},
    [LED_DRV2] = {GPIOG, GPIO_PIN_3},
    [LED_DRV3] = {GPIOG, GPIO_PIN_4},
    [LED_DRV4] = {GPIOG, GPIO_PIN_5},
    [LED_DRV5] = {GPIOG, GPIO_PIN_6},
    [LED_DATA_TRANSFER_STATUS_1] = {GPIOG, GPIO_PIN_7},
    [LED_DATA_TRANSFER_STATUS_2] = {GPIOG, GPIO_PIN_8},
    [CHARGE] = {GPIOF, GPIO_PIN_5},
    [DISCHARGE] = {GPIOF, GPIO_PIN_6},
    [STRAIGHT] = {GPIOF, GPIO_PIN_11},
    [CHIP] = {GPIOF, GPIO_PIN_12},
};

gpio::GPIODescriptor in_pins_desc[] = {
    [BUTTON_ADDR_UP] = {GPIOE, GPIO_PIN_11},
    [BUTTON_ADDR_DOWN] = {GPIOE, GPIO_PIN_10},
    [BUTTON_SELECT] = {GPIOE, GPIO_PIN_9},
};

gpio::GPIODescriptor analog_pins_desc[] = {
    [KICKER_VOLTAGE] = {GPIOC, GPIO_PIN_1},
};

gpio::ADCDescriptor adc_desc[] = {
    [KICKER_VOLTAGE] = {ADC1, ADC_RESOLUTION_12B, ADC_DATAALIGN_LEFT, DISABLE, ENABLE, ADC_SOFTWARE_START, ADC_EXTERNALTRIGCONVEDGE_NONE, ADC_CHANNEL_11},
};  

void init()
{
    drivers::system_clock_drv.init();
    drivers::bootstrap_drv.init();

    // HAL_Delay(5000);

    drivers::uart4.init();
    drivers::i2c2.init();
    __HAL_RCC_ADC1_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    
    for (int i = 0; i < OUT_COUNT; i++)
    {
        out_pins[i].init(out_pins_desc[i]);
    }
    for (int i = 0; i < INPUT_COUNT; i++)
    {
        in_pins[i].init(in_pins_desc[i]);
    }
    for (int i = 0; i < 1; i++)
    {
        analog_in_pins[i].init(analog_pins_desc[i], adc_desc[i]);
    }

    drivers::can_drv.init();
    drivers::spi2.init();
    
}

}  // namespace drivers
