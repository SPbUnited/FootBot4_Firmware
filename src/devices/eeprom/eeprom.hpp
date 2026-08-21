#pragma once

#ifdef BARE_METAL

#include "FlashStorage_STM32.hpp"

namespace devices::eeprom
{

template <typename T>
void put(uint8_t id, T value)
{
    EEPROM.put(0, value);
    EEPROM.commit();
}

template <typename T>
void get(uint8_t id, T &value)
{
    EEPROM.get(0, value);
}

}  // namespace devices::eeprom

#endif
