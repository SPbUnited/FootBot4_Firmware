#pragma once

#include "stm32f4xx_hal.h"

#define LED_GPIO_CLK_ENABLE() __HAL_RCC_GPIOG_CLK_ENABLE()

namespace LEDMGR
{

    enum Leds
    {
        STM32,
        DRV1,
        DRV2,
        DRV3,
        DRV4,
        DRV5,
        DATA_TRANSFER_STATUS_1,
        DATA_TRANSFER_STATUS_2,
        NUM_LEDS
    };

    struct LedDescriptor
    {
        GPIO_TypeDef *GPIOx;
        uint16_t GPIO_Pin;
    };

    LedDescriptor leds[] = {
        [STM32] = {GPIOD, GPIO_PIN_15},
        [DRV1] = {GPIOG, GPIO_PIN_2},
        [DRV2] = {GPIOG, GPIO_PIN_3},
        [DRV3] = {GPIOG, GPIO_PIN_4},
        [DRV4] = {GPIOG, GPIO_PIN_5},
        [DRV5] = {GPIOG, GPIO_PIN_6},
        [DATA_TRANSFER_STATUS_1] = {GPIOG, GPIO_PIN_7},
        [DATA_TRANSFER_STATUS_2] = {GPIOG, GPIO_PIN_8},
    };

    void init_led(LedDescriptor led)
    {
        LED_GPIO_CLK_ENABLE(); // Скорее всего надо сделать switch case для всех GPIO
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = led.GPIO_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(led.GPIOx, &GPIO_InitStruct);
    }

    void init_leds()
    {
        for (auto &led : leds)
        {
            init_led(led);
        }
    }

    void write_led(Leds led, bool state)
    {
        HAL_GPIO_WritePin(leds[led].GPIOx, leds[led].GPIO_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

    void toggle_led(Leds led)
    {
        HAL_GPIO_TogglePin(leds[led].GPIOx, leds[led].GPIO_Pin);
    }

} // namespace LEDMGR