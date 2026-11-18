#pragma once

#include "../nrfm_decoder.hpp"
#include "half_float.h"
#include "kernel/kernel.hpp"

namespace devices::nrfm_decoder
{

void NRFMDecoder::debug_override(NRFMPacket *packet, uint8_t len)
{
    kinfo("debug_override()");

    switch (packet->data[0])
    {
        default:
        case 0x0A:
        {
            // Motherboard
            uint8_t REG_ID = packet->packet.payload.debug_override.data[1];
            uint8_t *PAYLOAD = &packet->packet.payload.debug_override.data[2];
            uint8_t PAYLOAD_LEN = len - 3;
            uint8_t reversed_payload[4] = {0x00, 0x00, 0x00, 0x00};
            for (int i = 0; i < 4; i++)
            {
                reversed_payload[i] = PAYLOAD[PAYLOAD_LEN - 1 - i];
            }
            // robot_dev.angle_kp = PAYLOAD;
            // 
            robot_dev.nrfm_callback(REG_ID, reversed_payload, PAYLOAD_LEN);
            break;
        }
        case 0x0C:
        {
            uint8_t DRV_ID = packet->packet.payload.debug_override.data[2];
            uint8_t REG_ID = packet->packet.payload.debug_override.data[3];
            uint8_t *PAYLOAD = &packet->packet.payload.debug_override.data[4];
            uint8_t PAYLOAD_LEN = len - 4;
            robot_dev.bldcs_drv.motorWrite(DRV_ID, REG_ID, PAYLOAD, PAYLOAD_LEN);
            kverbose("Debug override: DRV_ID = %d, REG_ID = %d, PAYLOAD_LEN = %d", DRV_ID, REG_ID,
                     PAYLOAD_LEN);
            break;
        }
        
    }
}

}  // namespace devices::nrfm_decoder
