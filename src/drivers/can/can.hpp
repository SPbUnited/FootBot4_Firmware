#pragma once

#include <inttypes.h>

namespace drivers::can
{

class CanDriver
{
  private:
  public:
    CanDriver();

    void init();
    void write(uint32_t id, uint8_t *data, uint8_t len);
    void read(uint32_t id, uint8_t *data, uint8_t len);
};

}  // namespace drivers::can
