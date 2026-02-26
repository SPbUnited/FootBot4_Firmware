#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "ssd1306_tests.h"
// #include ""
#include "Error.h"
#include "stm32f4xx_hal.h"

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
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    // RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    // RCC_OscInitStruct.PLL.PLLM = 8;          // Делитель HSE (8 МГц / 8 = 1 МГц)
    // RCC_OscInitStruct.PLL.PLLN = 360;        // Множитель VCO (1 МГц * 360 = 360 МГц)
    // RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // Делитель PLLP (360 / 2 = 180 МГц)
    // RCC_OscInitStruct.PLL.PLLQ = 7;          // Для USB/SDIO, не обязательно
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        Error_Handler();

    // Настройка шин AHB, APB
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // SYSCLK = PLL (180 МГц)
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;        // HCLK = 180 МГц
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;         // APB1 = 45 МГц
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;         // APB2 = 90 МГц

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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

    // LEDMGR::init_leds();
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

    ssd1306_Fill(White);
    ssd1306_SetCursor(5, 10);
    ssd1306_WriteString("Hello world!", Font_11x18, White);
    ssd1306_UpdateScreen();
    while (1)
    {
        // for (size_t i = 0; i < LEDMGR::NUM_LEDS; i++)
        // {
        //     LEDMGR::toggle_led(LEDMGR::Leds(i));
        //     HAL_Delay(100);
        // }

        // buzz();
    }
}
