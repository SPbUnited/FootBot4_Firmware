#pragma once

#include "types.hpp"

namespace robot_core
{

struct OdometerConfig
{
    float Ts;  // Time step in seconds
};

class Odometer : public OdometerConfig
{
  public:
    StateVector state{};

    Odometer(OdometerConfig config);

    void init();

    void update(StateVector vel);
    void getState(StateVector *vel);

    StateVector rotate(StateVector vec, float angle);

    void thetaAntiWindup();
};

}  // namespace robot_core
