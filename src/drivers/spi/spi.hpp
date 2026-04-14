#pragma once

#include <inttypes.h>

#include "stm32f4xx_hal.h"

namespace drivers::spi
{

struct SPIConfig
{
    SPI_TypeDef *instance;
    //
    void (*clk_enable)();

    // SPI pins
    uint32_t sckPin;
    GPIO_TypeDef *sckPort;
    uint32_t sckAlternate;
    uint32_t misoPin;
    GPIO_TypeDef *misoPort;
    uint32_t misoAlternate;
    uint32_t mosiPin;
    GPIO_TypeDef *mosiPort;
    uint32_t mosiAlternate;

    // Chip Enable pin
    uint32_t cePin;
    GPIO_TypeDef *cePort;

    // Chip Select pin
    uint32_t csPin;
    GPIO_TypeDef *csPort;

    // IRQ pin (optional)
    uint32_t irqPin;
    GPIO_TypeDef *irqPort;
};

class SPIDriver : public SPIConfig
{
  private:
  public:
    SPI_HandleTypeDef handle;

    SPIDriver(SPIConfig config);

    void init();
    int rawWrite(uint8_t reg_addr, uint8_t *value, uint8_t len);
    int rawRead(uint8_t reg_addr, uint8_t *value, uint8_t len);

    void setCe();
    void resetCe();

    void setCS();
    void resetCS();

    void flushTx(void);
    void flushRx(void);
};

};  // namespace drivers::spi
