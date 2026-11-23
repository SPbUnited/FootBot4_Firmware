#include "nrf_feedback.hpp"

#include <cstdio>
#include "devices/device_manager.hpp"
#include "drivers/system_clock/system_clock.hpp"
#include "stm32f4xx_ll_bus.h"

namespace apps::nrf_feedback
{
namespace
{
drivers::spi::SPIDriver spi({
    .instance = SPI4,
    .clk_enable = [] { __HAL_RCC_SPI4_CLK_ENABLE(); },
    .sckPin = GPIO_PIN_2, .sckPort = GPIOE,
    .sckAlternate = GPIO_AF5_SPI4,
    .misoPin = GPIO_PIN_5, .misoPort = GPIOE,
    .misoAlternate = GPIO_AF5_SPI4,
    .mosiPin = GPIO_PIN_6, .mosiPort = GPIOE,
    .mosiAlternate = GPIO_AF5_SPI4,
    .cePin = GPIO_PIN_8, .cePort = GPIOD,
    .csPin = GPIO_PIN_12, .csPort = GPIOB,
    .irqPin = GPIO_PIN_15, .irqPort = GPIOB,  // Завершение TX проверяем по STATUS.
});

constexpr uint8_t statusReg = 0x07, txDone = 0x20, maxRetries = 0x10;
constexpr uint8_t flushTx = 0xE1, writePayload = 0xA0;
bool ready = false;
bool sending = false;

bool writeReg(uint8_t reg, uint8_t value)
{
    return spi.rawWrite(reg, &value, 1) == 0;
}
}

void init()
{
    if (!spi.sckPin || !spi.sckPort || !spi.misoPin || !spi.misoPort ||
        !spi.mosiPin || !spi.mosiPort || !spi.csPin || !spi.csPort || !spi.cePin || !spi.cePort)
        return;

    GPIO_TypeDef *ports[] = {spi.sckPort, spi.misoPort, spi.mosiPort, spi.csPort, spi.cePort};
    for (auto port : ports)
        LL_AHB1_GRP1_EnableClock(RCC_AHB1ENR_GPIOAEN << GPIO_GET_INDEX(port));
    if (!spi.init())
        return;
    spi.resetCe();
    spi.setCS();

    uint8_t address[] = {0xAB, 0xAD, 0xAF};
    uint8_t config = 0;
    ready = writeReg(0x00, 0x0E) &&  // Powered PTX, two-byte CRC
            writeReg(0x01, 0) &&     // No auto-ack
            writeReg(0x03, 1) &&     // Three-byte address
            writeReg(0x05, 76) &&
            writeReg(0x06, 0x0E) &&  // 2 Mbps, 0 dBm
            spi.rawWrite(0x10, address, sizeof(address)) == 0 &&
            spi.rawWrite(flushTx, nullptr, 0) == 0 && writeReg(statusReg, 0x70) &&
            spi.rawRead(0x00, &config, 1) == 0 && config == 0x0E;
    if (ready)
        HAL_Delay(2);  // Power-up to standby before the first CE pulse.
}

void send()
{
    if (!ready)
        return;
    if (sending)
    {
        uint8_t status = 0;
        if (spi.rawRead(statusReg, &status, 1) || !(status & (txDone | maxRetries)))
            return;
        if ((status & maxRetries) && spi.rawWrite(flushTx, nullptr, 0))
            return;
        if (!writeReg(statusReg, txDone | maxRetries))
            return;
        sending = false;
    }

    const unsigned ball_sensors = devices::kicker_drv.read_front() +
                                  (devices::kicker_drv.read_deep() << 1);
    const float volts = can_battery_voltage;
    if (!(volts >= 0 && volts <= 1000))
        return;

    char packet[33];
    const int length = std::snprintf(packet, sizeof(packet), "id=%u,k=%u,mv=%u",
                                     static_cast<unsigned>(devices::robot_dev.robot_id),
                                     ball_sensors, static_cast<unsigned>(volts * 1000 + 0.5f));
    if (length <= 0 || length >= static_cast<int>(sizeof(packet)) ||
        spi.rawWrite(writePayload, reinterpret_cast<uint8_t *>(packet), length))
        return;

    sending = true;
    spi.setCe();
    const uint32_t start = drivers::system_clock::micros();
    while (drivers::system_clock::micros() - start < 12) {}
    spi.resetCe();
}
}
