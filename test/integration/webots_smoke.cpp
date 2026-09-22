#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <webots/Supervisor.hpp>

#include "adapter.hpp"
// A fixed base with four independent rotors tests the real SDK device boundary.
// It is deliberately not a physical model of an omnidirectional robot.
int main()
{
    std::ofstream progress("smoke-progress.log", std::ios::trunc);
    progress << "starting" << std::endl;
    webots::Supervisor world;
    try
    {
        simulation::Adapter adapter(world, {});
        robot_core::Robot controller(robot_core::default_settings());
        const robot_core::StateVector scenarios[] = {
            {0.4f, 0, 0}, {0, 0.4f, 0}, {0, 0, 1}, {0.2f, -0.2f, 0.5f}, {0, 0, 0}};
        for (const auto scenario : scenarios)
        {
            progress << "scenario " << scenario.x << "," << scenario.y << "," << scenario.theta
                     << std::endl;
            controller.reset();
            controller.set_target_local_linear_vel(scenario.x, scenario.y, true);
            controller.set_target_angular_vel(scenario.theta);
            robot_core::SensorFrame feedback;
            robot_core::ActuatorFrame output;
            for (unsigned step = 0; step < 150; ++step)
            {
                if (world.step(10) == -1)
                    throw std::runtime_error("World ended early");
                feedback = adapter.sense();
                controller.sense(feedback);
                controller.plan();
                output = controller.act();
                adapter.apply(output);
            }
            if (!feedback.wheels_measured)
                throw std::runtime_error("No encoder feedback");
            for (unsigned wheel = 0; wheel < 4; ++wheel)
                if (std::abs(feedback.wheels.vec[wheel] - output.wheels.vec[wheel]) > 0.15f)
                    throw std::runtime_error(
                        "Encoder did not follow shared controller wheel output");
        }
        progress << "passed" << std::endl;
        std::cout << "Webots adapter smoke checks passed" << std::endl;
        // Let Webots drain controller output before requesting simulator shutdown.
        world.step(10);
        world.simulationQuit(0);
    }
    catch (const std::exception &error)
    {
        progress << "failure: " << error.what() << std::endl;
        std::cerr << "Webots smoke failure: " << error.what() << '\n';
        world.simulationQuit(1);
        return 1;
    }
}
