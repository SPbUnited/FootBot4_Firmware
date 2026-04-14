#include "adc.hpp"

#include "drivers/driver_manager.hpp"
#include "kernel/kernel.hpp"

namespace drivers::adc
{

ADCDriver::ADCDriver(ADCConfig config) : ADCConfig(config) {}

void ADCDriver::init()
{
    clk_enable();

    // Configure GPIO pin as analog input
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &GPIO_InitStruct);

    handle.Instance = instance;

    handle.Init.Resolution = resolution;
    handle.Init.DataAlign = dataAlign;
    handle.Init.ScanConvMode = scanConvMode;
    handle.Init.ContinuousConvMode = continuousConvMode;
    handle.Init.ExternalTrigConv = externalTrigConv;
    handle.Init.ExternalTrigConvEdge = externalTrigConvEdge;

    HAL_StatusTypeDef status = HAL_ADC_Init(&handle);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_ADC_GetError(&handle);
        kerror("ADC init error: %d, error code: %lu\n", int(status), error);
    }

    // Configure channel
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    sConfig.Offset = 0;

    status = HAL_ADC_ConfigChannel(&handle, &sConfig);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_ADC_GetError(&handle);
        kerror("ADC channel config error: %d, error code: %lu\n", int(status), error);
    }
}

uint32_t ADCDriver::read()
{
    uint32_t value = 0;
    HAL_StatusTypeDef status = HAL_ADC_Start(&handle);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_ADC_GetError(&handle);
        kerror("ADC start error: %d, error code: %lu\n", int(status), error);
        return 0;
    }

    status = HAL_ADC_PollForConversion(&handle, 100);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_ADC_GetError(&handle);
        kerror("ADC conversion error: %d, error code: %lu\n", int(status), error);
        HAL_ADC_Stop(&handle);
        return 0;
    }

    value = HAL_ADC_GetValue(&handle);
    HAL_ADC_Stop(&handle);

    return value;
}

void ADCDriver::readMulti(uint32_t *data, uint8_t len)
{
    HAL_StatusTypeDef status = HAL_ADC_Start(&handle);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_ADC_GetError(&handle);
        kerror("ADC start error: %d, error code: %lu\n", int(status), error);
        return;
    }

    for (uint8_t i = 0; i < len; i++)
    {
        status = HAL_ADC_PollForConversion(&handle, 100);
        if (status != HAL_OK)
        {
            uint32_t error = HAL_ADC_GetError(&handle);
            kerror("ADC conversion error: %d, error code: %lu\n", int(status), error);
            HAL_ADC_Stop(&handle);
            return;
        }
        data[i] = HAL_ADC_GetValue(&handle);
    }

    HAL_ADC_Stop(&handle);
}

}  // namespace drivers::adc
