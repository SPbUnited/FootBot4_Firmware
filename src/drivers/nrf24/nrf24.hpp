#pragma once

#include <inttypes.h>

#include "stm32f4xx_hal.h"

namespace drivers::nrf24
{

struct NRF24Config
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

class NRF24Driver : public NRF24Config
{
  private:
  public:
    SPI_HandleTypeDef handle;

    NRF24Driver(NRF24Config config);

    void init();
    void write(uint32_t id, uint8_t *data, uint8_t len);
    void read(uint32_t id, uint8_t *data, uint8_t len);
    void setReg(uint32_t reg, uint32_t value);
};

}  // namespace drivers::nrf24
