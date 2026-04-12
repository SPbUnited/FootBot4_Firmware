// #include "nrf.hpp"
// #include <logging/log.h>
// #include <math.h>
// // #include "../cannabus/half_float.h"
// #include "minifloat3.h"

// LOG_MODULE_REGISTER(nrf_recv, LOG_LEVEL_ERR);

// uint8_t Nrf24Recv::m_address = 0;
// uint32_t Nrf24Recv::m_prevTime_ms = 0;
// uint32_t Nrf24Recv::m_packetsReceived = 0;
// uint32_t Nrf24Recv::m_packetsReceived_1s = 0;
// uint32_t Nrf24Recv::m_ownPacketsReceived = 0;
// uint32_t Nrf24Recv::m_ownPacketsReceived_1s = 0;

// uint8_t Nrf24Recv::m_incomeArray[m_incomePacketLen];
// uint8_t Nrf24Recv::m_iArray[8];
// const struct spi_dt_spec *Nrf24Recv::spec;
// uint8_t Nrf24Recv::m_lenDbg;

// motion::Control *Nrf24Recv::m_motionControl;
// kicker::Kicker *Nrf24Recv::m_kicker;
// LedDisplay *Nrf24Recv::m_display;
// canbroadcast::CanBroadcast *Nrf24Recv::m_cannabus;

// static uint8_t radioBarrier = 0;
// // uint8_t barrier;
// // static uint8_t Nrf24Recv::m_barrier = 0;

// bool Nrf24Recv::m_isNewPacketRec = 0;
// volatile uint32_t Nrf24Recv::m_lastPacketTime = 0;

// #ifdef CONFIG_BOARD_stm32f429zi
// static const struct gpio_dt_spec led1_r = GPIO_DT_SPEC_GET(DT_N_ALIAS_led1_r, gpios);
// static const struct gpio_dt_spec led2_r = GPIO_DT_SPEC_GET(DT_N_ALIAS_led2_r, gpios);
// static const struct gpio_dt_spec led3_r = GPIO_DT_SPEC_GET(DT_N_ALIAS_led3_r, gpios);
// static const struct gpio_dt_spec led4_r = GPIO_DT_SPEC_GET(DT_N_ALIAS_led4_r, gpios);
// static const struct gpio_dt_spec led5_r = GPIO_DT_SPEC_GET(DT_N_ALIAS_led5_r, gpios);

// static const struct gpio_dt_spec led1_b = GPIO_DT_SPEC_GET(DT_N_ALIAS_led1_b, gpios);
// static const struct gpio_dt_spec led2_b = GPIO_DT_SPEC_GET(DT_N_ALIAS_led2_b, gpios);
// static const struct gpio_dt_spec led3_b = GPIO_DT_SPEC_GET(DT_N_ALIAS_led3_b, gpios);
// static const struct gpio_dt_spec led4_b = GPIO_DT_SPEC_GET(DT_N_ALIAS_led4_b, gpios);
// static const struct gpio_dt_spec led5_b = GPIO_DT_SPEC_GET(DT_N_ALIAS_led5_b, gpios);

// static const struct gpio_dt_spec ledl_b = GPIO_DT_SPEC_GET(DT_N_ALIAS_ledl_b, gpios);

// static const gpio_dt_spec *redLed[5] = {&led1_r, &led2_r, &led3_r, &led4_r, &led5_r};
// static const gpio_dt_spec *blueLed[5] = {&led1_b, &led2_b, &led3_b, &led4_b, &led5_b};
// #endif
// uint32_t timeout_nrf_timer_recv = 0;
// int Nrf24Recv::recv()
// {
//     volatile int a = 0;
//     // while (1)
//     // {
//     int rc = 0;
//     static uint32_t time_ms;
//     uint8_t reg = 1;
//     time_ms = k_uptime_get();
//     rc = readReg(7, &reg);
//     if (rc < 0)
//         LOG_ERR("Error in read 7 reg");
//     if (reg & 0x40)
//     {
//         // узнаю длину
//         timeout_nrf_timer_recv = k_uptime_get();
//         rc = readReg(0x60, &m_lenDbg);
//         if (rc < 0)
//             LOG_ERR("Error in read 0x60 reg");

//         if (m_lenDbg < 6)
//         {
//             LOG_ERR("Error in message lenght");
//         }
//         if (time_ms - m_prevTime_ms > 1000)
//         {
//             m_prevTime_ms = time_ms;
//             m_packetsReceived_1s = m_packetsReceived;
//             m_packetsReceived = 0;
//             m_ownPacketsReceived_1s = m_ownPacketsReceived;
//             m_ownPacketsReceived = 0;
//         }
//         m_packetsReceived++;
//         if (m_lenDbg == 6)
//         {
//         }
//         // uint8_t *m_incomeArray = new uint8_t[m_lenDbg];
//         if (m_lenDbg == 6)
//             rawRead(0x61, m_incomeArray, m_lenDbg);
//         else if (m_lenDbg == 8)
//             rawRead(0x61, m_iArray, m_lenDbg);
//         writeReg(0x07, 0x40);

