#pragma once

#include "drivers/driver_manager.hpp"

namespace devices::bldc
{

enum CANFuocoRegisterMap
{
    EMERGENCY_STOP_W = 0,
    SUPPLY_VOLTAGE_R = 1,
    MONITOR_SETTINGS_RW = 4,

    // State variables
    TARGET_RW = 10,
    FEED_FORWARD_VELOCITY_R = 11,
    SHAFT_ANGLE_R = 12,
    ELECTRICAL_ANGLE_R = 13,
    SHAFT_VELOCITY_R = 14,
    CURRENT_SP_R = 15,
    SHAFT_VELOCITY_SP_R = 16,
    SHAFT_ANGLE_SP_R = 17,
    VOLTAGE_D_R = 18,
    VOLTAGE_Q_R = 19,
    CURRENT_D_R = 20,
    CURRENT_Q_R = 21,
    VOLTAGE_BEMF_R = 22,
    U_ALPHA_R = 23,
    U_BETA_R = 24,

    // Motor configuration parameters
    VOLTAGE_SENSOR_ALIGN_RW = 25,
    VELOCITY_INDEX_SEARCH_RW = 26,

    // Motor physical parameters
    PHASE_RESISTANCE_RW = 27,
    POLE_PAIRS_RW = 28,
    KV_RATING_RW = 29,
    PHASE_INDUCTANCE_RW = 30,

    // Limiting variables
    VOLTAGE_LIMIT_RW = 31,
    CURRENT_LIMIT_RW = 32,
    VELOCITY_LIMIT_RW = 33,

    // Motor status
    ENABLED_RW = 34,
    MOTOR_STATUS_R = 35,

    // PWM modulation settings
    FOC_MODULATION_RW = 36,
    MODULATION_CENTERED_RW = 37,

    // Control mode
    TORQUE_CONTROLLER_RW = 38,
    CONTROLLER_RW = 39,

    // Controller and low pass filter settings
    PID_CURRENT_D_P_RW = 40,
    PID_CURRENT_D_I_RW = 41,
    PID_CURRENT_D_D_RW = 42,
    PID_CURREND_D_OUTPUT_RAMP_RW = 43,
    PID_CURRENT_D_LIMIT_RW = 44,
    LPF_CURRENT_D_TF_RW = 45,
    PID_CURRENT_Q_P_RW = 46,
    PID_CURRENT_Q_I_RW = 47,
    PID_CURRENT_Q_D_RW = 48,
    PID_CURRENT_Q_OUTPUT_RAMP_RW = 49,
    PID_CURRENT_Q_LIMIT_RW = 50,
    LPF_CURRENT_Q_TF_RW = 51,
    PID_VELOCITY_P_RW = 52,
    PID_VELOCITY_I_RW = 53,
    PID_VELOCITY_D_RW = 54,
    PID_VELOCITY_OUTPUT_RAMP_RW = 55,
    PID_VELOCITY_LIMIT_RW = 56,
    LPF_VELOCITY_TF_RW = 57,
    P_ANGLE_P_RW = 58,
    P_ANGLE_LIMIT_RW = 59,
    LPF_ANGLE_TF_RW = 60,
    MOTION_DOWNSAMPLE_RW = 61,
    MOTION_CNT_RW = 62,

    // Sensor settings
    SENSOR_OFFSET_RW = 63,
    ZERO_ELECTRIC_ANGLE_RW = 64,
    SENSOR_DIRECTION_RW = 65,
    PP_CHECK_RESULT_RW = 66,

    // Virtual registers
    MULTI_TARGET_W = 96,
    SHAFT_FEEDBACK_R = 102,
    SAVE_TO_EEPROM_W = 127,
};

union BldcsVec
{
    float vec[4];
    struct
    {
        float fl;
        float rl;
        float rr;
        float fr;
    };
};

struct BldcsConfig
{
    drivers::can::CanDriver *can;
    float drive_vel_p;
    float drive_vel_i;
    float drive_vel_limit;
};

class BldcsDriver : public BldcsConfig
{
  private:
    BldcsVec current;
    BldcsVec current_vel;
    BldcsVec current_pos;

    BldcsVec target_vel;

    void motorWrite(uint8_t motor_addr, uint8_t register_addr, uint8_t *data, uint8_t len);

  public:
    BldcsDriver(BldcsConfig config) : BldcsConfig(config) {}

    void init();

    void setDriveSettings(bool is_verbose = false);

    void setDriveVel(BldcsVec vel);
    void getDriveVel(BldcsVec *vel);

    void setDribblerVel(float vel);
    void getDribblerVel(float *vel);

    void setDriveRegisterFloat(uint8_t register_addr, float value);
};

}  // namespace devices::bldc
