#include "gpio.hpp"
#include "kernel/kernel.hpp"
#include "stm32f4xx_hal_adc_ex.h"

namespace drivers::gpio
{
uint32_t adcData[1];
void GPIOOutputDriver::init(GPIODescriptor pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin.GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(pin.GPIOx, &GPIO_InitStruct);

    static_cast<GPIODescriptor&>(*this) = pin;
}

void GPIOOutputDriver::write(bool state)
{
    HAL_GPIO_WritePin(this->GPIOx, this->GPIO_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void GPIOOutputDriver::toggle()
{
    HAL_GPIO_TogglePin(this->GPIOx, this->GPIO_Pin);
}

void GPIOInputDriver::init(GPIODescriptor pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin.GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(pin.GPIOx, &GPIO_InitStruct);

    static_cast<GPIODescriptor&>(*this) = pin;
}

bool GPIOInputDriver::read()
{
    return HAL_GPIO_ReadPin(this->GPIOx, this->GPIO_Pin);
}

void GPIOAnalogInputDriver::init(GPIODescriptor pin, ADCDescriptor adc_config)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin.GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    // GPIO_InitStruct.Alternate = pin.Alternate;
    HAL_GPIO_Init(pin.GPIOx, &GPIO_InitStruct);

    // handle.Instance = adc_config.instance;
    handle.Instance = ADC1;
    handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    handle.Init.Resolution = ADC_RESOLUTION_12B;
    handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    handle.Init.ScanConvMode = ENABLE;
    handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    handle.Init.ContinuousConvMode = ENABLE;
    handle.Init.NbrOfConversion = 1;
    handle.Init.DiscontinuousConvMode = DISABLE;
    handle.Init.NbrOfDiscConversion = 0;
    handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    handle.Init.DMAContinuousRequests = ENABLE;

    // handle
    // handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV6;
    // handle.Init.Resolution = adc_config.resolution;
    // handle.Init.DataAlign = adc_config.dataAlign;
    // handle.Init.ScanConvMode = adc_config.scanConvMode;
    // handle.Init.ContinuousConvMode = adc_config.continuousConvMode;
    // handle.Init.ExternalTrigConv = adc_config.externalTrigConv;
    // // handle.Init.ExternalTrigConvEdge = externalTrigConvEdge;
    // handle.Init.NbrOfConversion = 1;

    // ADC_Common_

    HAL_StatusTypeDef status = HAL_ADC_Init(&handle);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_ADC_GetError(&handle);
        kinfo("ADC init error: %d, error code: %lu\n", int(status), error);
    }

    // Configure channel
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_11;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
    sConfig.Offset = 0;

    status = HAL_ADC_ConfigChannel(&handle, &sConfig);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_ADC_GetError(&handle);
        kinfo("ADC channel config error: %d, error code: %lu\n", int(status), error);
    }

    // HAL_ADCEx_Calibration_Start(&handle);
    
    // start_status = HAL_ADC_PollForConversion(&handle, 100);
    // if (start_status != HAL_OK)
    // {
    //     // uint32_t error = HAL_ADC_GetError(&handle);
    //     // kerror("ADC conversion error: %d, error code: %lu\n", int(status), error);
    //     HAL_ADC_Stop(&handle);
    //     // return 0;
    // }
    HAL_ADC_Start_DMA(&handle, adcData, 1);
    // HAL_StatusTypeDef statusasd = HAL_ADC_Start(&handle);
    // if (statusasd != HAL_OK)
    // {
    //     uint32_t error = HAL_ADC_GetError(&handle);
    //     kinfo("ADC start error: %d, error code: %lu\n", int(status), error);
    //     // return 0;
    // }

    static_cast<GPIODescriptor&>(*this) = pin;
}

uint32_t GPIOAnalogInputDriver::read()
{
    uint32_t value = 0;
    return adcData[0];
    // HAL_StatusTypeDef statusasd = HAL_ADC_Start(&handle);
    // if (statusasd != HAL_OK)
    // {
    //     uint32_t error = HAL_ADC_GetError(&handle);
    //     // kinfo("ADC start error: %d, error code: %lu\n", int(status), error);
    //     // return 0;
    // }

    // HAL_StatusTypeDef status = HAL_ADC_PollForConversion(&handle, 10);
    // if (status != HAL_OK)
    // {
    //     uint32_t error = HAL_ADC_GetError(&handle);
    //     // kinfo("ADC conversion error: %d, error code: %lu\n", int(status), error);
    //     HAL_ADC_Stop(&handle);
    //     if (error != 0)
    //     {
            
    //     }
    //     return 0;
    // }

    // value = HAL_ADC_GetValue(&handle);
    // // HAL_ADC_Stop(&handle);
    // return value;
}

}  // namespace drivers::gpio
