#pragma once

#include <cstdint>
#include "drivers/spi/spi.hpp"

namespace devices::nrf24
{
struct TxStats
{
    bool ready;
    uint32_t completed;
    uint32_t lastCompletedMs;
};

struct Nrf24FeedbackConfig
{
    drivers::spi::SPIDriver &spi_instance;
};

class Nrf24Feedback : private Nrf24FeedbackConfig
{
    bool init();
    TxStats stats{};
    bool sending = false;
    int readReg(uint8_t reg, uint8_t *value) { return spi_instance.rawRead(reg, value, 1); }
    bool writeReg(uint8_t reg, uint8_t value);

  public:
    explicit Nrf24Feedback(Nrf24FeedbackConfig &config) : Nrf24FeedbackConfig(config) {}
    const TxStats &txStats() const { return stats; }
    void start();
    void send();
};
}