//         if (m_incomeArray[0] != 0)
//         {
//             LOG_DBG("%d %d %d %d %d %d %d", m_incomeArray[0], m_incomeArray[1], m_incomeArray[2], m_incomeArray[3], m_incomeArray[4], m_incomeArray[5]);
//         }

//         m_lastPacketTime = k_uptime_get();

//         flushRx();
//         m_address = m_display->adrAndCh[0];
//         if (m_lenDbg == 8)
//         {
//             if (m_address + 0xA0 == m_iArray[m_lenDbg - 1])
//             {
//                 uint8_t t_test_arr[4];
//                 memcpy(t_test_arr, m_iArray, 4);
//                 // uint8_t *ptr = t_test_arr;
//                 m_cannabus->sendDebugOverride(t_test_arr, m_iArray[m_lenDbg - 4] + (m_iArray[m_lenDbg - 3] << 8), m_lenDbg);
//             }
//         }
//         if ((m_address != (m_incomeArray[5] & 0x0F)) || (((m_address + 0xF0) == (m_incomeArray[5])) && ((m_address + 0xF0) == (m_incomeArray[4])) && ((m_address + 0xF0) == (m_incomeArray[3])) && ((m_address + 0xF0) == (m_incomeArray[2])) && ((m_address + 0xF0) == (m_incomeArray[1])) && ((m_address + 0xF0) == (m_incomeArray[0]))))
//         {
//             return 0; // continue;
//         }

//         // Тут раскидываем полученный массив по значениям

//         uint8_t flags = m_incomeArray[0];
//         uint8_t kvlSpd = m_incomeArray[1];
//         uint8_t kvlVal = ((kvlSpd & m_kvlMask) >> 4);

//         m_kicker->kicker_voltage_level_10v = kvlVal * 2.2;
//         LOG_INF("kicker_voltage_level %d", m_kicker->kicker_voltage_level_10v);

//         // Понять где он использует!!!
//         m_kicker->dribbler_enable = (flags & m_wpDrbenMask) > 0 ? 1 : 0;
//         // m_kicker->dribbler_enable = 1;
//         m_kicker->kicker_enable = (flags & m_wpKceMask) > 0 ? 1 : 0;
//         // m_kicker->kick_forward = (flags & m_wpKforMask) > 0 ? 1 : 0;
//         // m_kicker->kick_up = (flags & m_wpKupMask) > 0 ? 1 : 0;
//         // m_kicker->force_kick_up = ((flags & m_wpFrcKickUp) > 0) && (flags & m_wpFrcKickForward) == 0 ? 1 : 0;
//         // m_kicker->force_kick_forward = (flags & m_wpFrcKickForward) > 0 ? 1 : 0;

//         // m_kicker->kick_forward = ((flags & 0x08) > 0 ? 1 : 0) && (((flags & 0x20) > 0 ? 1 : 0));
//         // m_kicker->kick_up = ((flags & 0x10) > 0 ? 1 : 0) && (((flags & 0x20) > 0 ? 1 : 0));

//         // m_kicker->force_kick_forward = ((flags & 0x40) > 0 ? 1 : 0) && !(((flags & 0x20) > 0 ? 1 : 0));
//         // m_kicker->force_kick_up = ((flags & 0x10) > 0 ? 1 : 0) && !(((flags & 0x20) > 0 ? 1 : 0));
//         m_kicker->force_kick_up = ((flags & 0x20) > 0 ? 1 : 0);
//         m_kicker->force_kick_forward = ((flags & 0x40) > 0 ? 1 : 0);
//         // m_kicker->force_kick_up = ((flags & m_wpFrcKickUp) > 0) && (flags & m_wpFrcKickForward) == 0 ? 1 : 0;
//         // m_kicker->force_kick_forward = (flags & m_wpFrcKickForward) > 0 ? 1 : 0;

//         m_kicker->kick_up = ((flags & 0x10) > 0 ? 1 : 0);
//         m_kicker->kick_forward = ((flags & 0x08) > 0 ? 1 : 0);
//         // if (m_kicker->force_kick_up)
//         // {
//         //     m_kicker->kick_up = 0;
//         // }
//         // if (m_kicker->force_kick_forward)
//         // {
//         //     m_kicker->kick_forward = 0;
//         // }
//         m_cannabus->m_enableLed = (flags & m_wpLed) > 0 ? 1 : 0;
//         radioBarrier = (flags & 0x80) > 0 ? 1 : 0;

//         LOG_DBG("kicker_enable %d", m_kicker->kicker_enable);
//         LOG_DBG("kick_forward %d", m_kicker->kick_forward);
//         LOG_DBG("kick_up %d ", m_kicker->kick_up);
//         LOG_DBG("force_kick_up %d", m_kicker->force_kick_up);
//         LOG_DBG("force_kick_forward %d", m_kicker->force_kick_forward);

//         uint8_t speedDribler = 0;

//         // if (m_kicker->dribbler_enable == 1)
//         // {
//         speedDribler = (kvlSpd & m_spdMask);
//         // }
//         m_motionControl->setDribblerSpeed(speedDribler);

