#include "can.hpp"

namespace drivers::can
{

CanDriver::CanDriver() {}

void CanDriver::init() {}
void CanDriver::write(uint32_t id, uint8_t *data, uint8_t len) {}
void CanDriver::read(uint32_t id, uint8_t *data, uint8_t len) {}

}  // namespace drivers::can