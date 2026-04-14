#pragma once

#include "half_float.h"
#include "inttypes.h"

namespace devices::nrfm_decoder
{

enum NRFMPacketType
{
    NRFM_OLD_FORMAT = 0x0,
    NRFM_KICKER_AND_DRIBBLER = 0x1,
    NRFM_SPEED_CONTROL = 0x2,
    NRFM_COORDINATE_CONTROL = 0x3,
    NRFM_GLOBAL_COORDINATE = 0x4,
    NRFM_CAP_VEL_AND_ACCEL = 0x5,
    NRFM_DEBUG_OVERRIDE = 0xA,
    NRFM_NEW_OLD_FORMAT = 0xF,
};

union NRFMPacket
{
    uint8_t data[32];
    struct __attribute__((packed, aligned(1)))
    {
        uint8_t robot_id : 4;  // SWAPPED FOR SOME REASON
        uint8_t packet_type : 4;
        union payload_t
        {
            struct __attribute__((packed, aligned(1))) old_format_t
            {
                int8_t velx;
                int8_t vely;
                int8_t angular_velocity_or_angle;
                uint8_t dribbler_setting : 4;
                uint8_t kicker_setting : 4;
                bool reserved : 1;
                bool AF : 1;  // Прямой автокик
                bool AU : 1;  // Навесной автокик
                bool KF : 1;  // Прямой кикер
                bool KU : 1;  // Навесной кикер
                bool AS : 1;  // Angle vel or angle switch
                bool DE : 1;  // Dribbler enable
                bool HE : 1;  // High voltage enable
            } old_format;
            struct __attribute__((packed, aligned(1))) global_coordinate_t
            {
                int16_t coorx : 12;
                int16_t coory : 12;
                int8_t angle : 8;
            } global_coordinate;
        } payload;
    } packet;
};

void nrfm_rx_callback(uint8_t *data, uint8_t len);

}  // namespace devices::nrfm_decoder
