#include "device_manager.hpp"

#include "drivers/driver_manager.hpp"

namespace devices
{

oled_lib::OledConfig oled_config = {
    .i2cAddr7 = 0x3C,  // 7-битный адрес
    .width = 128,
    .height = 32,
    .i2cFreq = 100000,
    .vccMode = oled_lib::VccMode::InternalChargePump,
    .flip180 = false,
    // .resetCallback = &oledResetCallback, // Если есть RST пин
};

oled::OledDriver oled_drv(oled_config, &drivers::i2c2.handle);

}  // namespace devices