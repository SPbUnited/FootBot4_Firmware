#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "ssd1306_tests.h"
// #include ""
#include "Error.h"
#include "stm32f4xx_hal.h"
#include "Usart4.h"
#include "LedManager.h"
// #include "ScreenManager.h"

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
    RCC_OscInitStruct.PLL.PLLM = 8;          // Делитель HSE (8 МГц / 8 = 1 МГц)
    RCC_OscInitStruct.PLL.PLLN = 360;        // Множитель VCO (1 МГц * 360 = 360 МГц)
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // Делитель PLLP (360 / 2 = 180 МГц)
    RCC_OscInitStruct.PLL.PLLQ = 7;          // Для USB/SDIO, не обязательно
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        Error_Handler();

    // Настройка шин AHB, APB
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // SYSCLK = PLL (180 МГц)
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;        // HCLK = 180 МГц
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;         // APB1 = 45 МГц
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;         // APB2 = 90 МГц

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
        Error_Handler();

    // Включение тактирования необходимых периферийных модулей
    __HAL_RCC_GPIOF_CLK_ENABLE();   // для PF0, PF1
    __HAL_RCC_I2C2_CLK_ENABLE();    // для I2C2
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    turn_on_robot();

    HAL_Delay(200);

    LEDMGR::init_leds();
    initUsart4();
    // SCREENMGR::init_screen();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_I2C2_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD; // Открытый коллектор
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    hi2c2.Instance = I2C2;
    hi2c2.Init.ClockSpeed = 100000;
    hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c2.Init.OwnAddress1 = 0;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c2) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_Delay(200);

    // init_buzzer();
    ssd1306_Init();
            // ssd1306_TestAll();

    ssd1306_Fill(Black);
    ssd1306_SetCursor(5, 10);
    ssd1306_WriteString("Hello world!", Font_11x18, White);
    ssd1306_UpdateScreen();
    while (1)
    {
        for (size_t i = 0; i < LEDMGR::NUM_LEDS; i++)
        {
            LEDMGR::toggle_led(LEDMGR::Leds(i));
            Uart4OutputChar('S');
            Uart4OutputChar('S');
            Uart4OutputChar('L');
            Uart4OutputChar('\n');
            HAL_Delay(100);
            
        }

        // buzz();
    }
}


// #include <stm32f4xx_hal.h>
// #include <string.h>

// // ==================== Настройки дисплея ====================
// #define SSD1306_ADDR    0x3C            // 7-битный адрес (0x3C или 0x3D)
// #define SSD1306_WIDTH   128
// #define SSD1306_HEIGHT  32

// // Буфер кадра (512 байт)
// static uint8_t framebuffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// // ==================== Прототипы функций ====================
// static void SystemClock_Config(void);
// static void MX_GPIO_Init(void);
// static void MX_I2C2_Init(void);

// static void SSD1306_WriteCmd(uint8_t cmd);
// static void SSD1306_WriteData(uint8_t* data, uint16_t size);
// static void SSD1306_SetCursor(uint8_t page, uint8_t column);
// void SSD1306_Init(void);
// void SSD1306_UpdateScreen(void);
// void SSD1306_Clear(void);
// void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
// void SSD1306_DrawChar(char ch, uint8_t x, uint8_t y);
// void SSD1306_DrawString(const char* str, uint8_t x, uint8_t y);

// // Глобальный дескриптор I2C
// I2C_HandleTypeDef hi2c2;

// // ==================== Обработчик ошибок ====================
// void Error_Handler(void)
// {
//     while (1) {
//         // Можно зажечь светодиод или ничего не делать
//     }
// }

// // ==================== Обработчики прерываний (weak, но могут быть переопределены) ====================
// extern "C" {
//     void SysTick_Handler(void)
//     {
//         HAL_IncTick();
//     }
// }

// // ==================== Основная программа ====================
// int main(void)
// {
//     HAL_Init();
//     SystemClock_Config();
//     MX_GPIO_Init();
//     MX_I2C2_Init();

//     SSD1306_Init();

//     SSD1306_Clear();
//     SSD1306_DrawString("Hello PlatformIO!", 10, 10);
//     SSD1306_DrawString("I2C2 on PF0 PF1", 10, 20);
//     SSD1306_UpdateScreen();

