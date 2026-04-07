#include "nrfm_decoder.hpp"

#include "decoders/new_old_and_old_format.hpp"
#include "devices/device_manager.hpp"

namespace devices::nrfm_decoder
{

void nrfm_rx_callback(uint8_t *data, uint8_t len)
{
    NRFMPacket *packet = reinterpret_cast<NRFMPacket *>(data);

    if (packet->packet.robot_id != devices::robot_dev.robot_id)
    {
        return;
    }

    switch (packet->packet.packet_type)
    {
        case NRFM_OLD_FORMAT:
            kdebug("Old format packet");
            decoders::new_old_and_old_format(packet);
            break;
        case NRFM_NEW_OLD_FORMAT:
            kdebug("New old format packet");
            decoders::new_old_and_old_format(packet, true);
            break;
        default:
            kerror("Unknown packet type %d", packet->packet.packet_type);
            break;
    }
}

void nrfmTestOldPacket(int8_t velx, int8_t vely, int8_t angular_velocity_or_angle,
                       uint8_t kicker_setting, uint8_t dribbler_setting, uint8_t flags)
{
    NRFMPacket packet = {
        .packet = {.packet_type = NRFM_OLD_FORMAT,
                   .robot_id = 15,
                   .payload = {
                       .old_format =
                           {
                               .velx = velx,
                               .vely = vely,
                               .angular_velocity_or_angle = angular_velocity_or_angle,
                               .kicker_setting = kicker_setting,
                               .dribbler_setting = dribbler_setting,
                               .reserved = flags & 0b10000000,
                               .AF = flags & 0b01000000,
                               .AU = flags & 0b00100000,
                               .KF = flags & 0b00010000,
                               .KU = flags & 0b00001000,
                               .AS = flags & 0b00000100,
                               .DE = flags & 0b00000010,
                               .HE = flags & 0b00000001,
                           },
                   }}};

    nrfm_rx_callback(packet.data, 32);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), nrfmTestOldPacket,
                 nrfmTestOldPacket, nrfmTestOldPacket);

void nrfmTestNewOldPacket(int8_t velx, int8_t vely, int8_t angular_velocity_or_angle,
                          uint8_t kicker_setting, uint8_t dribbler_setting, uint8_t flags)
{
    NRFMPacket packet = {
        .packet = {.packet_type = NRFM_NEW_OLD_FORMAT,
                   .robot_id = 15,
                   .payload = {
                       .old_format =
                           {
                               .velx = velx,
                               .vely = vely,
                               .angular_velocity_or_angle = angular_velocity_or_angle,
                               .kicker_setting = kicker_setting,
                               .dribbler_setting = dribbler_setting,
                               .reserved = flags & 0b10000000,
                               .AF = flags & 0b01000000,
                               .AU = flags & 0b00100000,
                               .KF = flags & 0b00010000,
                               .KU = flags & 0b00001000,
                               .AS = flags & 0b00000100,
                               .DE = flags & 0b00000010,
                               .HE = flags & 0b00000001,
                           },
                   }}};

    nrfm_rx_callback(packet.data, 32);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC),
                 nrfmTestNewOldPacket, nrfmTestNewOldPacket, nrfmTestNewOldPacket);

}  // namespace devices::nrfm_decoder
