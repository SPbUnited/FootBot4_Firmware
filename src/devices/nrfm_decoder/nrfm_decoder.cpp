#include "nrfm_decoder.hpp"
namespace devices::nrfm_decoder
{
void NRFMDecoder::nrfm_rx_callback(uint8_t *data, uint8_t len)
{
    if (!data || len == 0 || len > 32 || (data[0] & 15) != robot_dev.robot_id)
        return;
    // Raw motor register overrides are a board debug service, not controller commands.
    if ((data[0] >> 4) == 0xA)
    {
        if (len >= 5 && data[1] == 0x0C && len - 5 <= 8)
        {
            robot_dev.bldcs_drv.motorWrite(data[3], data[4], data + 5, len - 5);
            dts_led.toggle();
        }
        return;
    }
    robot_core::RobotCommand command;
    if (robot_core::decode_command(data, len, robot_dev, drivers::system_clock::micros(),
                                   command) == robot_core::DecodeStatus::accepted)
    {
        robot_core::apply_command(robot_dev, command);
        dts_led.toggle();
    }
}
}  // namespace devices::nrfm_decoder
