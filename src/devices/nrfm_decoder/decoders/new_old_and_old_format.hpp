#pragma once

#include "../nrfm_decoder.hpp"
#include "devices/device_manager.hpp"
#include "half_float.h"

namespace devices::nrfm_decoder::decoders
{

void new_old_and_old_format(NRFMPacket *packet, bool is_new_old_format = false)
{
    kverbose("new_old_and_old_format(is_new_old_format = %d)", is_new_old_format);

    constexpr float cm_to_m = 0.01f;
    constexpr float pi = 3.1415926f;

    float velx = is_new_old_format ? fp8_to_fp32(packet->packet.payload.old_format.velx) * cm_to_m
                                   : packet->packet.payload.old_format.velx / 127.0f *
                                         devices::robot_dev.max_linear_vel;
    float vely = is_new_old_format ? fp8_to_fp32(packet->packet.payload.old_format.vely) * cm_to_m
                                   : packet->packet.payload.old_format.vely / 127.0f *
                                         devices::robot_dev.max_linear_vel;
    float angular_velocity =
        is_new_old_format
            ? fp8_to_fp32(packet->packet.payload.old_format.angular_velocity_or_angle)
            : packet->packet.payload.old_format.angular_velocity_or_angle / 127.0f *
                  devices::robot_dev.max_angular_vel;

    float dangle = packet->packet.payload.old_format.angular_velocity_or_angle * pi / 128.0f;

    devices::robot_dev.set_target_linear_vel(velx, vely);
    if (packet->packet.payload.old_format.AS)
    {
        devices::robot_dev.set_target_angular_dpos(dangle);
    }
    else
    {
        devices::robot_dev.set_target_angular_vel(angular_velocity);
    }

    devices::robot_dev.set_kicker_setting(packet->packet.payload.old_format.kicker_setting *
                                          packet->packet.payload.old_format.HE);
    devices::robot_dev.set_dribbler_setting(packet->packet.payload.old_format.dribbler_setting *
                                            packet->packet.payload.old_format.DE);

    devices::robot::KickerMode kicker_mode = devices::robot::DISABLED;

    if (packet->packet.payload.old_format.KF)
    {
        kicker_mode = devices::robot::KICK_FRONT;
    }
    else if (packet->packet.payload.old_format.KU)
    {
        kicker_mode = devices::robot::KICK_UP;
    }
    else if (packet->packet.payload.old_format.AF)
    {
        kicker_mode = devices::robot::AUTOKICK_FRONT;
    }
    else if (packet->packet.payload.old_format.AU)
    {
        kicker_mode = devices::robot::AUTOKICK_UP;
    }

    devices::robot_dev.set_kicker_mode(kicker_mode);
}

}  // namespace devices::nrfm_decoder::decoders
