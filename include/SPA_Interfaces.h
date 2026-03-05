#pragma once

struct SenseIface
{
    // UART:IMU
    struct Imu
    {
        float accel[3];
        float gyro[3];
        float mag[3];
    } imu;

    // UART:Debug
    // ???

    // SPI:NRF

    // CAN:BLDC
    struct Bldc
    {
        float voltage;
        float current;
        float power;
    } bldc[5];
};

struct ActIface
{
    // UART:Debug

    // SPI:NRF

    // SPI:SDcard

    // I2C:OLED

    // CAN:BLDC
};
