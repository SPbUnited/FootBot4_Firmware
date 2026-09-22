#pragma once
#include "types.hpp"
namespace robot_core
{
enum KickerState
{
    PREPARE,
    KICK,
    AFTER_KICK
};
class Kicker
{
  public:
    uint8_t state = PREPARE;
    double target = 0, actual_voltage = 0;
    uint32_t timer = 0;
    KickerOutput output{};
    KickerOutput update(uint8_t setting, uint8_t mode, const SensorFrame &sensors);

  private:
    bool prepared = false;
};
}  // namespace robot_core
