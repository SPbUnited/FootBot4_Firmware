/**
 * @file WireI2cAdapter.cpp
 * @brief Реализация адаптера Arduino Wire
 */

#include "../../include/oled/adapters/WireI2cAdapter.hpp"

#if OLED_ENABLED && OLED_USE_ARDUINO

namespace oled_lib
{

bool WireI2cAdapter::write(uint8_t addr7, const uint8_t* data, size_t len)
{
    if (!wire_ || data == nullptr || len == 0)
    {
        return false;
    }

    // Отправляем данные пакетами по WIRE_BUFFER_SIZE байт
    // Адрес не занимает место в буфере данных Wire
    constexpr size_t maxChunk = WIRE_BUFFER_SIZE;

    size_t offset = 0;
    while (offset < len)
    {
        size_t chunkSize = (len - offset > maxChunk) ? maxChunk : (len - offset);

        wire_->beginTransmission(addr7);
        wire_->write(data + offset, chunkSize);

        if (wire_->endTransmission() != 0)
        {
            return false;
        }

        offset += chunkSize;
    }

    return true;
}

bool WireI2cAdapter::probe(uint8_t addr7)
{
    if (!wire_)
        return false;

    wire_->beginTransmission(addr7);
    return wire_->endTransmission() == 0;  // 0 = ACK received
}

}  // namespace oled_lib

#endif  // OLED_ENABLED && OLED_USE_ARDUINO
