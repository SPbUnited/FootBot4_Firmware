#include "oled.hpp"

// #include "drivers/driver_manager.hpp"
#include "kernel/kernel.hpp"
#include "devices/device_manager.hpp"

namespace devices::oled
{

OledDriver::OledDriver(oled_lib::OledConfig cfg, I2C_HandleTypeDef* hi2c)
    : oled_lib::OledConfig(cfg), display(hi2c), bus_(hi2c)
{
}

void OledDriver::init()
{
    if (display.begin(oled_lib::OledConfig(*this)) != oled_lib::OledResult::Ok)
    {
        while (1)
        {
            kerror("OLED init error: %d %s\n", display.getLastResult(), display.getLastError());
            HAL_Delay(1000);
        }
    }
    kinfo("OLED init ok\n");
    __HAL_RCC_DMA1_CLK_ENABLE();
    static DMA_HandleTypeDef dma{};
    dma.Instance = DMA1_Stream7;
    dma.Init.Channel = DMA_CHANNEL_7;
    dma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dma.Init.PeriphInc = DMA_PINC_DISABLE;
    dma.Init.MemInc = DMA_MINC_ENABLE;
    dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    dma.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    dma.Init.Mode = DMA_NORMAL;
    dma.Init.Priority = DMA_PRIORITY_LOW;
    dma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&dma) != HAL_OK) { kerror("OLED DMA init failed"); return; }
    __HAL_LINKDMA(bus_, hdmatx, dma);
    HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
    HAL_NVIC_SetPriority(I2C2_EV_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
    HAL_NVIC_SetPriority(I2C2_ER_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
    display.clear();
    display.print("Booting...");
    display.flush();
}

bool OledDriver::ready()
{
    if (!display.isDMAComplete() &&
        uint32_t(drivers::system_clock::micros() - started_us_) > 50000) {
        HAL_DMA_Abort(bus_->hdmatx);
        HAL_I2C_DeInit(bus_);
        HAL_I2C_Init(bus_);
        transferComplete(false);
    }
    return display.isDMAComplete();
}

bool OledDriver::submit()
{
    if (!bus_->hdmatx || !ready()) return false;
    // Restore the address window on every frame, including after an interrupted transfer.
    uint8_t window[] = {0x00, 0x21, 0, uint8_t(width - 1), 0x22, 0, uint8_t(height / 8 - 1)};
    if (HAL_I2C_Master_Transmit(bus_, i2cAddr7 << 1, window, sizeof(window), 2) != HAL_OK) {
        ++transfer_errors;
        return false;
    }
    started_us_ = drivers::system_clock::micros();
    if (display.flushDMA() == oled_lib::OledResult::Ok) return true;
    ++transfer_errors;
    return false;
}

void OledDriver::transferComplete(bool success)
{
    uint32_t elapsed = drivers::system_clock::micros() - started_us_;
    if (elapsed > max_transfer_us) max_transfer_us = elapsed;
    if (!success) { ++transfer_errors; }
    display.onDmaComplete();
}

}  // namespace devices::oled

extern "C" void DMA1_Stream7_IRQHandler() {
    HAL_DMA_IRQHandler(drivers::i2c2.handle.hdmatx);
}
extern "C" void I2C2_EV_IRQHandler() { HAL_I2C_EV_IRQHandler(&drivers::i2c2.handle); }
extern "C" void I2C2_ER_IRQHandler() { HAL_I2C_ER_IRQHandler(&drivers::i2c2.handle); }
extern "C" void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *bus) {
    if (bus == &drivers::i2c2.handle) devices::oled_drv.transferComplete(true);
}
extern "C" void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *bus) {
    if (bus == &drivers::i2c2.handle) devices::oled_drv.transferComplete(false);
}