//         //    if ((flags & m_wpBspMask) > 0)
//         //     {
//         //         for (int i = 0; i < 5; ++i)
//         //         {
//         gpio_pin_set_dt(redLed[1], 1);
//         //             gpio_pin_set_dt(redLed[i], 1);
//         //         }
//         //     }
//         static int linkStateBlink = 0;
//         linkStateBlink = linkStateBlink == 0 ? 1 : 0;

//         // gpio_pin_set_dt(&ledl_b, (m_cannabus->m_enableLed) ? linkStateBlink : 1);

//         if (0x00 == (m_incomeArray[m_lenDbg - 1] & 0xF0))
//         {
//             int8_t iVal = u8Toi8(m_incomeArray[2]);
//             // m_motionControl->m_speedR = float(iVal / 4) * (3.14159265358979323846/127.0);/// 2.5;// * 2;
//             if (iVal != 0)
//                 // m_motionControl->setSpeedR(200 * (std::exp((float(iVal) * std::log(19)) / (127 * std::copysign(1.0, iVal))) - 1) * std::copysign(1.0, iVal) * (3.14159265358979323846 / 180.0));
//                 // m_motionControl->setSpeedR(-100 * (std::exp((float(iVal) * std::log(19)) / (127 * std::copysign(1.0, iVal))) - 1) * std::copysign(1.0, iVal) * (3.14159265358979323846 / 180.0));
//                 m_motionControl->setSpeedR(-iVal);
//             else
//                 m_motionControl->setSpeedR(0);

//             iVal = u8Toi8(m_incomeArray[4]);
//             if (iVal != 0)
//                 m_motionControl->setSpeedX(iVal / 2); /// 2.5;// * 2;
//             else
//                 m_motionControl->setSpeedX(0);

//             iVal = u8Toi8(m_incomeArray[3]);
//             if (iVal != 0)
//                 m_motionControl->setSpeedY(iVal / 2); /// 2.5;// * 2;
//             else
//                 m_motionControl->setSpeedY(0);
//         }
//         else if (0xF0 == (m_incomeArray[m_lenDbg - 1] & 0xF0))
//         {
//             float iVal = minif_to_float3(m_incomeArray[2]);
//             // m_motionControl->m_speedR = float(iVal / 4) * (3.14159265358979323846/127.0);/// 2.5;// * 2;
//             if (iVal != 0)
//                 // m_motionControl->setSpeedR(200 * (std::exp((float(iVal) * std::log(19)) / (127 * std::copysign(1.0, iVal))) - 1) * std::copysign(1.0, iVal) * (3.14159265358979323846 / 180.0));
//                 m_motionControl->setSpeedR(-25 * (std::exp((float(iVal) * std::log(19)) / (127 * std::copysign(1.0, iVal))) - 1) * std::copysign(1.0, iVal) * (3.14159265358979323846 / 180.0));
//             else
//                 m_motionControl->setSpeedR(0);

//             iVal = minif_to_float3(m_incomeArray[4]);
//             if (iVal != 0)
//                 m_motionControl->setSpeedX(iVal); /// 2.5;// * 2;
//             else
//                 m_motionControl->setSpeedX(0);

//             iVal = minif_to_float3(m_incomeArray[3]);
//             if (iVal != 0)
//                 m_motionControl->setSpeedY(iVal); /// 2.5;// * 2;
//             else
//                 m_motionControl->setSpeedY(0);
//         }
//         LOG_INF("m_speedY %f", m_motionControl->getGoalSpeedY());
//         LOG_INF("m_speedR %f", m_motionControl->getGoalSpeedR());
//         LOG_INF("m_speedX %f", m_motionControl->getGoalSpeedX());
//         // m_motionControl->curr_w = 0;
//         m_motionControl->Integrator = 0;
//         m_motionControl->beep = (flags & m_wpBspMask) > 0 ? 1 : 0;
//         // m_motionControl
//         // m_incomeArr
//         // free(m_incomeArray);
//     }
//     else
//     {
//         gpio_pin_set_dt(redLed[1], 0);
//         // if (k_uptime_get() - timeout_nrf_timer_recv > 1000)
//         // {
//         //     m_motionControl->setSpeedX(0);
//         //     m_motionControl->setSpeedY(0);
//         //     m_motionControl->setSpeedR(0);
//         // }
//     }
//     k_sleep(K_MSEC(1));
//     return 0;
//     // }
//     // a++;
// }

// void Nrf24Recv::send(uint8_t checker, uint8_t id)
// {
//     static uint8_t default_addr[]{2, checker, id};
//     // uint8_t send_tmp = checker + 0xF0;
//     rawWrite(NRF24_CMD_W_TX_PAYLOAD, default_addr, 3);
//     // arra[0] = 0x05;
//     // writePayload(default_addr, 5);
//     // return 1;
// }

