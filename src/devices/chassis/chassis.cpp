#include "chassis.hpp"

#include <math.h>

#include "devices/device_manager.hpp"
#include "vt_linalg"

namespace devices::chassis
{

Chassis::Chassis(ChassisConfig config) : ChassisConfig(config) {}

template <typename T, size_t Row, size_t Col>
void print_matrix(const vt::generic_matrix<T, Row, Col> &matrix)
{
    char buffer[512];
    size_t offset = 0;
    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");
    for (size_t i = 0; i < matrix.r(); ++i)
    {
        for (size_t j = 0; j < matrix.c(); ++j)
        {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%06.4f ", matrix[i][j]);
        }
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");
    }
    kdebug("%s", buffer);
}

void Chassis::init()
{
    kdebug(
        "Chassis initialized:\n  wheel_angles: %f %f %f %f\n  wheel_radius: %f\n  "
        "robot_radius: %f",
        wheel_angles[0], wheel_angles[1], wheel_angles[2], wheel_angles[3], wheel_radius,
        robot_radius);

    J = vt::numeric_matrix<4, 3>({
        {-cos(wheel_angles[0]), -sin(wheel_angles[0]), robot_radius},
        {-cos(wheel_angles[1]), -sin(wheel_angles[1]), robot_radius},
        {-cos(wheel_angles[2]), -sin(wheel_angles[2]), robot_radius},
        {-cos(wheel_angles[3]), -sin(wheel_angles[3]), robot_radius},
    });

    Jinv = vt::numeric_matrix<3, 4>({{-0.394976, 0.394976, 0.394976, -0.394976},
                                     {-0.372225, -0.276332, 0.276332, 0.372225},
                                     {3.95441, 2.58154, 2.58154, 3.95441}});

    kdebug("J:");
    print_matrix(J);
    kdebug("Jinv:");
    print_matrix(Jinv);
}

void Chassis::setVel(StateVector vel)
{
    kdebug("setVel: %f %f %f", vel.x, vel.y, vel.theta);

    bldc::BldcsVec wheel_vels;

    vt::numeric_vector<3> vel_vec({vel.x, vel.y, vel.theta});

    vt::numeric_vector<4> w =
        vt::numeric_matrix<4, 4>::diagonals(1.0 / wheel_radius) * J * vel_vec;

    for (int i = 0; i < 4; i++)
    {
        wheel_vels.vec[i] = w[i];
    }

    bldcs->setDriveVel(wheel_vels);
}

void Chassis::getVel(StateVector *vel, bldc::BldcsVec wheel_vels)
{
    kdebug("getVel: %f %f %f %f", wheel_vels.vec[0], wheel_vels.vec[1], wheel_vels.vec[2],
           wheel_vels.vec[3]);

    vt::numeric_vector<4> w(
        {wheel_vels.vec[0], wheel_vels.vec[1], wheel_vels.vec[2], wheel_vels.vec[3]});

    vt::numeric_vector<3> vel_vec = Jinv * vt::numeric_matrix<4, 4>::diagonals(wheel_radius) * w;

    vel->x = vel_vec[0];
    vel->y = vel_vec[1];
    vel->theta = vel_vec[2];

    kdebug("getVel: %f %f %f", vel->x, vel->y, vel->theta);
}

}  // namespace devices::chassis