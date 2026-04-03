#pragma once

#include "devices/bldc/bldc.hpp"
#include "vt_linalg"

namespace devices::chassis
{

struct StateVector
{
    float x;
    float y;
    float theta;
};

struct ChassisConfig
{
    devices::bldc::BldcsDriver *bldcs;
    float wheel_angles[4];  // Углы расположения колёс в радианах
    float wheel_radius;     // Радиус колёс в метрах
    float robot_radius;     // Радиус робота (расстояние от центра до колеса) в метрах
};

class Chassis : public ChassisConfig
{
  private:
    vt::numeric_matrix<4, 3> J;
    vt::numeric_matrix<3, 4> Jinv;

  public:
    Chassis(ChassisConfig config);

    void init();
    void setVel(StateVector vel);
    void getVel(StateVector *vel);
    void getVel(StateVector *vel, bldc::BldcsVec wheel_vels);
};

}  // namespace devices::chassis
