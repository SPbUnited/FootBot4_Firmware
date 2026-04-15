#include "i2c.hpp"

#include "kernel/kernel.hpp"

namespace drivers::i2c
{

I2cDriver::I2cDriver(I2cConfig config) : I2cConfig(config) {}

void I2cDriver::init()
{
    clk_enable();

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = sclPin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = sclAlternate;

    HAL_GPIO_Init(sclPort, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = sdaPin;
    GPIO_InitStruct.Alternate = sdaAlternate;

    HAL_GPIO_Init(sdaPort, &GPIO_InitStruct);

    handle.Instance = instance;

    handle.Init.ClockSpeed = clockSpeed;
    handle.Init.DutyCycle = dutyCycle;
    handle.Init.OwnAddress1 = ownAddress1;
    handle.Init.AddressingMode = addressingMode;
    handle.Init.DualAddressMode = dualAddressMode;
    handle.Init.OwnAddress2 = ownAddress2;
    handle.Init.GeneralCallMode = generalCallMode;
    handle.Init.NoStretchMode = noStretchMode;

    HAL_StatusTypeDef status = HAL_I2C_Init(&handle);
    if (status != HAL_OK)
    {
        kerror("I2C init error: %d %d\n", 42, int(status));
    }
}

}  // namespace drivers::i2c
