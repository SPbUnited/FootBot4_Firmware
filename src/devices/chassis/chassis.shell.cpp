#include "devices/device_manager.hpp"

namespace
{

int chassisPrint(float x, float y, float theta)
{
    kinfo("setting vel: %f %f %f", x, y, theta);
    devices::chassis::StateVector vel = {x, y, theta};
    devices::chassis_drv.setVel(vel);

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), chassisPrint,
                 chassisPrint, write info on chassis);

int chassisGetVelPrint(float w0, float w1, float w2, float w3)
{
    devices::bldc::BldcsVec wheel_vels = {{w0, w1, w2, w3}};
    devices::chassis::StateVector vel;
    devices::chassis_drv.getVel(&vel, wheel_vels);
    kinfo("getting vel: %f %f %f", vel.x, vel.y, vel.theta);

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), chassisGetVelPrint,
                 chassisGetVelPrint, get vel from wheel_vels);

}  // namespace

#define printf(...) shellPrint(ch_shell, ##__VA_ARGS__)
#define getchar() shellGetChar(ch_shell)

#define SIZE 4
uint32_t score = 0;
uint8_t scheme = 0;

Shell *shell2048 = NULL;

char shellGetChar(Shell *shell)
{
    char data;
    if (shell->read && shell->read(&data, 1) == 1)
    {
        return data;
    }
    else
    {
        return -1;
    }
}

int main_chassis_teleop(int argc, char *argv[])
{
    Shell *ch_shell = shellGetCurrent();
    if (!ch_shell)
    {
        return -1;
    }

    float vf = 0.1;
    float w = 1.0;

    while (true)
    {
        char c = getchar();
        if (c == -1)
        {
            printf("\nError! Cannot read keyboard input!");
            break;
        }
        switch (c)
        {
            case 'q':
                chassisPrint(0, 0, 0);
                return 0;
            case 'i':
                chassisPrint(vf, 0, 0);
                break;
            case ',':
                chassisPrint(-vf, 0, 0);
                break;
            case 'j':
                chassisPrint(0, vf, 0);
                break;
            case 'l':
                chassisPrint(0, -vf, 0);
                break;
            case 'u':
                chassisPrint(0, 0, w);
                break;
            case 'o':
                chassisPrint(0, 0, -w);
                break;
            case 'k':
                chassisPrint(0, 0, 0);
                break;
            default:
                break;
        }
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), chassisTeleop,
                 main_chassis_teleop, chassis teleop);