#include "uart_control.hpp"

#include <cstdio>
#include <cstring>

#include "kernel/kernel.hpp"
#include "devices/device_manager.hpp"

namespace devices::uart_control
{

// Static member definitions
uint32_t UartControl::m_prevTime_ms = 0;
uint32_t UartControl::m_packetsReceived = 0;
uint32_t UartControl::m_packetsReceived_1s = 0;

uint8_t UartControl::m_incomeArray[UartControl::m_incomePacketLen];
uint8_t UartControl::m_lenDbg;

UartControl::UartControl(UartControlConfig &uart_control_config) : UartControlConfig(uart_control_config) {}

void UartControl::recv_irq()
{
    uint8_t buf[512];
    uint16_t available = 0;

    while (uart_instance.getc(reinterpret_cast<char *>(&buf[available])) == HAL_OK && available < sizeof(buf))
    {
        available++;
    }

    if (available == 0)
    {
        return;
    }

    m_lastPacketTime = HAL_GetTick();
    m_packetsReceived++;

    uint32_t time_ms = HAL_GetTick();
    if (time_ms - m_prevTime_ms > 1000)
    {
        m_prevTime_ms = time_ms;
        m_packetsReceived_1s = m_packetsReceived;
        m_packetsReceived = 0;
    }

    UartPacket packet = {{}, static_cast<uint8_t>(available)};

    for (size_t i = 0; i < available; i++)
    {
        packet.data[i] = buf[i];
    }

    m_rx_queue.push_back(packet);
}

int UartControl::fetch()
{
    while (!m_rx_queue.isEmpty())
    {
        UartPacket packet = m_rx_queue.pop_front();

        char json[256];
        memcpy(json, packet.data, packet.len);
        json[packet.len] = '\0';

        process_command(json);
    }

    return 0;
}

void UartControl::process_command(const char *json)
{
    CommandData cmd = {};

    const char *p = json;

    auto find_int = [&]() -> int {
        while (*p && !(*p >= '0' && *p <= '9') && *p != '-') p++;
        return atoi(p);
    };

    auto find_float = [&]() -> float {
        while (*p && !(*p >= '0' && *p <= '9') && *p != '-') p++;
        return atof(p);
    };

    auto find_bool = [&]() -> uint8_t {
        while (*p && *p != ':') p++;
        p++;
        while (*p == ' ') p++;
        if (*p == 't') return 1;
        if (*p == '1') return 1;
        return 0;
    };

    p = strstr(json, "\"xvel\"");
    if (p) { p += 6; cmd.xvel = find_float(); }

    p = strstr(json, "\"yvel\"");
    if (p) { p += 6; cmd.yvel = find_float(); }

    p = strstr(json, "\"wvel\"");
    if (p) { p += 6; cmd.wvel = find_float(); }

    p = strstr(json, "\"dribbler\"");
    if (p) { p += 10; cmd.dribbler = find_int(); }

    p = strstr(json, "\"voltage\"");
    if (p) { p += 9; cmd.voltage = find_int(); }

    p = strstr(json, "\"kick_lower\"");
    if (p) { cmd.kick_lower = find_bool(); }

    p = strstr(json, "\"kick_upper\"");
    if (p) { cmd.kick_upper = find_bool(); }

    p = strstr(json, "\"autokick_lower\"");
    if (p) { cmd.autokick_lower = find_bool(); }

    p = strstr(json, "\"autokick_upper\"");
    if (p) { cmd.autokick_upper = find_bool(); }

    p = strstr(json, "\"autokick_momentum\"");
    if (p) { cmd.autokick_momentum = find_bool(); }

    p = strstr(json, "\"angle_mode\"");
    if (p) { cmd.angle_mode = find_bool(); }

    robot_dev.set_target_local_linear_vel(cmd.xvel, cmd.yvel, true);
    if (cmd.angle_mode)
    {
        robot_dev.set_target_angular_dpos(cmd.wvel);
    }
    else
    {
        robot_dev.set_target_angular_vel(cmd.wvel);
    }
    robot_dev.set_dribbler_setting(cmd.dribbler);
    robot_dev.set_kicker_setting(cmd.voltage);

    robot::KickerMode kicker_mode = robot::DISABLED;

    if (cmd.kick_lower)
    {
        kicker_mode = robot::KICK_FRONT;
    }
    else if (cmd.kick_upper)
    {
        kicker_mode = robot::KICK_UP;
    }
    else if (cmd.autokick_lower)
    {
        if (cmd.autokick_upper)
        {
            kicker_mode = robot::AUTOKICK_MOMENTUM;
        }
        else
        {
            kicker_mode = robot::AUTOKICK_FRONT;
        }
    }
    else if (cmd.autokick_upper)
    {
        kicker_mode = robot::AUTOKICK_UP;
    }

    robot_dev.set_kicker_mode(kicker_mode);
}

bool UartControl::init()
{
    uart_instance.init();
    return false;
}

void UartControl::send_telemetry()
{
    devices::bno055_drv.get_angles();
    float yaw = devices::bno055_drv.euler.yaw;
    float pitch = devices::bno055_drv.euler.pitch;
    float roll = devices::bno055_drv.euler.roll;

    float voltage = devices::kicker_drv.get_voltage();

    float odom_x = devices::odom_dev.state.x;
    float odom_y = devices::odom_dev.state.y;
    float odom_theta = devices::odom_dev.state.theta;

    uint8_t ball_deep = devices::kicker_drv.read_deep();
    uint8_t ball_front = devices::kicker_drv.read_front();

    char json[256];
    snprintf(json, sizeof(json),
        "{\"id\":%i,"
        "\"voltage\":%.1f,"
        "\"ball\":{\"deep\":%d,\"front\":%d},"
        "\"wvel_tgt\":%.2f,"
        "\"wvel_out\":%.2f}",
        devices::robot_dev.robot_id,
        voltage,
        ball_deep, ball_front,
        devices::robot_dev.vel_global_target.theta,
        devices::robot_dev.vel_local_output_smoothed.theta);

    for (uint16_t i = 0; i < strlen(json); i++)
    {
        uart_instance.putc(json[i]);
    }
    uart_instance.putc('\n');
}

}  // namespace devices::uart_control
