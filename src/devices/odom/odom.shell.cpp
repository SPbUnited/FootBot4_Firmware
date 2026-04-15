#include "devices/device_manager.hpp"
#include "devices/shell/shell.main.hpp"
#include "kernel/kernel.hpp"
#include "vt_kalman"

int odomUpdate()
{
    getshell();

    vt::real_t dt1 = 1.0;
    vt::real_t base_noise_value = 0.001;

    vt::numeric_matrix<2, 2> F({{1, dt1}, {0, 1}});
    vt::numeric_matrix<2, 1> B({{1}, {0}});
    vt::numeric_matrix<1, 2> H({{1, 0}});
    vt::numeric_matrix<2, 2> Q = vt::numeric_matrix<2, 2>::diagonals(base_noise_value);
    vt::numeric_matrix<1, 1> R = vt::numeric_matrix<1, 1>::diagonals(base_noise_value);
    vt::numeric_vector<2> x0;  // {x_, v_x}

    vt::kalman_filter_t<2, 1, 1> kf(F, B, H, Q, R, x0);

    vt::real_t u = 0;

    while (true)
    {
        char c = getchar();
        if (c == -1)
        {
            printf("\nError! Cannot read keyboard input!");
            break;
        }
        if (c == 'q')
        {
            break;
        }

        if (c == 'w')
        {
            u = 1.0;
        }
        else if (c == 'x')
        {
            u = -1.0;
        }
        else if (c == 's')
        {
            u = 0.0;
        }

        kf.predict(vt::make_numeric_vector({u}));

        printf("%f, %f, %f\n", u, kf.state_vector[0], kf.state_vector[1]);

        kernel::scheduler::delay_ms(100);
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), odomUpdate,
                 odomUpdate, odom update);
