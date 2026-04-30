#pragma once

#include "devices/chassis/chassis.hpp"

namespace devices::odom
{

struct OdometerConfig
{
    float Ts;  // Time step in seconds
};

class Odometer : public OdometerConfig
{
  private:
  public:
    devices::chassis::StateVector state;

    Odometer(OdometerConfig config);

    void init();

    void update(devices::chassis::StateVector vel);
    void getState(devices::chassis::StateVector *vel);

    devices::chassis::StateVector rotate(devices::chassis::StateVector vec, float angle);

    void thetaAntiWindup();
};

}  // namespace devices::odom
