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

#include "devices/shell/shell.main.hpp"

#define SIZE 4
uint32_t score = 0;
uint8_t scheme = 0;

Shell *shell2048 = NULL;

int main_chassis_teleop(int argc, char *argv[])
{
    getshell();

    logSetLevel(&devices::logger::uartLog, LOG_WRANING);

    float vf_base = 0.1;
    float w_base = 1.0;

    float vf_mult = 1.0;
    float w_mult = 1.0;

    devices::chassis::StateVector vel = {0, 0, 0};

    printf("\033[2J");  // Clear screen

    while (true)
    {
        float vf = vf_base * vf_mult;
        float w = w_base * w_mult;

        devices::chassis_drv.getVel(&vel);

        printf(
            "\033[H\nControl robot with keyboard [vel: %5.2fm/s, %5.2frad/s]:\n"
            "  qweasdzxc for car movement, \n"
            "  uiojklm,. for translation, \n",
            vf, w);
        printf(
            "  -= change linear velocity, \n"
            "  [] change angular velocity, \n"
            "  ; to quit.,\n");
        printf("current vel: x: %5.2fm/s, y: %5.2fm/s, theta: %5.2frad/s", vel.x, vel.y,
               vel.theta);

        char c = getchar();
        if (c == -1)
        {
            printf("\nError! Cannot read keyboard input!");
            break;
        }
        switch (c)
        {
            case ';':
                chassisPrint(0, 0, 0);
                return 0;
            case 'q':
                chassisPrint(vf, 0, w);
                break;
            case 'w':
                chassisPrint(vf, 0, 0);
                break;
            case 'e':
                chassisPrint(vf, 0, -w);
                break;
            case 'a':
                chassisPrint(0, 0, w);
                break;
            case 's':
                chassisPrint(0, 0, 0);
                break;
            case 'd':
                chassisPrint(0, 0, -w);
                break;
            case 'z':
                chassisPrint(-vf, 0, -w);
                break;
            case 'x':
                chassisPrint(-vf, 0, 0);
                break;
            case 'c':
                chassisPrint(-vf, 0, w);
                break;
            case 'u':
                chassisPrint(vf, vf, 0);
                break;
            case 'i':
                chassisPrint(vf, 0, 0);
                break;
            case 'o':
                chassisPrint(vf, -vf, 0);
                break;
            case 'j':
                chassisPrint(0, vf, 0);
                break;
            case 'k':
                chassisPrint(0, 0, 0);
                break;
            case 'l':
                chassisPrint(0, -vf, 0);
                break;
            case 'm':
                chassisPrint(-vf, vf, 0);
                break;
            case ',':
                chassisPrint(-vf, 0, 0);
                break;
            case '.':
                chassisPrint(-vf, -vf, 0);
                break;

            case '-':
                vf_mult /= 1.1;
                break;
            case '=':
                vf_mult *= 1.1;
                break;
            case '[':
                w_mult /= 1.1;
                break;
            case ']':
                w_mult *= 1.1;
                break;
            default:
                break;
        }

        kernel::scheduler::delay_ms(50);
    }

    logSetLevel(&devices::logger::uartLog, LOG_DEBUG);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), chassisTeleop,
                 main_chassis_teleop, chassis teleop);