#include "devices/device_manager.hpp"
namespace
{
void testPacket(uint8_t type, int8_t x, int8_t y, int8_t angular, uint8_t kicker, uint8_t dribbler,
                uint8_t flags)
{
    // Preserve shell flag order (KU/KF before AU/AF), convert to radio bit order.
    const uint8_t wire_flags = (flags & 0x87) | ((flags & 0x18) << 2) | ((flags & 0x60) >> 2);
    uint8_t bytes[] = {static_cast<uint8_t>((type << 4) | devices::robot_dev.robot_id),
                       static_cast<uint8_t>(x),
                       static_cast<uint8_t>(y),
                       static_cast<uint8_t>(angular),
                       static_cast<uint8_t>(((kicker & 15) << 4) | (dribbler & 15)),
                       wire_flags};
    devices::nrfm_decoder_dev.nrfm_rx_callback(bytes, sizeof(bytes));
}
}  // namespace
void nrfmTestOldPacket(int8_t x, int8_t y, int8_t angular, uint8_t kicker, uint8_t dribbler,
                       uint8_t flags)
{
    testPacket(0, x, y, angular, kicker, dribbler, flags);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), nrfmTestOldPacket,
                 nrfmTestOldPacket, nrfmTestOldPacket);
void nrfmTestNewOldPacket(int8_t x, int8_t y, int8_t angular, uint8_t kicker, uint8_t dribbler,
                          uint8_t flags)
{
    testPacket(15, x, y, angular, kicker, dribbler, flags);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC),
                 nrfmTestNewOldPacket, nrfmTestNewOldPacket, nrfmTestNewOldPacket);
