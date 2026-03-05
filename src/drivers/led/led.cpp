#include "led.hpp"

namespace drivers::led
{

struct LedDescriptor
{
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
};

LedDescriptor leds_desc[] = {
    [LED::STM32] = {GPIOD, GPIO_PIN_15},
    [LED::DRV1] = {GPIOG, GPIO_PIN_2},
    [LED::DRV2] = {GPIOG, GPIO_PIN_3},
    [LED::DRV3] = {GPIOG, GPIO_PIN_4},
    [LED::DRV4] = {GPIOG, GPIO_PIN_5},
    [LED::DRV5] = {GPIOG, GPIO_PIN_6},
    [LED::DATA_TRANSFER_STATUS_1] = {GPIOG, GPIO_PIN_7},
    [LED::DATA_TRANSFER_STATUS_2] = {GPIOG, GPIO_PIN_8},
};

void LedDriver::init_led(LedDescriptor led)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = led.GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(led.GPIOx, &GPIO_InitStruct);
}

void LedDriver::init()
{
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    for (auto &led : leds_desc)
    {
        init_led(led);
    }
}

void LedDriver::write(LED led, bool state)
{
    HAL_GPIO_WritePin(leds_desc[int(led)].GPIOx, leds_desc[int(led)].GPIO_Pin,
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LedDriver::toggle(LED led)
{
    HAL_GPIO_TogglePin(leds_desc[int(led)].GPIOx, leds_desc[int(led)].GPIO_Pin);
}

void LedDriver::display_number(uint8_t value)
{
    for (size_t i = 0; i < 4; i++)
    {
        write(LED(4 - i), (value & (1 << i)) != 0);
    }
}

}  // namespace drivers::led
