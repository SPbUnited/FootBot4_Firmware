#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "adapter.hpp"
namespace
{
std::vector<std::string> split(const std::string &value)
{
    std::vector<std::string> result;
    std::istringstream stream(value);
    std::string item;
    while (std::getline(stream, item, ','))
        result.push_back(item);
    return result;
}
float number(const std::string &value)
{
    size_t count = 0;
    const float result = std::stof(value, &count);
    if (count != value.size() || !std::isfinite(result))
        throw std::runtime_error("Invalid number: " + value);
    return result;
}
}  // namespace
int main(int argc, char **argv)
{
    try
    {
        simulation::Config config;
        auto settings = robot_core::default_settings();
        robot_core::RobotCommand initial;
        bool initial_command = false;
        for (int i = 1; i < argc; ++i)
        {
            const std::string argument(argv[i]);
            const auto equals = argument.find('=');
            const auto key = argument.substr(0, equals);
            const auto value = equals == std::string::npos ? "" : argument.substr(equals + 1);
            if (key == "--help")
            {
                std::cout
                    << "--wheels=0,1,2,3 (fl,rl,rr,fr) --signs=1,1,1,1 --imu=name --yaw-sign=1\n"
                       "--receiver=name --dribbler=name --robot-id=15 --velocity=x,y,w "
                       "--heading=delta_radians\n";
                return 0;
            }
            else if (key == "--wheels" || key == "--signs")
            {
                const auto values = split(value);
                if (values.size() != 4)
                    throw std::runtime_error(key + " requires four entries");
                for (size_t j = 0; j < 4; ++j)
                {
                    if (key == "--wheels")
                        config.wheels[j] = values[j];
                    else
                    {
                        config.signs[j] = number(values[j]);
                        if (std::abs(config.signs[j]) != 1)
                            throw std::runtime_error("Wheel signs must be +/-1");
                    }
                }
            }
            else if (key == "--imu")
                config.imu = value;
            else if (key == "--receiver")
                config.receiver = value;
            else if (key == "--dribbler")
                config.dribbler = value;
            else if (key == "--yaw-sign")
            {
                config.yaw_sign = number(value);
                if (std::abs(config.yaw_sign) != 1)
                    throw std::runtime_error("Yaw sign must be +/-1");
            }
            else if (key == "--robot-id")
            {
                const float id = number(value);
                if (id < 0 || id > 15 || std::floor(id) != id)
                    throw std::runtime_error("Robot ID must be 0..15");
                settings.robot_id = static_cast<uint8_t>(id);
            }
            else if (key == "--velocity")
            {
                const auto values = split(value);
                if (values.size() != 3)
                    throw std::runtime_error("Velocity requires x,y,w");
                initial.local_x = number(values[0]);
                initial.local_y = number(values[1]);
                initial.angular = number(values[2]);
                initial_command = true;
            }
            else if (key == "--heading")
            {
                initial.angular = number(value);
                initial.relative_heading = true;
                initial_command = true;
            }
            else
                throw std::runtime_error("Unknown option: " + argument);
        }
        webots::Robot simulation;
        const double basic = simulation.getBasicTimeStep();
        const double ratio = 10.0 / basic;
        if (basic <= 0 || basic > 10 || std::abs(ratio - std::round(ratio)) > 1e-6)
            throw std::runtime_error(
                "Webots basicTimeStep must divide the 10 ms control period (e.g. 1, 2, 5, 10 ms)");
        simulation::Adapter adapter(simulation, config);
        robot_core::Robot controller(settings);
        while (simulation.step(10) != -1)
        {
            const auto sensors = adapter.sense();
            controller.sense(sensors);
            if (initial_command)
            {
                initial.received_us = sensors.timestamp_us;
                robot_core::apply_command(controller, initial);
                initial_command = false;
            }
            adapter.commands(controller);
            controller.plan();
            adapter.apply(controller.act());
        }
    }
    catch (const std::exception &error)
    {
        std::cerr << "Controller error: " << error.what() << '\n';
        return 1;
    }
}
