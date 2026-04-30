#include "devices/device_manager.hpp"
#include "devices/shell/shell.main.hpp"
#include "kernel/kernel.hpp"

// Shell *shell2048 = NULL;

int main_robot_teleop(int argc, char *argv[])
{
    getshell();

    float vf_base = 0.1;
    float w_base = 1.0;

    float vf_mult = 1.0;
    float w_mult = 1.0;

    devices::chassis::StateVector vel = {0, 0, 0};

    printf("\033[2J");  // Clear screen

    bool is_velocity_local = true;

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
                // chassisPrint(0, 0, 0);
                devices::robot_dev.set_target_local_linear_vel(0, 0, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(0);
                return 0;
            case 'q':
                // chassisPrint(vf, 0, w);
                devices::robot_dev.set_target_local_linear_vel(vf, 0, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(w);
                break;
            case 'w':
                // chassisPrint(vf, 0, 0);
                devices::robot_dev.set_target_local_linear_vel(vf, 0, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(0);
                break;
            case 'e':
                // chassisPrint(vf, 0, -w);
                devices::robot_dev.set_target_local_linear_vel(vf, 0, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(-w);
                break;
            case 'a':
                // chassisPrint(0, 0, w);
                devices::robot_dev.set_target_local_linear_vel(0, 0, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(w);
                break;
            case 's':
                // chassisPrint(0, 0, 0);
                devices::robot_dev.set_target_local_linear_vel(0, 0, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(0);
                break;
            case 'd':
                // chassisPrint(0, 0, -w);
                devices::robot_dev.set_target_local_linear_vel(0, 0, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(-w);
                break;
            case 'z':
                // chassisPrint(-vf, 0, -w);
                devices::robot_dev.set_target_local_linear_vel(-vf, 0, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(-w);
                break;
            case 'x':
                // chassisPrint(-vf, 0, 0);
                devices::robot_dev.set_target_local_linear_vel(-vf, 0, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(0);
                break;
            case 'c':
                // chassisPrint(-vf, 0, w);
                devices::robot_dev.set_target_local_linear_vel(-vf, 0, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(w);
                break;
            case 'u':
                // chassisPrint(vf, vf, 0);
                devices::robot_dev.set_target_local_linear_vel(vf, vf, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(0);
                break;
            case 'i':
                // chassisPrint(vf, 0, 0);
                devices::robot_dev.set_target_local_linear_vel(vf, 0, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(0);
                break;
            case 'o':
                // chassisPrint(vf, -vf, 0);
                devices::robot_dev.set_target_local_linear_vel(vf, -vf, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(0);
                break;
            case 'j':
                // chassisPrint(0, vf, 0);
                devices::robot_dev.set_target_local_linear_vel(0, vf, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(0);
                break;
            case 'k':
                // chassisPrint(0, 0, 0);
                devices::robot_dev.set_target_local_linear_vel(0, 0, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(0);
                break;
            case 'l':
                // chassisPrint(0, -vf, 0);
                devices::robot_dev.set_target_local_linear_vel(0, -vf, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(0);
                break;
            case 'm':
                // chassisPrint(-vf, vf, 0);
                devices::robot_dev.set_target_local_linear_vel(-vf, vf, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(0);
                break;
            case ',':
                // chassisPrint(-vf, 0, 0);
                devices::robot_dev.set_target_local_linear_vel(-vf, 0, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(0);
                break;
            case '.':
                // chassisPrint(-vf, -vf, 0);
                devices::robot_dev.set_target_local_linear_vel(-vf, -vf, is_velocity_local);
                devices::robot_dev.set_target_angular_vel(0);
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

        kernel::scheduler::sleep_ms(50);
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), robotTeleop,
                 main_robot_teleop, robot teleop);