// Nrf24Recv::Nrf24Recv(const struct spi_dt_spec *_spec, motion::Control *_motionControl, kicker::Kicker *_kicker, canbroadcast::CanBroadcast *_cannabus, const struct gpio_dt_spec *_chip_enable_pin, bool send_or_recieve) //, LedDisplay *_display)//, uint8_t *barrier)
// {
//     // bool flag_nrf = false;
//     // gpio_pin_set_dt(&cs_pin.gpio, flag_nrf);
//     // gpio_pin_set_dt(&cs_pin_recv.gpio, !flag_nrf);
//     // if (_display != NULL)
//     // {
//     //     m_display = _display;
//     m_chip_enable_pin = _chip_enable_pin;
//     // }

//     m_motionControl = _motionControl;

//     m_kicker = _kicker;

//     m_cannabus = _cannabus;

//     // m_barrier = barrier;

//     gpio_pin_configure_dt(m_chip_enable_pin, GPIO_OUTPUT_ACTIVE);
//     gpio_pin_configure_dt(m_chip_enable_pin, GPIO_OUTPUT_ACTIVE);
//     gpio_pin_configure_dt(&_spec->config.cs->gpio, GPIO_OUTPUT_ACTIVE);
//     // gpio_pin_configure_dt(_spec->config.cs, GPIO_OUTPUT_ACTIVE);

// #ifdef CONFIG_BOARD_stm32f429zi
//     for (int i = 0; i < 5; i++)
//     {
//         gpio_pin_configure_dt(redLed[i], GPIO_OUTPUT_ACTIVE);
//         gpio_pin_configure_dt(blueLed[i], GPIO_OUTPUT_ACTIVE);
//     }
// #endif
//     gpio_pin_configure_dt(&ledl_b, GPIO_OUTPUT_ACTIVE);

//     gpio_pin_set_dt(m_chip_enable_pin, 1);
//     k_sleep(K_MSEC(5));
//     gpio_pin_set_dt(m_chip_enable_pin, 0);

//     spec = _spec;

//     if (!spi_is_ready(spec))
//     {
//         LOG_ERR("NRF SPI not ready, aborting test");
//         return;
//     }

//     // init();
//     auto w = [this](uint8_t reg, uint8_t val)
//     { writeReg(reg, val); };
//     auto r = [this](uint8_t reg)
//     {
//         uint8_t data = 2;
//         readReg(reg, &data);
//         return data;
//     };

//     auto fail = []
//     {while(false); };
//     w(NRF24_REG_CONFIG, 0x0E);
//     uint8_t a = r(NRF24_REG_CONFIG);
//     if (a != 0x0E)
//     {
//         volatile int a = 0;
//         a++;
//         fail();
//     }

//     w(NRF24_REG_CONFIG, 0x0C);
//     // w(NRF24_REG_CONFIG, 0x0D);
//     w(NRF24_REG_RF_SETUP, 0x06);
//     w(NRF24_REG_EN_RXADDR, 0x00);
//     w(NRF24_REG_DYNPD, 0x3f);
//     w(NRF24_REG_EN_AA, 0x3f);
//     w(NRF24_REG_FEATURE, 0x05);
//     w(NRF24_REG_SETUP_RETR, 0x53);
//     // w(NRF24_REG_SETUP_RETR, 0x4C);

//     uint8_t default_addr_0[]{0xe7, 0xe7, 0xe7, 0xe7, 0xe7};
//     rawWrite(0x0A, default_addr_0, 5);

//     w(NRF24_REG_RX_PW_P0, 0x00);

//     uint8_t default_addr_1[]{0xc2, 0xc2, 0xc2, 0xc2, 0xc2};
//     rawWrite(0x0B, default_addr_1, 5);

//     w(NRF24_REG_RX_PW_P1, 0x00);
//     w(NRF24_REG_RX_ADDR_P2, 0xC3);
//     w(NRF24_REG_RX_PW_P2, 0x00);
//     w(NRF24_REG_RX_ADDR_P3, 0xC4);
//     w(NRF24_REG_RX_PW_P3, 0x00);
//     w(NRF24_REG_RX_ADDR_P4, 0xC5);
//     w(NRF24_REG_RX_PW_P4, 0x00);
//     w(NRF24_REG_RX_ADDR_P5, 0xC6);
//     w(NRF24_REG_RX_PW_P5, 0x00);

//     rawWrite(0x10, default_addr_0, 5);

//     w(0x03, 0x03);

//     if (send_or_recieve)
//         w(0x05, 0x4C); // channel set
//     else
//         w(0x05, 0x34); // channel set
//     w(0x00, 0x0D);
//     flushRx();
//     w(0x00, 0x0C);
//     flushTx();
//     w(0x07, 0x70);

//     k_sleep(K_MSEC(10));
//     w(0x06, 0x0E);

//     if (r(0x00) != 0x0C)
//     {
//         volatile int a = 0;
//         a++;
//         fail();
//     }
//     w(0x03, 0x01);
//     w(0x06, 0x0E);
//     // w(0x01, 0x40);
//     w(0x01, 0x00);

//     if (r(0x03) != 0x01)
//     {
//         volatile int a = 0;
//         a++;
//         fail();
//     }

