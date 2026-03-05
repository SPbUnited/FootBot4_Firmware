/**
 * @file main.cpp
 * @brief Пример использования OLED SSD1315 с STM32 HAL
 * 
 * Требует настройки I2C через STM32CubeMX или вручную.
 * 
 * Платформа: STM32F4 / STM32H7
 * Framework: stm32cube
 * 
 * build_flags:
 *   -DOLED_SSD1315_ENABLE=1
 *   -DOLED_PLATFORM_STM32HAL=1
 */

#include "stm32f4xx_hal.h"
#include <oled/OledSsd1315.hpp>

// Прототипы
void Error_Handler(void);

// I2C handle (инициализируется в MX_I2C1_Init)
I2C_HandleTypeDef hi2c1;

// OLED дисплей
OledSsd1315 display(&hi2c1);

// Прототипы функций инициализации
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);

/**
 * @brief Пример callback для аппаратного reset (опционально)
 * 
 * Если у вас подключён пин RST дисплея:
 * - Раскомментируйте эту функцию
 * - Добавьте cfg.resetCallback = &oledResetCallback;
 */
// void oledResetCallback(bool high) {
//     HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, 
//                       high ? GPIO_PIN_SET : GPIO_PIN_RESET);
// }

int main(void) {
    // Инициализация HAL
    HAL_Init();
    
    // Настройка системного тактирования
    SystemClock_Config();
    
    // Инициализация GPIO
    MX_GPIO_Init();
    
    // Инициализация I2C
    MX_I2C1_Init();
    
    // === Инициализация OLED ===
    OledConfig cfg;
    cfg.i2cAddr7 = 0x3C;           // 7-битный адрес
    cfg.width = 128;
    cfg.height = 64;
    cfg.vccMode = VccMode::InternalChargePump;
    cfg.flip180 = false;
    // cfg.resetCallback = &oledResetCallback; // Если есть RST пин
    
    if (display.begin(cfg) != OledResult::Ok) {
        // Ошибка инициализации - мигаем LED
        while (1) {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            HAL_Delay(100);
        }
    }
    
    // === Демонстрация возможностей ===
    
    // Очистка и вывод текста
    display.clear();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("OLED SSD1315");
    
    display.setCursor(0, 10);
    display.print("STM32 HAL Demo");
    
    display.setCursor(0, 24);
    display.setTextSize(2);
    display.print("Привет!");
    
    // Рисуем рамку
    display.rect(0, 0, 128, 64, true);
    
    // Отправляем на дисплей
    display.flush();
    
    // Главный цикл
    uint32_t counter = 0;
    while (1) {
        HAL_Delay(1000);
        
        // Обновляем счётчик
        display.rectFill(0, 48, 128, 16, false);  // Очищаем область
        display.setCursor(4, 50);
        display.setTextSize(1);
        display.printf("Uptime: %lu sec", counter++);
        display.flush();
    }
}

/**
 * @brief Инициализация I2C1
 * 
 * Настройте пины в соответствии с вашей платой:
 * - BlackPill F411: PB6 (SCL), PB7 (SDA)
 * - DevEBox H743: PB8 (SCL), PB9 (SDA)
 */
static void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 400000;           // 400 kHz
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void SystemClock_Config(void) {
    // Настройте через STM32CubeMX для вашего MCU
    // Здесь упрощённая версия для F411 @ 100MHz
    
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 200;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
        Error_Handler();
    }
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}
