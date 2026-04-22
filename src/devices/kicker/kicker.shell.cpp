#include "devices/device_manager.hpp"
// #include "kernel/kernel.hpp"
namespace
{

int voltagePrint(float x)
{
    // kinfo("setting target: %f %f %f", x, y, theta);
    if ((devices::kicker_drv.target + x >= 0) && (devices::kicker_drv.target + x <= 350))
    {
        devices::kicker_drv.target += x;
    }
    return 0;
}

int kickPrint()
{
    devices::kicker_drv.state = devices::kicker::KICK;
    return 0;
}
// SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), chassisPrint,
//                  chassisPrint, write info on chassis);

// int chassisGetVelPrint(float w0, float w1, float w2, float w3)
// {
//     devices::bldc::BldcsVec wheel_vels = {{w0, w1, w2, w3}};
//     devices::chassis::StateVector vel;
//     devices::chassis_drv.getVel(&vel, wheel_vels);
//     kinfo("getting vel: %f %f %f", vel.x, vel.y, vel.theta);

//     return 0;
// }
// SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), chassisGetVelPrint,
//                  chassisGetVelPrint, get vel from wheel_vels);

}  // namespace

#include "devices/shell/shell.main.hpp"

// #define SIZE 4
// uint32_t score = 0;
// uint8_t scheme = 0;

// Shell *shell2048 = NULL;

int main_voltage_teleop(int argc, char *argv[])
{
    getshell();

    // float vf_base = 0.1;
    // float w_base = 1.0;

    // float vf_mult = 1.0;
    // float w_mult = 1.0;

    // devices::chassis::StateVector vel = {0, 0, 0};

    printf("\033[2J");  // Clear screen

    while (true)
    {
        // float vf = vf_base * vf_mult;
        // float w = w_base * w_mult;

        // devices::chassis_drv.getVel(&vel);

        printf("\033[H\nControl voltage with keyboard w up, s down \n Current voltage: %6.3f", devices::kicker_drv.target);

        char c = getchar();
        if (c == -1)
        {
            printf("\nError! Cannot read keyboard input!");
            break;
        }
        switch (c)
        {
            case ';':
                voltagePrint(0);
                return 0;
            case 'w':
                voltagePrint(5);
                break;
            case 's':
                voltagePrint(-5);
                break;
            case 'k':
                kickPrint();
                break;

        }

        kernel::delay_ms(10);
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), voltageTeleop,
                 main_voltage_teleop, voltage teleop);