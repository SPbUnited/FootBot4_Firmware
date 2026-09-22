#pragma once

#include "devices/bldc/bldc.hpp"
#include "robot_core/kinematics.hpp"

namespace devices::chassis
{

using StateVector = robot_core::StateVector;

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
    robot_core::Kinematics kinematics;

  public:
    Chassis(ChassisConfig config);

    void init();
    void setVel(StateVector vel);
    void getVel(StateVector *vel);
    void getVel(StateVector *vel, bldc::BldcsVec wheel_vels);
};

}  // namespace devices::chassis
