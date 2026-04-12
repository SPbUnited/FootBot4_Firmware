#include "gpio.hpp"

namespace drivers::gpio
{

GPIODescriptor out_pins[] = {
    [LED_STM32] = {GPIOD, GPIO_PIN_15},
    [LED_DRV1] = {GPIOG, GPIO_PIN_2},
    [LED_DRV2] = {GPIOG, GPIO_PIN_3},
    [LED_DRV3] = {GPIOG, GPIO_PIN_4},
    [LED_DRV4] = {GPIOG, GPIO_PIN_5},
    [LED_DRV5] = {GPIOG, GPIO_PIN_6},
    [LED_DATA_TRANSFER_STATUS_1] = {GPIOG, GPIO_PIN_7},
    [LED_DATA_TRANSFER_STATUS_2] = {GPIOG, GPIO_PIN_8},
};

GPIODescriptor in_pins[] = {
    [BUTTON_ADDR_UP] = {GPIOE, GPIO_PIN_11},
    [BUTTON_ADDR_DOWN] = {GPIOE, GPIO_PIN_10},
    [BUTTON_SELECT] = {GPIOE, GPIO_PIN_9},
};

void init_out_pin(GPIODescriptor pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin.GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(pin.GPIOx, &GPIO_InitStruct);
}

void GPIOOutputDriver::init()
{
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    for (auto &pin : out_pins)
    {
        init_out_pin(pin);
    }
}

void GPIOOutputDriver::write(GPIO_OUT pin, bool state)
{
    HAL_GPIO_WritePin(out_pins[int(pin)].GPIOx, out_pins[int(pin)].GPIO_Pin,
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void GPIOOutputDriver::toggle(GPIO_OUT pin)
{
    HAL_GPIO_TogglePin(out_pins[int(pin)].GPIOx, out_pins[int(pin)].GPIO_Pin);
}

void init_in_pin(GPIODescriptor pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin.GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(pin.GPIOx, &GPIO_InitStruct);
}

void GPIOInputDriver::init()
{
    __HAL_RCC_GPIOE_CLK_ENABLE();
    for (auto &pin : in_pins)
    {
        init_in_pin(pin);
    }
}

bool GPIOInputDriver::read(GPIO_INPUT pin)
{
    return HAL_GPIO_ReadPin(in_pins[int(pin)].GPIOx, in_pins[int(pin)].GPIO_Pin);
}

}