//     uint8_t self_addr[]{0xAB, 0xAD, 0xAF};
//     // rawWrite(0x0A, self_addr, 3);
//     // uint8_t self_addr[]{0xAF, 0xAD, 0xAB};
//     if (send_or_recieve)
//         rawWrite(0x10, self_addr, 3);
//     else
//         rawWrite(0x0A, self_addr, 3);

//     if (r(0x02) != 0x00)
//     {
//         volatile int a = 0;
//         a++;
//         fail();
//     }
//     if (send_or_recieve)
//         w(0x02, 0x06);
//     else
//         w(0x02, 0x01);
//     w(0x11, 0x20);

//     rawWrite(0x0A, self_addr, 3);
//     if (send_or_recieve)
//         w(0x00, 0x0E);
//     else
//         w(0x00, 0x0F);

//     flushRx();

//     w(0x07, 0x40);

//     setCe();
//     k_sleep(K_MSEC(1));
//     resetCe();
//     k_sleep(K_MSEC(1));
//     setCe();
//     k_sleep(K_MSEC(10));
//     // flushTx();
//     // k_thread_create(&thread, thread_stack, K_THREAD_STACK_SIZEOF(thread_stack),
//     //                 (k_thread_entry_t)Nrf24Recv::work,
//     //                 NULL, NULL, NULL,
//     //                 K_PRIO_COOP(2),
//     //                 0, K_NO_WAIT);
//     // k_thread_name_set(&thread, "nrf24_thread");
//     gpio_pin_set_dt(&_spec->config.cs->gpio, true);
//     k_sleep(K_MSEC(10));
// }

// void Nrf24Recv::init()
// {
//     writeReg(NRF24_REG_CONFIG, 0x08);
//     writeReg(NRF24_REG_EN_AA, 0x3F);
//     writeReg(NRF24_REG_EN_RXADDR, 0x03);
//     writeReg(NRF24_REG_SETUP_AW, 0x03);
//     writeReg(NRF24_REG_SETUP_RETR, 0x03);
//     writeReg(NRF24_REG_RF_CH, 0x02);
//     writeReg(NRF24_REG_RF_SETUP, 0x0E);
//     writeReg(NRF24_REG_STATUS, 0x00);
//     writeReg(NRF24_REG_RX_PW_P0, 0x00);
//     writeReg(NRF24_REG_RX_PW_P1, 0x00);
//     writeReg(NRF24_REG_RX_PW_P2, 0x00);
//     writeReg(NRF24_REG_RX_PW_P3, 0x00);
//     writeReg(NRF24_REG_RX_PW_P4, 0x00);
//     writeReg(NRF24_REG_RX_PW_P5, 0x00);
//     writeReg(NRF24_REG_DYNPD, 0x00);
//     writeReg(NRF24_REG_FEATURE, 0x00);

//     // очистить FIFO
//     flushRx();
//     flushTx();

//     // очистить флаги прерываний
//     clearIrqFlags();
// }

// int8_t Nrf24Recv::u8Toi8(uint8_t x)
// {
//     return ((x >= (1 << 7)) ? -(255 - x + 1) : x);
// }

// /**************************************************************************************************
// Описание:  - дебаговая проверка наличия nrf в шине.
// Аргументы: -
// Возврат:   -
// Замечания: - записывает в регист адреса заданный массив, и так и оставляет!
// **************************************************************************************************/
// bool Nrf24Recv::isConnected(void)
// {
//     uint8_t rxBuf[NRF24_TEST_ADDR_LEN];
//     uint8_t arrToCheck[NRF24_TEST_ADDR_LEN] = NRF24_TEST_ADDR;
//     volatile int err = 0;
//     err = rawWrite(NRF24_REG_TX_ADDR, arrToCheck, NRF24_TEST_ADDR_LEN);
//     err += rawRead(NRF24_REG_TX_ADDR, rxBuf, NRF24_TEST_ADDR_LEN);
//     if (err < 0)
//     {
//         return false;
//     }
//     for (uint8_t i = 0; i < NRF24_TEST_ADDR_LEN; i++)
//     {
//         if (rxBuf[i] != arrToCheck[i])
//             return false;
//     }
//     return true;
// }

// /**************************************************************************************************
// Описание:  - включение/отключение модуля nrf.
// Аргументы: - mode - новое состояние для модуля.
// Возврат:   -
// Замечания: -
// **************************************************************************************************/
// void Nrf24Recv::setPowerMode(Nrf24Power mode)
// {
//     uint8_t reg;
//     readReg(NRF24_REG_CONFIG, &reg);
//     switch (mode)
//     {
//     case Nrf24Power::NRF24_PWR_UP:
//         reg |= NRF24_CONFIG_PWR_UP;
//         break;
//     case Nrf24Power::NRF24_PWR_DOWN:
//         reg &= ~NRF24_CONFIG_PWR_UP;
//         break;
//     default:
//         LOG_ERR("wrong power mode.");
//         return;
//     }
//     writeReg(NRF24_REG_CONFIG, reg);
// }

