#include "gpio.hpp"
#include "kernel/kernel.hpp"
#include "stm32f4xx_hal_adc_ex.h"

// External declaration for ADC MSP initialization
extern "C" void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc);

namespace drivers::gpio
{

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

void GPIOAnalogInputDriver::init(GPIODescriptor pin, ADCDescriptor adc, uint32_t* buffer, uint32_t size)
{
    // Enable GPIO clock for the specific port
    if (pin.GPIOx == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (pin.GPIOx == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (pin.GPIOx == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (pin.GPIOx == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (pin.GPIOx == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();
    else if (pin.GPIOx == GPIOF) __HAL_RCC_GPIOF_CLK_ENABLE();
    else if (pin.GPIOx == GPIOG) __HAL_RCC_GPIOG_CLK_ENABLE();
    else if (pin.GPIOx == GPIOH) __HAL_RCC_GPIOH_CLK_ENABLE();
    else if (pin.GPIOx == GPIOI) __HAL_RCC_GPIOI_CLK_ENABLE();

    // Enable ADC clock
    __HAL_RCC_ADC1_CLK_ENABLE();

    // Enable DMA clock
    __HAL_RCC_DMA2_CLK_ENABLE();

    // Configure GPIO pin as analog input
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin.GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(pin.GPIOx, &GPIO_InitStruct);

    // Store buffer info
    this->adcDataBuffer = buffer;
    this->bufferSize = size;
    // setBuffer(buffer);
    // setBufferSize(size);

    // Initialize ADC handle
    handle.Instance = ADC1;
    handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    handle.Init.Resolution = adc.resolution;
    handle.Init.DataAlign = adc.dataAlign;
    handle.Init.ScanConvMode = adc.scanConvMode;
    handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    handle.Init.ContinuousConvMode = adc.continuousConvMode;
    handle.Init.NbrOfConversion = 1;
    handle.Init.DiscontinuousConvMode = DISABLE;
    handle.Init.NbrOfDiscConversion = 0;
    handle.Init.ExternalTrigConv = adc.externalTrigConv;
    handle.Init.ExternalTrigConvEdge = adc.externalTrigConvEdge;
    handle.Init.DMAContinuousRequests = ENABLE;

    HAL_StatusTypeDef status = HAL_ADC_Init(&handle);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_ADC_GetError(&handle);
        kinfo("ADC init error: %d, error code: %lu\n", int(status), error);
    }
    else
    {
        kinfo("ADC initialized successfully\n");
    }

    // Configure ADC channel
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = adc.channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
    sConfig.Offset = 0;

    status = HAL_ADC_ConfigChannel(&handle, &sConfig);
    if (status != HAL_OK)
    {
        uint32_t error = HAL_ADC_GetError(&handle);
        kinfo("ADC channel config error: %d, error code: %lu\n", int(status), error);
    }
    else
    {
        kinfo("ADC channel configured successfully\n");
    }

    // Configure DMA for ADC
    hdma_adc.Instance = DMA2_Stream0;
    hdma_adc.Init.Channel = DMA_CHANNEL_0;
    hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc.Init.Mode = DMA_CIRCULAR;
    hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_adc.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_adc.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
    hdma_adc.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_adc.Init.PeriphBurst = DMA_PBURST_SINGLE;

    HAL_DMA_Init(&hdma_adc);

    // Associate DMA handle with ADC handle
    __HAL_LINKDMA(&handle, DMA_Handle, hdma_adc);
    
    kinfo("DMA configured and linked to ADC\n");

    // Start DMA conversion
    if (adcDataBuffer != nullptr && bufferSize > 0)
    {
        kinfo("Starting DMA: buffer=%p, size=%d\n", (void*)adcDataBuffer, bufferSize);
        kinfo("DMA2_Stream0 CR = 0x%08X\n", DMA2_Stream0->CR);
        kinfo("DMA2_Stream0 NDTR = 0x%08X\n", DMA2_Stream0->NDTR);
        kinfo("DMA2_Stream0 PAR = 0x%08X\n", DMA2_Stream0->PAR);
        kinfo("DMA2_Stream0 M0AR = 0x%08X\n", DMA2_Stream0->M0AR);
        
        status = HAL_ADC_Start_DMA(&handle, adcDataBuffer, bufferSize);
        if (status != HAL_OK)
        {
            uint32_t error = HAL_ADC_GetError(&handle);
            kinfo("ADC DMA start error: %d, error code: %lu\n", int(status), error);
            // Try to get more info from DMA
            uint32_t dma_error = HAL_DMA_GetError(&hdma_adc);
            kinfo("DMA error code: %lu\n", dma_error);
        }
        else
        {
            kinfo("DMA started successfully!\n");
        }
    }

    // static_cast<GPIODescriptor&>(*this) = pin;
}

uint32_t GPIOAnalogInputDriver::read()
{
    if (this->adcDataBuffer != nullptr && this->bufferSize > 0)
    {
        return this->adcDataBuffer[0];
    }
    return 0;
}

// void GPIOAnalogInputDriver::startDMA()
// {
//     if (adcDataBuffer != nullptr && bufferSize > 0)
//     {
//         HAL_StatusTypeDef status = HAL_ADC_Start_DMA(&handle, adcDataBuffer, bufferSize);
//         if (status != HAL_OK)
//         {
//             uint32_t error = HAL_ADC_GetError(&handle);
//             kinfo("ADC DMA start error: %d, error code: %lu\n", int(status), error);
//         }
//     }
// }

// void GPIOAnalogInputDriver::stopDMA()
// {
//     HAL_ADC_Stop_DMA(&handle);
// }

// // ADC MSP initialization - required for DMA support
// extern "C" void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
// {
//     if (hadc->Instance == ADC1)
//     {
//         // Enable ADC clock
//         __HAL_RCC_ADC1_CLK_ENABLE();
        
//         // Enable DMA clock
//         __HAL_RCC_DMA2_CLK_ENABLE();
//     }
// }

}  // namespace drivers::gpio
