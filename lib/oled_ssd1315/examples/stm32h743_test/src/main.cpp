/**
 * @file main.cpp
 * @brief Тест OLED SSD1315 на STM32H743
 *
 * I2C1: PB6 (SCL), PB7 (SDA)
 * Дисплей: 128x64, адрес 0x3C
 */

#include "stm32h7xx_hal.h"
#include <oled/OledSsd1315.hpp>

// I2C handle
I2C_HandleTypeDef hi2c1;

// OLED дисплей (указатель, инициализируется после I2C)
oled::OledSsd1315* pDisplay = nullptr;

// Прототипы
static void SystemClock_Config();
static void MX_GPIO_Init();
static void MX_I2C1_Init();
static void Error_Handler();

int main() {
    // Инициализация HAL
    HAL_Init();

    // Настройка системных часов (480 MHz)
    SystemClock_Config();

    // Инициализация GPIO
    MX_GPIO_Init();

    // Инициализация I2C1 (PB6/PB7)
    MX_I2C1_Init();

    // Небольшая задержка для стабилизации питания дисплея
    HAL_Delay(100);

    // Создаём объект дисплея с I2C handle
    pDisplay = new oled::OledSsd1315(&hi2c1);

    // Конфигурация дисплея
    oled::OledConfig cfg;
    cfg.i2cAddr7 = 0x3C;      // Стандартный адрес SSD1315/SSD1306
    cfg.width = 128;
    cfg.height = 64;
    cfg.vccMode = oled::VccMode::InternalChargePump;
    cfg.flip180 = false;

    // Инициализация дисплея
    oled::OledResult result = pDisplay->begin(cfg);

    if (result != oled::OledResult::Ok) {
        // Ошибка инициализации - мигаем LED (если есть)
        Error_Handler();
    }

    // Очищаем экран
    pDisplay->clear();

    // Выводим тестовый текст
    pDisplay->setCursor(0, 0);
    pDisplay->setTextSize(1);
    pDisplay->print("STM32H743 OLED Test");

    pDisplay->setCursor(0, 16);
    pDisplay->print("I2C1: PB6/PB7");

    pDisplay->setCursor(0, 32);
    pDisplay->print("Addr: 0x3C");

    pDisplay->setCursor(0, 48);
    pDisplay->printf("Result: %d", static_cast<int>(result));

    // Отправляем на дисплей
    pDisplay->flush();

    // Ждём 2 секунды
    HAL_Delay(2000);

    // Тест графики
    pDisplay->clear();

    // Рамка
    pDisplay->rect(0, 0, 128, 64, true);

    // Диагональные линии
    pDisplay->line(0, 0, 127, 63, true);
    pDisplay->line(127, 0, 0, 63, true);

    // Текст в центре
    pDisplay->setCursor(32, 28);
    pDisplay->setTextSize(1);
    pDisplay->print("Graphics OK");

    pDisplay->flush();

    HAL_Delay(2000);

    // Тест кириллицы
    pDisplay->clear();
    pDisplay->setCursor(0, 0);
    pDisplay->setTextSize(2);
    pDisplay->print("Привет!");

    pDisplay->setCursor(0, 32);
    pDisplay->setTextSize(1);
    pDisplay->print("Тест кириллицы UTF-8");

    pDisplay->flush();

    // Бесконечный цикл с счётчиком
    uint32_t counter = 0;
    while (1) {
        pDisplay->setCursor(0, 56);
        pDisplay->setTextSize(1);
        pDisplay->rectFill(0, 56, 128, 8, false); // Очистка строки
        pDisplay->printf("Counter: %lu", counter++);
        pDisplay->flush();

        HAL_Delay(100);
    }
}

/**
 * @brief Конфигурация системных часов STM32H743
 *
 * HSE -> PLL1 -> SYSCLK = 480 MHz
 * APB1/APB2 = 120 MHz
 */
static void SystemClock_Config() {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Настройка источника питания
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    // Настройка voltage scaling
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

    // Настройка HSE и PLL
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 5;   // Делитель входной частоты
    RCC_OscInitStruct.PLL.PLLN = 192; // Множитель PLL
    RCC_OscInitStruct.PLL.PLLP = 2;   // Делитель PLLP -> SYSCLK
    RCC_OscInitStruct.PLL.PLLQ = 4;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    // Настройка системных часов
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief Инициализация GPIO
 */
static void MX_GPIO_Init() {
    // Включаем тактирование GPIO
    __HAL_RCC_GPIOB_CLK_ENABLE();
}

/**
 * @brief Инициализация I2C1
 *
 * SCL: PB6 (AF4)
 * SDA: PB7 (AF4)
 * Скорость: 400 kHz (Fast Mode)
 */
static void MX_I2C1_Init() {
    // Включаем тактирование I2C1
    __HAL_RCC_I2C1_CLK_ENABLE();

    // Настройка GPIO для I2C1
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;  // PB6, PB7
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;          // Open-drain для I2C
    GPIO_InitStruct.Pull = GPIO_PULLUP;              // Подтяжка вверх
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;       // AF4 = I2C1
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Настройка I2C1
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00C0EAFF;  // 400 kHz при 120 MHz APB1
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        Error_Handler();
    }

    // Настройка аналогового фильтра
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        Error_Handler();
    }

    // Настройка цифрового фильтра
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief Обработчик ошибок
 */
static void Error_Handler() {
    // Бесконечный цикл при ошибке
    __disable_irq();
    while (1) {
        // Можно добавить мигание LED для индикации ошибки
    }
}

// HAL требует эту функцию
extern "C" void SysTick_Handler() {
    HAL_IncTick();
}
