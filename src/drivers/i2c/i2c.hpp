#pragma once

#include <cstdint>

#include "stm32f4xx_hal.h"

namespace drivers::i2c
{

struct I2cConfig
{
    I2C_TypeDef *instance;
    //
    void (*clk_enable)();
    //
    uint32_t sclPin;
    GPIO_TypeDef *sclPort;
    uint32_t sclAlternate;
    uint32_t sdaPin;
    GPIO_TypeDef *sdaPort;
    uint32_t sdaAlternate;
    //
    uint32_t clockSpeed;
    uint32_t dutyCycle;
    uint32_t ownAddress1;
    uint32_t addressingMode;
    uint32_t dualAddressMode;
    uint32_t ownAddress2;
    uint32_t generalCallMode;
    uint32_t noStretchMode;
};

class I2cDriver : public I2cConfig
{
  private:
  public:
    I2C_HandleTypeDef handle;

    I2cDriver(I2cConfig config);

    void init();
};

};  // namespace drivers::i2c
