#include "robot_core/kicker.hpp"

#include "robot_core/robot.hpp"
namespace robot_core
{
KickerOutput Kicker::update(uint8_t setting, uint8_t mode, const SensorFrame &sensors)
{
    actual_voltage = sensors.capacitor_voltage;

    float MIN_VOLTAGE = 100;
    float MAX_VOLTAGE = 330;

    if (setting == 0)
    {
        target = 0;
    }
    else
    {
        target = (setting - 1) / (15.0 - 1) * (MAX_VOLTAGE - MIN_VOLTAGE) + MIN_VOLTAGE;
    }

    if (target >= 330)
    {
        target = 330;
    }
    else if (target <= 0)
    {
        target = 0;
    }
    if ((state == PREPARE) || !prepared)
    {
        output.straight = false;
        output.chip = false;
        double bound = target * 0.05;
        if (bound < 5)
        {
            bound = 5;
        }
        else if (bound > 10)
        {
            bound = 10;
        }
        if (target - actual_voltage > bound)
        {
            output.discharge = false;
            output.charge = true;
            prepared = false;
        }
        else if (target - actual_voltage < -bound)
        {
            output.discharge = true;
            output.charge = false;
            prepared = false;
        }
        else
        {
            output.discharge = false;
            output.charge = false;
            prepared = true;
            if (mode != DISABLED)
            {
                state = KICK;
            }
        }
    }
    else if (state == KICK)
    {
        output.discharge = false;
        output.charge = false;

        state = PREPARE;

        if (mode == KICK_FRONT)
        {
            timer = sensors.timestamp_us;
            output.straight = true;
            state = AFTER_KICK;
        }
        else if (mode == KICK_UP)
        {
            timer = sensors.timestamp_us;
            output.chip = true;
            state = AFTER_KICK;
        }
        else if (mode == AUTOKICK_FRONT)
        {
            if (sensors.ball_deep && sensors.ball_front)
            {
                timer = sensors.timestamp_us;
                output.straight = true;
                state = AFTER_KICK;
            }
        }
        else if (mode == AUTOKICK_UP)
        {
            if (sensors.ball_deep && sensors.ball_front)
            {
                timer = sensors.timestamp_us;
                output.chip = true;
                state = AFTER_KICK;
            }
        }
        else if (mode == AUTOKICK_MOMENTUM)
        {
            if (sensors.ball_deep || sensors.ball_front)
            {
                timer = sensors.timestamp_us;
                output.straight = true;
                state = AFTER_KICK;
            }
        }
    }
    else if (state == AFTER_KICK)
    {
        if ((sensors.timestamp_us - timer > 10000) && (actual_voltage < 10))
        {
            state = PREPARE;
        }
    }
    return output;
}
}  // namespace robot_core
