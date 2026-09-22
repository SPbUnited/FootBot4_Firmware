#pragma once
#include "devices/robot/robot.hpp"
#include "robot_core/protocol.hpp"
namespace devices::nrfm_decoder
{
struct NRFMDecoderConfig
{
    devices::robot::Robot &robot_dev;
    drivers::gpio::GPIOOutputDriver &dts_led;
};
class NRFMDecoder : public NRFMDecoderConfig
{
  public:
    explicit NRFMDecoder(NRFMDecoderConfig &config) : NRFMDecoderConfig(config) {}
    void nrfm_rx_callback(uint8_t *data, uint8_t len);
};
}  // namespace devices::nrfm_decoder
