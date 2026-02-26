#pragma once

#include "stm32f4xx_hal.h"
#include "Error.h"
#include <oled/OledSsd1315.hpp>
#include "LedManager.h"

namespace SCREENMGR
{
    I2C_HandleTypeDef hi2c2;
    oled::OledSsd1315 *display;

    void init_screen() // MX_I2C2_Init
    {
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_I2C2_CLK_ENABLE();
        
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD; // Открытый коллектор
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

        hi2c2.Instance = I2C2;
        hi2c2.Init.ClockSpeed = 400000;
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

        /** Configure Analogue filter
         */
        // if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
        // {
        //     Error_Handler();
        // }

        // /** Configure Digital filter
        //  */
        // if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
        // {
        //     Error_Handler();
        // }

        // display = new oled::OledSsd1315(&hi2c2);

        // oled::OledConfig cfg;
        // cfg.i2cAddr7 = 0x3C;
        // cfg.height = 32;
        // cfg.width = 128;
        // cfg.i2cFreq = 400000;

        // HAL_StatusTypeDef status;

        // for (int i = 0; i < 256; i++)
        // {
        //     status = HAL_I2C_IsDeviceReady(&hi2c2, 0x3C, 10, 25);
        //     LEDMGR::display_number(i);
        //     if (status == HAL_OK)
        //     {
        //         break;
        //     }
        // }
        // {
            // while (1)
            // {
            //     LEDMGR::display_number(int(status));
            //     HAL_Delay(500);
            //     LEDMGR::display_number(0);
            //     HAL_Delay(500);
            // }
        // }

        // if (oled::OledResult res = display->begin(cfg); res != oled::OledResult::Ok)
        // {
        //     // Ошибка инициализации - мигаем LED
        //     while (1)
        //     {
        //         LEDMGR::display_number(int(res));
        //         HAL_Delay(500);
        //         LEDMGR::display_number(0);
        //         HAL_Delay(500);
        //     }
        // }
        // display->clear();
        // display->print("Привет STM32!");
        // display->flush();
    }
}