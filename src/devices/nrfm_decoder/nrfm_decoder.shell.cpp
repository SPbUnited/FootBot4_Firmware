#include "devices/device_manager.hpp"

void nrfmTestOldPacket(int8_t velx, int8_t vely, int8_t angular_velocity_or_angle,
                       uint8_t kicker_setting, uint8_t dribbler_setting, uint8_t flags)
{
    devices::nrfm_decoder::NRFMPacket packet = {
        .packet = {.robot_id = 15,
                   .packet_type = devices::nrfm_decoder::NRFM_OLD_FORMAT,
                   .payload = {
                       .old_format =
                           {
                               .velx = velx,
                               .vely = vely,
                               .angular_velocity_or_angle = angular_velocity_or_angle,
                               .dribbler_setting = dribbler_setting,
                               .kicker_setting = kicker_setting,
                               .HE = flags & 0b00000001,
                               .DE = flags & 0b00000010,
                               .AS = flags & 0b00000100,
                               .AU = flags & 0b00100000,
                               .AF = flags & 0b01000000,
                               .KU = flags & 0b00001000,
                               .KF = flags & 0b00010000,
                               .reserved = flags & 0b10000000,
                           },
                   }}};

    devices::nrfm_decoder_dev.nrfm_rx_callback(packet.data, 6);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), nrfmTestOldPacket,
                 nrfmTestOldPacket, nrfmTestOldPacket);

void nrfmTestNewOldPacket(int8_t velx, int8_t vely, int8_t angular_velocity_or_angle,
                          uint8_t kicker_setting, uint8_t dribbler_setting, uint8_t flags)
{
    devices::nrfm_decoder::NRFMPacket packet = {
        .packet = {.robot_id = 15,
                   .packet_type = devices::nrfm_decoder::NRFM_NEW_OLD_FORMAT,
                   .payload = {
                       .old_format =
                           {
                               .velx = velx,
                               .vely = vely,
                               .angular_velocity_or_angle = angular_velocity_or_angle,
                               .dribbler_setting = dribbler_setting,
                               .kicker_setting = kicker_setting,
                               .HE = flags & 0b00000001,
                               .DE = flags & 0b00000010,
                               .AS = flags & 0b00000100,
                               .AU = flags & 0b00100000,
                               .AF = flags & 0b01000000,
                               .KU = flags & 0b00001000,
                               .KF = flags & 0b00010000,
                               .reserved = flags & 0b10000000,
                           },
                   }}};

    devices::nrfm_decoder_dev.nrfm_rx_callback(packet.data, 6);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC),
                 nrfmTestNewOldPacket, nrfmTestNewOldPacket, nrfmTestNewOldPacket);