// /**************************************************************************************************
// Описание:  - установка режима работы модуля: TX/RX.
// Аргументы: - mode - режим работы.
// Возврат:   -
// Замечания: -
// **************************************************************************************************/
// void Nrf24Recv::setOperationMode(Nrf24OperationMode mode)
// {
//     uint8_t reg;
//     readReg(NRF24_REG_CONFIG, &reg);
//     switch (mode)
//     {
//     case Nrf24OperationMode::NRF24_Operation_PRX:
//         reg |= NRF24_CONFIG_PRIM_RX;
//         break;
//     case Nrf24OperationMode::NRF24_Operation_PTX:
//         reg &= ~NRF24_CONFIG_PRIM_RX;
//         break;
//     default:
//         LOG_ERR("wrong operation mode.");
//         return;
//     }
//     writeReg(NRF24_REG_CONFIG, reg);
// }

// /**************************************************************************************************
// Описание:  - установка режима аппаратного crc.
// Аргументы: - scheme - режим работы.
// Возврат:   -
// Замечания: -
// **************************************************************************************************/
// void Nrf24Recv::setCrcScheme(Nrf24Crc scheme)
// {
//     uint8_t reg;
//     readReg(NRF24_REG_CONFIG, &reg);
//     reg &= ~NRF24_MASK_CRC;

//     switch (scheme)
//     {
//     case Nrf24Crc::NRF24_CRC_OFF:
//         reg |= scheme;
//         break;
//     case Nrf24Crc::NRF24_CRC_1_BYTE:
//         reg |= scheme;
//         break;
//     case Nrf24Crc::NRF24_CRC_2_BYTE:
//         reg |= scheme;
//         break;
//     default:
//         LOG_ERR("wrong crc scheme.");
//         return;
//     }
//     writeReg(NRF24_REG_CONFIG, reg);
// }

// /**************************************************************************************************
// Описание:  - установка заданного канала.
// Аргументы: - channel - заданный канал (от 0 до 127).
// Возврат:   -
// Замечания: - рабочая частота = (2400 + channel)GHz.
//             - запись в регистр RF_CH обнуляет биты [7:4] регистра OBSERVE_TX (PLOS_CNT).
// **************************************************************************************************/
// void Nrf24Recv::setRfChannel(uint8_t channel)
// {
//     writeReg(NRF24_REG_RF_CH, channel);
// }

// /**************************************************************************************************
// Описание:  - установка периодов и количества попыток отправки пакета (регистр SETUP_RETR).
// Аргументы: - dealay - временной период между окончанием отправки и началом следующей отправки, микросекунды. @Nrf24SetupRetry.
//             - count - количество попыток до дерганья ногой.
// Возврат:   -
// Замечания: -
// **************************************************************************************************/
// void Nrf24Recv::setAutoRetr(Nrf24SetupRetry delay, uint8_t count)
// {
//     writeReg(NRF24_REG_SETUP_RETR, (uint8_t)((delay << 4) | (delay & NRF24_MASK_RETR_ARC)));
// }

// /**************************************************************************************************
// Описание:  - установка размера адреса.
// Аргументы: - addrWidth - размер адреса. @Nrf24SetupAddressWidth
// Возврат:   -
// Замечания: -
// **************************************************************************************************/
// void Nrf24Recv::setAddrWidth(Nrf24SetupAddressWidth addrWidth)
// {
//     writeReg(NRF24_REG_SETUP_AW, addrWidth);
// }

// // Set static RX address for a specified pipe
// // input:
// //   pipe - pipe to configure address, one of nRF24_PIPEx values
// //   addr - pointer to the buffer with address
// // note: pipe can be a number from 0 to 5 (RX pipes) and 6 (TX pipe)
// // note: buffer length must be equal to current address width of transceiver
// // note: for pipes[2..5] only first byte of address will be written because
// //       other bytes of address equals to pipe1
// // note: for pipes[2..5] only first byte of address will be written because
// //       pipes 1-5 share the four most significant address bytes
// void Nrf24Recv::setAddr(Nrf24RxpipeAddresses pipe, const uint8_t *addr)
// {
//     uint8_t addrWidth;
//     // RX_ADDR_Px register
//     switch (pipe)
//     {
//     case NRF24_PIPETX:
//     case NRF24_PIPE0:
//     case NRF24_PIPE1:
//         // Get address width
//         readReg(NRF24_REG_SETUP_AW, &addrWidth);
//         addrWidth++;
//         // Write address in reverse order (LSByte first)
//         addr += addrWidth;
//         // sendData(NRF24_CMD_W_REGISTER | NRF24_ADDR_REGS[pipe]);
//         // do
//         // {
//         //     sendData(*addr--);
//         // }
//         // while (addrWidth--);
//         break;
//     case NRF24_PIPE2:
//     case NRF24_PIPE3:
//     case NRF24_PIPE4:
//     case NRF24_PIPE5:
//         // Write address LSBbyte (only first byte from the addr buffer)
//         writeReg(NRF24_ADDR_REGS[pipe], *addr);
//         break;
//     default:
//         LOG_ERR("wrong pipe.");
//         break;
//     }
// }

