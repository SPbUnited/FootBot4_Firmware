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

  public:
    BldcsDriver(BldcsConfig config) : BldcsConfig(config) {}

    void init() {}

    void setDriveVel(BldcsVec vel);
    void getDriveVel(BldcsVec *vel);
};

}  // namespace devices::bldc
