// #include "ssd1306.h"
// #include "ssd1306_fonts.h"
// #include "ssd1306_tests.h"
// #include ""
#include "Error.h"
#include "stm32f4xx_hal.h"
// #include "Usart4.h"
// #include "LedManager.h"
// #include "ScreenManager.h"
#include <oled/OledSsd1315.hpp>

#include "drivers/device_manager.hpp"

using namespace oled_lib;

extern "C" void SysTick_Handler(void)
{
    HAL_IncTick();
}

I2C_HandleTypeDef hi2c2;

void turn_on_robot()
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
}

void init_buzzer()
{
    __HAL_RCC_GPIOF_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

void buzz()
{
    // HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_SET);
    // HAL_Delay(1000);
    // HAL_GPIO_WritePin(GPIOF, GPIO_PIN_13, GPIO_PIN_RESET);
    for (size_t i = 0; i < 10000; i++)
    {
        HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_13);
        // HAL_Delay(1);
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Включение внешнего кварца HSE (предполагаем 8 МГц)
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;              // Делитель HSE (8 МГц / 8 = 1 МГц)
    RCC_OscInitStruct.PLL.PLLN = 360;            // Множитель VCO (1 МГц * 360 = 360 МГц)
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;  // Делитель PLLP (360 / 2 = 180 МГц)
    RCC_OscInitStruct.PLL.PLLQ = 7;              // Для USB/SDIO, не обязательно
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        Error_Handler();

    // Настройка шин AHB, APB
    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;  // SYSCLK = PLL (180 МГц)
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;         // HCLK = 180 МГц
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;          // APB1 = 45 МГц
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;          // APB2 = 90 МГц

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
        Error_Handler();

    // Включение тактирования необходимых периферийных модулей
    __HAL_RCC_GPIOF_CLK_ENABLE();  // для PF0, PF1
    __HAL_RCC_I2C2_CLK_ENABLE();   // для I2C2
}

#define display drivers::oled_drv.display

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    turn_on_robot();

    HAL_Delay(200);

    // LEDMGR::init_leds();

    drivers::uart4.init();
    drivers::i2c2.init();
    drivers::leds.init();
    drivers::oled_drv.init();

    HAL_Delay(200);

    // init_buzzer();
    // ssd1306_Init();
    // OledSsd1315 display(&hi2c2);
    // OledSsd1315 display(&i2c2.handle);
    // ssd1306_TestAll();

    // === Инициализация OLED ===
    // OledConfig cfg;
    // cfg.i2cAddr7 = 0x3C; // 7-битный адрес
    // cfg.width = 128;
    // cfg.height = 32;
    // cfg.vccMode = VccMode::InternalChargePump;
    // cfg.flip180 = false;
    // // cfg.resetCallback = &oledResetCallback; // Если есть RST пин

    // if (display.begin(cfg) != OledResult::Ok)
    // {
    //     // Ошибка инициализации - мигаем LED
    //     while (1)
    //     {
    //         // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
    //         uart4.printf("OLED init error\n");
    //         HAL_Delay(100);
    //     }
    // }

    drivers::uart4.printf("OLED init ok\n");

    display.clear();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("OLED SSD1315");

    display.setCursor(0, 8);
    display.print("STM32 HAL Demo");

    display.setCursor(0, 16);
    display.setTextSize(1);
    display.print("Привет!");

    // Рисуем рамку
    display.rect(0, 0, 128, 32, true);

    // Отправляем на дисплей
    display.flush();

    // Главный цикл
    uint32_t counter = 0;
    while (1)
    {
        HAL_Delay(1000);

        // Обновляем счётчик
        display.rectFill(1, 24, 126, 7, false);  // Очищаем область
        display.setCursor(4, 24);
        display.setTextSize(1);
        display.printf("Uptime: %lu sec", counter);
        display.flush();
        drivers::leds.display_number(counter);
        drivers::uart4.printf("Uptime: %lu sec\n", counter);

        counter++;
    }

    // while (1)
    // {
    //     for (size_t i = 0; i < LEDMGR::NUM_LEDS; i++)
    //     {
    //         LEDMGR::toggle_led(LEDMGR::Leds(i));
    //         // Uart4OutputChar('S');
    //         // Uart4OutputChar('S');
    //         // Uart4OutputChar('L');
    //         // Uart4OutputChar('\n');

    //         uart4.putc('S');
    //         uart4.putc('S');
    //         uart4.putc('L');
    //         uart4.putc('\n');

    //         HAL_Delay(100);
    //     }

    //     // buzz();
    // }
}