// /**************************************************************************************************
// Описание:  - установка выходной мощности передатчика.
// Аргументы: - txPwr - мощность. @Nrf24RfPower
// Возврат:   -
// Замечания: -
// **************************************************************************************************/
// void Nrf24Recv::setTxPower(Nrf24RfPower txPwr)
// {
//     uint8_t reg;
//     readReg(NRF24_REG_RF_SETUP, &reg);
//     reg &= ~NRF24_MASK_RF_PWR;
//     reg |= txPwr;
//     writeReg(NRF24_REG_RF_SETUP, reg);
// }

// /**************************************************************************************************
// Описание:  - установка скорости передачи данных.
// Аргументы: - dataRate - скорость передачи. @Nrf24DataRate
// Возврат:   -
// Замечания: -
// **************************************************************************************************/
// void Nrf24Recv::setDataRate(Nrf24DataRate dataRate)
// {
//     uint8_t reg;
//     readReg(NRF24_REG_RF_SETUP, &reg);
//     reg &= ~NRF24_MASK_DATARATE;
//     reg |= dataRate;
//     writeReg(NRF24_REG_RF_SETUP, reg);
// }

// // Configure a specified RX pipe
// // input:
// //   pipe - number of the RX pipe, value from 0 to 5
// //   aa_state - state of auto acknowledgment, one of nRF24_AA_xx values
// //   payload_len - payload length in bytes
// void Nrf24Recv::setRxPipe(Nrf24RxpipeAddresses pipe, uint8_t aaState, uint8_t payloadLen)
// {
//     uint8_t reg;
//     // Enable the specified pipe (EN_RXADDR register)
//     readReg((NRF24_REG_EN_RXADDR) | (1 << pipe), &reg);
//     reg = reg & NRF24_MASK_EN_RX;
//     writeReg(NRF24_REG_EN_RXADDR, reg);

//     // Set RX payload length (RX_PW_Px register)
//     writeReg(NRF24_RX_PW_PIPE[pipe], payloadLen & NRF24_MASK_RX_PW);

//     // Set auto acknowledgment for a specified pipe (EN_AA register)
//     readReg(NRF24_REG_EN_AA, &reg);
//     if (aaState == NRF24_AA_ON)
//         reg |= (1 << pipe);
//     else
//         reg &= ~(1 << pipe);
//     writeReg(NRF24_REG_EN_AA, reg);
// }

// /**************************************************************************************************
// Описание:  - закрыть соединение на заданной линии.
// Аргументы: - pipe - заданная линия. @Nrf24RxpipeAddresses
// Возврат:   -
// Замечания: -
// **************************************************************************************************/
// void Nrf24Recv::closePipe(Nrf24RxpipeAddresses pipe)
// {
//     uint8_t reg;
//     readReg(NRF24_REG_EN_RXADDR, &reg);
//     reg &= ~(1 << pipe);
//     reg &= NRF24_MASK_EN_RX;
//     writeReg(NRF24_REG_EN_RXADDR, reg);
// }

// /**************************************************************************************************
// Описание:  - включить аппаратное подтверждение доставки на заданной линии.
// Аргументы: - pipe - заданная линия. @Nrf24RxpipeAddresses
// Возврат:   -
// Замечания: -
// **************************************************************************************************/
// void Nrf24Recv::enableAa(Nrf24RxpipeAddresses pipe)
// {
//     uint8_t reg;
//     readReg(NRF24_REG_EN_AA, &reg);
//     reg |= (1 << pipe);
//     writeReg(NRF24_REG_EN_AA, reg);
// }

// /**************************************************************************************************
// Описание:  - отключить аппаратное подтверждение доставки на заданной линии.
// Аргументы: - pipe - заданная линия. @Nrf24RxpipeAddresses
// Возврат:   -
// Замечания: -
// **************************************************************************************************/
// void Nrf24Recv::disableAa(Nrf24RxpipeAddresses pipe)
// {
//     uint8_t reg;
//     readReg(NRF24_REG_EN_AA, &reg);
//     reg &= ~(1 << pipe);
//     writeReg(NRF24_REG_EN_AA, reg);
// }

// /**************************************************************************************************
// Описание:  - возвращает значение регистра STATUS.
// Аргументы: -
// Возврат:   - значение регистра STATUS.
// Замечания: -
// **************************************************************************************************/
// uint8_t Nrf24Recv::getStatus(void)
// {
//     uint8_t reg;
//     readReg(NRF24_REG_STATUS, &reg);
//     return reg;
// }

// /**************************************************************************************************
// Описание:  - возвращает значение сработавшего прерывания.
// Аргументы: -
// Возврат:   - прерывание. @Nrf24IrqStatus
// Замечания: -
// **************************************************************************************************/
// Nrf24IrqStatus Nrf24Recv::getIrqStatus(void)
// {
//     uint8_t status;
//     readReg(NRF24_REG_STATUS, &status);
//     if (status & Nrf24IrqStatus::NRF24_DATA_RECEIVE_IRQ)
//         return Nrf24IrqStatus::NRF24_DATA_RECEIVE_IRQ;
//     if (status & Nrf24IrqStatus::NRF24_DATA_SENDED_IRQ)
//         return Nrf24IrqStatus::NRF24_DATA_SENDED_IRQ;
//     if (status & Nrf24IrqStatus::NRF24_MAX_RETRANSMITS_IRQ)
//         return Nrf24IrqStatus::NRF24_MAX_RETRANSMITS_IRQ;
//     return Nrf24IrqStatus::NRF24_NO_IRQ;
// }

