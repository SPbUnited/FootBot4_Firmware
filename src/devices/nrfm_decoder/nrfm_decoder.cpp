#include "nrfm_decoder.hpp"

#include "decoders/debug_override.hpp"
#include "decoders/new_old_and_old_format.hpp"
// #include "devices/device_manager.hpp"

namespace devices::nrfm_decoder
{

void NRFMDecoder::nrfm_rx_callback(uint8_t *data, uint8_t len)
{
    NRFMPacket *packet = reinterpret_cast<NRFMPacket *>(data);

    if (packet->packet.robot_id != robot_dev.robot_id)
    {
        return;
    }

    bool is_phantom_packet = true;

    for (size_t i = 1; i < 4; i++)
    {
        if (packet->data[i] != packet->data[0])
        {
            is_phantom_packet = false;
            break;
        }
    }

    if (is_phantom_packet)
    {
        // kerror(
        //     "Detected phantom packet: [%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X "
        //     "%02X %02X %02X %02X "
        //     "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X]",
        //     packet->data[0], packet->data[1], packet->data[2], packet->data[3], packet->data[4],
        //     packet->data[5], packet->data[6], packet->data[7], packet->data[8], packet->data[9],
        //     packet->data[10], packet->data[11], packet->data[12], packet->data[13],
        //     packet->data[14], packet->data[15], packet->data[16], packet->data[17],
        //     packet->data[18], packet->data[19], packet->data[20], packet->data[21],
        //     packet->data[22], packet->data[23], packet->data[24], packet->data[25],
        //     packet->data[26], packet->data[27], packet->data[28], packet->data[29],
        //     packet->data[30], packet->data[31]);
        return;
    }

    kverbose(
        "nrfm_packet: [%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X "
        "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X]",
        packet->data[0], packet->data[1], packet->data[2], packet->data[3], packet->data[4],
        packet->data[5], packet->data[6], packet->data[7], packet->data[8], packet->data[9],
        packet->data[10], packet->data[11], packet->data[12], packet->data[13], packet->data[14],
        packet->data[15], packet->data[16], packet->data[17], packet->data[18], packet->data[19],
        packet->data[20], packet->data[21], packet->data[22], packet->data[23], packet->data[24],
        packet->data[25], packet->data[26], packet->data[27], packet->data[28], packet->data[29],
        packet->data[30], packet->data[31]);

    kverbose(" packet_type = %d, robot_id = %d", packet->packet.packet_type,
             packet->packet.robot_id);

    dts_led.toggle();

    switch (packet->packet.packet_type)
    {
        case NRFM_OLD_FORMAT:
            if (len != sizeof(packet->packet.payload.old_format) + 1)
            {
                kerror("Old format packet length mismatch: expected %d, got %d",
                       sizeof(packet->packet.payload.old_format) + 1, len);
                return;
            }
            kdebug("Old format packet");
            new_old_and_old_format(packet);
            break;
        case NRFM_NEW_OLD_FORMAT:
            if (len != sizeof(packet->packet.payload.old_format) + 1)
            {
                kerror("New old format packet length mismatch: expected %d, got %d",
                       sizeof(packet->packet.payload.old_format) + 1, len);
                return;
            }
            kdebug("New old format packet");
            new_old_and_old_format(packet, true);
            break;
        case NRFM_DEBUG_OVERRIDE:
            kdebug("Debug override packet");
            debug_override(packet, len);
            break;
        default:
            kerror("Unknown packet type %d", packet->packet.packet_type);
            break;
    }
}

}  // namespace devices::nrfm_decoder
