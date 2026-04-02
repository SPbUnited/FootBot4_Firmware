#pragma once

#include "drivers/driver_manager.hpp"

namespace devices::bldc
{

union BldcsVec
{
    float vec[4];
    struct
    {
        float fl;
        float rl;
        float rr;
        float fr;
    };
};

struct BldcsConfig
{
    drivers::can::CanDriver *can;
};

class BldcsDriver : public BldcsConfig
{
  private:
    BldcsVec current;
    BldcsVec current_vel;
    BldcsVec current_pos;

    BldcsVec target_vel;

    void motorWrite(uint8_t motor_addr, uint8_t register_addr, uint8_t *data, uint8_t len);

  public:
    BldcsDriver(BldcsConfig config) : BldcsConfig(config) {}

    void init() {}

    void setDriveVel(BldcsVec vel);
    void getDriveVel(BldcsVec *vel);

    void setDribblerVel(float vel);
    void getDribblerVel(float *vel);
};

}  // namespace devices::bldc
