#include "gpio.hpp"

namespace drivers::gpio
{

void GPIOOutputDriver::init(GPIODescriptor pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin.GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(pin.GPIOx, &GPIO_InitStruct);

    static_cast<GPIODescriptor&>(*this) = pin;
}

void GPIOOutputDriver::write(bool state)
{
    HAL_GPIO_WritePin(this->GPIOx, this->GPIO_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void GPIOOutputDriver::toggle()
{
    HAL_GPIO_TogglePin(this->GPIOx, this->GPIO_Pin);
}

void GPIOInputDriver::init(GPIODescriptor pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin.GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(pin.GPIOx, &GPIO_InitStruct);

    static_cast<GPIODescriptor&>(*this) = pin;
}

bool GPIOInputDriver::read()
{
    return HAL_GPIO_ReadPin(this->GPIOx, this->GPIO_Pin);
}

}  // namespace drivers::gpio