//     while (1)
//     {
//         // Здесь можно добавить основной код
//     }
// }

// // ==================== Инициализация тактирования ====================
// void SystemClock_Config(void)
// {
//     RCC_OscInitTypeDef RCC_OscInitStruct = {0};
//     RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

//     // Включение внешнего кварца HSE (предполагаем 8 МГц)
//     RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
//     RCC_OscInitStruct.HSEState = RCC_HSE_ON;
//     RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
//     RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
//     RCC_OscInitStruct.PLL.PLLM = 8;          // Делитель HSE (8 МГц / 8 = 1 МГц)
//     RCC_OscInitStruct.PLL.PLLN = 360;        // Множитель VCO (1 МГц * 360 = 360 МГц)
//     RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // Делитель PLLP (360 / 2 = 180 МГц)
//     RCC_OscInitStruct.PLL.PLLQ = 7;          // Для USB/SDIO, не обязательно
//     if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//         Error_Handler();

//     // Настройка шин AHB, APB
//     RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
//                                 | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
//     RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // SYSCLK = PLL (180 МГц)
//     RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;        // HCLK = 180 МГц
//     RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;         // APB1 = 45 МГц
//     RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;         // APB2 = 90 МГц

//     if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
//         Error_Handler();

//     // Включение тактирования необходимых периферийных модулей
//     __HAL_RCC_GPIOF_CLK_ENABLE();   // для PF0, PF1
//     __HAL_RCC_I2C2_CLK_ENABLE();    // для I2C2
// }

// // ==================== Инициализация GPIO (PF0, PF1 как I2C2) ====================
// void MX_GPIO_Init(void)
// {
//     GPIO_InitTypeDef GPIO_InitStruct = {0};

//     // PF0 и PF1 работают как I2C2 (альтернативная функция AF4)
//     GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
//     GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;       // открытый сток
//     GPIO_InitStruct.Pull = GPIO_PULLUP;
//     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//     GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;    // альтернативная функция для I2C2
//     HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
// }

// // ==================== Инициализация I2C2 ====================
// void MX_I2C2_Init(void)
// {
//     hi2c2.Instance = I2C2;
//     hi2c2.Init.ClockSpeed = 100000;          // 100 кГц (можно 400 кГц)
//     hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
//     hi2c2.Init.OwnAddress1 = 0;               // адрес slave (не используется)
//     hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
//     hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
//     hi2c2.Init.OwnAddress2 = 0;
//     hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
//     hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

//     if (HAL_I2C_Init(&hi2c2) != HAL_OK)
//         Error_Handler();
// }

// // ==================== Функции работы с SSD1306 ====================

// // Отправка команды
// static void SSD1306_WriteCmd(uint8_t cmd)
// {
//     HAL_I2C_Mem_Write(&hi2c2, SSD1306_ADDR << 1, 0x00, I2C_MEMADD_SIZE_8BIT, &cmd, 1, HAL_MAX_DELAY);
// }

// // Отправка данных
// static void SSD1306_WriteData(uint8_t* data, uint16_t size)
// {
//     HAL_I2C_Mem_Write(&hi2c2, SSD1306_ADDR << 1, 0x40, I2C_MEMADD_SIZE_8BIT, data, size, HAL_MAX_DELAY);
// }

// // Установка курсора (страница/колонка)
// static void SSD1306_SetCursor(uint8_t page, uint8_t column)
// {
//     SSD1306_WriteCmd(0xB0 + page);           // номер страницы
//     SSD1306_WriteCmd(column & 0x0F);          // младшие 4 бита колонки
//     SSD1306_WriteCmd(((column >> 4) & 0x0F) | 0x10); // старшие 4 бита
// }

// // Инициализация дисплея
// void SSD1306_Init(void)
// {
//     HAL_Delay(100); // ждём стабилизации питания

//     SSD1306_WriteCmd(0xAE); // выкл.

