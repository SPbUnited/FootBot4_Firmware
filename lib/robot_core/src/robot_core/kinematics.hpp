#pragma once
#include "types.hpp"
namespace robot_core
{
class Kinematics
{
  public:
    explicit Kinematics(Geometry geometry = {});
    WheelVector wheels(StateVector body) const;
    StateVector body(WheelVector wheels) const;

  private:
    double forward_[4][3]{};
    double inverse_[3][4]{};
};
}  // namespace robot_core
