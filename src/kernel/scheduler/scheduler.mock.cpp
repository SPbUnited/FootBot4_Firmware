#ifdef WEBOTS_INTEGRATION
#include "webots/Robot.hpp"
#include "webots.robot.hpp"

namespace kernel::scheduler
{

void yield() {}

void init()
{
    int timestep = wbrobot->getBasicTimeStep();
}

}  // namespace kernel::scheduler

#endif

