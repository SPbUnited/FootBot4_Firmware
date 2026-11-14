#pragma once

#include <inttypes.h>
#include <stddef.h>

#include "stm32f4xx_hal.h"

namespace drivers::can
{

struct CanConfig
{
    CAN_TypeDef *instance;
    //
    void (*clk_enable)();
    //
    uint32_t txPin;
    GPIO_TypeDef *txPort;
    uint32_t txAlternate;
    uint32_t rxPin;
    GPIO_TypeDef *rxPort;
    uint32_t rxAlternate;
    //
    uint32_t prescaler;
    uint32_t mode;
    uint32_t sjw;
    uint32_t timeSeg1;
    uint32_t timeSeg2;
    FunctionalState timeTriggeredMode;
    FunctionalState autoBusOff;
    FunctionalState autoWakeUp;
    FunctionalState autoRetrans;
    FunctionalState receiveFifoLocked;
    FunctionalState transmitFifoPriority;
};

class CanDriver : public CanConfig
{
  private:
    struct RxFrame
    {
        uint32_t id;
        uint8_t data[8];
        uint8_t len;
    };

    static constexpr uint8_t RX_QUEUE_SIZE = 32;
    static constexpr uint8_t RX_QUEUE_MASK = RX_QUEUE_SIZE - 1;
    static constexpr uint8_t RX_FIFO0_DEPTH = 3;

    RxFrame rx_queue_[RX_QUEUE_SIZE] = {};
    volatile uint8_t rx_head_ = 0;
    volatile uint8_t rx_tail_ = 0;
    volatile uint32_t rx_queue_drops_ = 0;
    volatile uint32_t rx_fifo_overruns_ = 0;
    volatile uint32_t rx_frames_ = 0;
    volatile uint32_t tx_frames_ = 0;
    volatile uint32_t rx_bits_estimate_ = 0;
    volatile uint32_t last_rx_interval_us_ = 0;
    volatile uint32_t max_rx_interval_us_ = 0;
    volatile uint32_t last_rx_time_us_ = 0;
    volatile uint32_t monitored_frame_counts_[16] = {};
    volatile uint32_t monitored_frame_intervals_us_[16] = {};
    volatile uint32_t monitored_frame_last_times_us_[16] = {};

    friend void handleCan1Rx0Pending(CAN_HandleTypeDef *handle);
    friend void handleCanError(CAN_HandleTypeDef *handle);

    void receivePending();
    bool enqueueFromIsr(const CAN_RxHeaderTypeDef &header, const uint8_t *data);
    void recordRxFrame(const CAN_RxHeaderTypeDef &header);

  public:
    CAN_HandleTypeDef handle;

    CanDriver(CanConfig config);

    void init();
    bool write(uint32_t id, const uint8_t *data, uint8_t len);
    /** Read one frame copied from the RX interrupt queue. */
    bool read(uint32_t *id, uint8_t *data, uint8_t *len);
    /** Return frames dropped because the software queue was full. */
    uint32_t rxQueueDrops() const;
    /** Return hardware RX FIFO overrun events. */
    uint32_t rxFifoOverruns() const;
    uint32_t rxFrames() const;
    uint32_t txFrames() const;
    uint32_t rxBitsEstimate() const;
    uint32_t lastRxIntervalUs() const;
    uint32_t maxRxIntervalUs() const;
    void updateMonitorWindow();
};

extern "C"
{
    extern volatile uint32_t can_monitor_rx_frames;
    extern volatile uint32_t can_monitor_tx_frames;
    extern volatile uint32_t can_monitor_rx_bits_estimate;
    extern volatile uint32_t can_monitor_rx_rate_millihz;
    extern volatile uint32_t can_monitor_tx_rate_millihz;
    extern volatile uint32_t can_monitor_bus_load_rx_permille;
    extern volatile uint32_t can_monitor_rx_interval_us;
    extern volatile uint32_t can_monitor_rx_interval_max_us;
    extern volatile uint32_t can_monitor_rx_queue_drops;
    extern volatile uint32_t can_monitor_rx_fifo_overruns;
    extern volatile uint32_t can_monitor_last_rx_id;
    extern volatile uint32_t can_monitor_last_rx_dlc;
    extern volatile uint32_t can_monitor_motor_register_frames[16];
    extern volatile uint32_t can_monitor_motor_register_interval_us[16];
    extern volatile uint32_t can_monitor_motor_register_ids[16];
}

void handleCan1Rx0Interrupt();
void handleCan1Rx0Pending(CAN_HandleTypeDef *handle);
void handleCanError(CAN_HandleTypeDef *handle);

}  // namespace drivers::can