// /**************************************************************************************************
// Описание:  - возвращает состояние приёмного буффера.
// Аргументы: -
// Возврат:   - состояние приёмного буффера.
// Замечания: -
// **************************************************************************************************/
// uint8_t Nrf24Recv::getStatusRxFIFO(void)
// {
//     uint8_t reg;
//     readReg(NRF24_REG_FIFO_STATUS, &reg);
//     return (reg & NRF24_MASK_RXFIFO);
// }

// /**************************************************************************************************
// Описание:  - возвращает состояние буффера отправки.
// Аргументы: -
// Возврат:   - состояние буффера отправки.
// Замечания: -
// **************************************************************************************************/
// uint8_t Nrf24Recv::getStatusTxFIFO(void)
// {
//     uint8_t reg;
//     readReg(NRF24_REG_FIFO_STATUS, &reg);
//     return ((reg & NRF24_MASK_TXFIFO) >> 4);
// }

// // Get pipe number for the payload available for reading from RX FIFO
// // return: pipe number or 0x07 if the RX FIFO is empty
// uint8_t Nrf24Recv::getRxSource(void)
// {
//     uint8_t reg;
//     readReg(NRF24_REG_STATUS, &reg);
//     return ((reg & NRF24_MASK_RX_P_NO) >> 1);
// }

// // Get auto retransmit statistic
// // return: value of OBSERVE_TX register which contains two counters encoded in nibbles:
// //   high - lost packets count (max value 15, can be reseted by write to RF_CH register)
// //   low  - retransmitted packets count (max value 15, reseted when new transmission starts)
// uint8_t Nrf24Recv::getRetransmitCounters(void)
// {
//     uint8_t reg;
//     readReg(NRF24_REG_OBSERVE_TX, &reg);
//     return reg;
// }

// // Reset packet lost counter (PLOS_CNT bits in OBSERVER_TX register)
// void Nrf24Recv::resetPLOS(void)
// {
//     uint8_t reg;

//     // The PLOS counter is reset after write to RF_CH register
//     readReg(NRF24_REG_RF_CH, &reg);
//     writeReg(NRF24_REG_RF_CH, reg);
// }

// // Flush the TX FIFO
// void Nrf24Recv::flushTx(void)
// {
//     writeReg(NRF24_CMD_FLUSH_TX, NRF24_CMD_NOP);
// }

// // Flush the RX FIFO
// void Nrf24Recv::flushRx(void)
// {
//     writeReg(NRF24_CMD_FLUSH_RX, NRF24_CMD_NOP);
// }

// // Clear any pending IRQ flags
// void Nrf24Recv::clearIrqFlags(void)
// {
//     uint8_t reg;
//     // Clear RX_DR, TX_DS and MAX_RT bits of the STATUS register
//     readReg(NRF24_REG_STATUS, &reg);
//     reg |= NRF24_MASK_STATUS_IRQ;
//     writeReg(NRF24_REG_STATUS, reg);
// }

// // Write TX payload
// // input:
// //   pBuf - pointer to the buffer with payload data
// //   length - payload length in bytes
// void Nrf24Recv::writePayload(uint8_t *pBuf, uint8_t length)
// {
//     rawWrite(NRF24_CMD_W_TX_PAYLOAD, pBuf, length);
// }

// // Read top level payload available in the RX FIFO
// // input:
// //   pBuf - pointer to the buffer to store a payload data
// //   length - pointer to variable to store a payload length
// // return: one of nRF24_RX_xx values
// //   nRF24_RX_PIPEX - packet has been received from the pipe number X
// //   nRF24_RX_EMPTY - the RX FIFO is empty
// Nrf24RxResult Nrf24Recv::readPayload(uint8_t *pBuf, uint8_t *length)
// {
//     uint8_t pipe;

//     // Extract a payload pipe number from the STATUS register
//     readReg(NRF24_REG_STATUS, &pipe);
//     pipe = (pipe & NRF24_MASK_RX_P_NO) >> 1;

//     // RX FIFO empty?
//     if (pipe < 6)
//     {
//         // Get payload length
//         readReg(NRF24_RX_PW_PIPE[pipe], length);

//         // Read a payload from the RX FIFO
//         if (*length)
//         {
//             rawRead(NRF24_CMD_R_RX_PAYLOAD, pBuf, *length);
//         }
//         return ((Nrf24RxResult)pipe);
//     }

//     // The RX FIFO is empty
//     *length = 0;

//     return NRF24_RX_EMPTY;
// }

// void Nrf24Recv::toggleFeatures(void)
// {
//     //   uint8_t dt = ACTIVATE;
//     //   CS_ON();
//     //   sendData(dt);
//     //   dt = 0x73;
//     //   sendData(dt);
//     //   CS_OFF();
// }