//     SSD1306_WriteCmd(0xD5); SSD1306_WriteCmd(0x80); // тактирование
//     SSD1306_WriteCmd(0xA8); SSD1306_WriteCmd(0x1F); // мультиплексор (32 строки)
//     SSD1306_WriteCmd(0xD3); SSD1306_WriteCmd(0x00); // смещение
//     SSD1306_WriteCmd(0x40);                         // начальная линия
//     SSD1306_WriteCmd(0x8D); SSD1306_WriteCmd(0x14); // вкл. charge pump
//     SSD1306_WriteCmd(0x20); SSD1306_WriteCmd(0x00); // горизонтальная адресация
//     SSD1306_WriteCmd(0xA1);                         // сегментное отображение (зеркально)
//     SSD1306_WriteCmd(0xC8);                         // сканирование COM
//     SSD1306_WriteCmd(0xDA); SSD1306_WriteCmd(0x02); // конфигурация COM (для 32 строк)
//     SSD1306_WriteCmd(0x81); SSD1306_WriteCmd(0xCF); // контрастность
//     SSD1306_WriteCmd(0xD9); SSD1306_WriteCmd(0xF1); // предзаряд
//     SSD1306_WriteCmd(0xDB); SSD1306_WriteCmd(0x40); // VCOMH
//     SSD1306_WriteCmd(0xA4);                         // возобновить отображение RAM
//     SSD1306_WriteCmd(0xA6);                         // нормальный режим (не инвертировать)
//     SSD1306_WriteCmd(0x2E);                         // деактивировать скролл
//     SSD1306_WriteCmd(0xAF);                         // вкл.

//     SSD1306_Clear();
//     SSD1306_UpdateScreen();
// }

// // Обновление экрана (копирование буфера в дисплей)
// void SSD1306_UpdateScreen(void)
// {
//     for (uint8_t page = 0; page < SSD1306_HEIGHT / 8; page++) {
//         SSD1306_SetCursor(page, 0);
//         SSD1306_WriteData(&framebuffer[page * SSD1306_WIDTH], SSD1306_WIDTH);
//     }
// }

// // Очистка буфера
// void SSD1306_Clear(void)
// {
//     memset(framebuffer, 0, sizeof(framebuffer));
// }

// // Рисование пикселя
// void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
// {
//     if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
//     uint16_t idx = x + (y / 8) * SSD1306_WIDTH;
//     if (color)
//         framebuffer[idx] |= (1 << (y % 8));
//     else
//         framebuffer[idx] &= ~(1 << (y % 8));
// }

// // Простой шрифт 6x8 (только символы для примера)
// static const uint8_t font6x8[][6] = {
//     {0x00,0x00,0x00,0x00,0x00,0x00}, // ' '
//     {0x00,0x00,0x00,0x5F,0x00,0x00}, // '!'
//     {0x00,0x3E,0x51,0x49,0x45,0x3E}, // '0'
//     {0x00,0x00,0x42,0x7F,0x40,0x00}, // '1'
//     {0x00,0x42,0x61,0x51,0x49,0x46}, // '2'
//     {0x00,0x21,0x41,0x45,0x4B,0x31}, // '3'
//     {0x00,0x18,0x14,0x12,0x7F,0x10}, // '4'
//     {0x00,0x27,0x45,0x45,0x45,0x39}, // '5'
//     {0x00,0x3C,0x4A,0x49,0x49,0x30}, // '6'
//     {0x00,0x01,0x71,0x09,0x05,0x03}, // '7'
//     {0x00,0x36,0x49,0x49,0x49,0x36}, // '8'
//     {0x00,0x06,0x49,0x49,0x29,0x1E}, // '9'
// };

// // Вывод символа
// void SSD1306_DrawChar(char ch, uint8_t x, uint8_t y)
// {
//     if (ch < 0x20 || ch > 0x7F) ch = 0x20;
//     uint8_t idx = ch - 0x20;
//     for (uint8_t col = 0; col < 6; col++) {
//         uint8_t line = font6x8[idx][col];
//         for (uint8_t row = 0; row < 8; row++) {
//             if (line & (1 << row))
//                 SSD1306_DrawPixel(x + col, y + row, 1);
//         }
//     }
// }

// // Вывод строки
// void SSD1306_DrawString(const char* str, uint8_t x, uint8_t y)
// {
//     while (*str) {
//         SSD1306_DrawChar(*str++, x, y);
//         x += 6;
//         if (x > SSD1306_WIDTH - 6) break;
//     }
// }

