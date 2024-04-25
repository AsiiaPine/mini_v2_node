/***
 * Copyright (C) 2024 Anastasiia Stepanova  <asiiapine96@gmail.com>
 *  Distributed under the terms of the GPL v3 license, available in the file LICENSE.
***/ 

#include "periphery/adc/adc.hpp"
#include "main.h"

extern ADC_HandleTypeDef hadc1;
static uint32_t adc_current_avg = 0;
static uint32_t adc_current_sum = 0;
static uint32_t adc_dma_counter = 0;

static uint32_t adc_force_avg_1 = 0;
static uint32_t adc_force_sum_1 = 0;
static uint32_t adc_force_dma_counter_1 = 0;

static uint32_t adc_force_avg_2 = 0;
static uint32_t adc_force_sum_2 = 0;
static uint32_t adc_force_dma_counter_2 = 0;

static inline uint16_t adc_dma_buffer[static_cast<uint8_t>(AdcChannel::ADC_NUMBER_OF_CNANNELS)];

int8_t AdcPeriphery::init() {
    if (_is_adc_already_inited) return 0;
    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK) {
        return -1;
    }

    auto channels_amount = static_cast<uint8_t>(AdcChannel::ADC_NUMBER_OF_CNANNELS);
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)(void*)&adc_dma_buffer, channels_amount) != HAL_OK) {
        return -1;
    }

    _is_adc_already_inited = true;
    return 0;
}

uint16_t AdcPeriphery::get(AdcChannel channel) {
    if (!_is_adc_already_inited || channel >= AdcChannel::ADC_NUMBER_OF_CNANNELS) {
        return 0;
    }
    if (channel == AdcChannel::ADC_CURRENT) {
        return adc_current_avg;
    }
    if (channel == AdcChannel::ADC_IN4) {
        return adc_force_avg_1;
    }
    if (channel == AdcChannel::ADC_IN5) {
        return adc_force_avg_2;
    }
    return adc_dma_buffer[static_cast<uint8_t>(channel)];
}

#ifdef HAL_ADC_MODULE_ENABLED
/**
 * @note We assume that hadc->Instance == ADC1 always!
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    adc_dma_counter++;

    adc_current_sum += adc_dma_buffer[static_cast<uint8_t>(AdcChannel::ADC_CURRENT)];
    if (adc_dma_counter > 4000) {
        adc_current_avg = adc_current_sum / adc_dma_counter;
        adc_current_sum = 0;

        adc_dma_counter = 0;
    }

    adc_force_dma_counter_1++;
    adc_force_sum_1 += adc_dma_buffer[static_cast<uint8_t>(AdcChannel::ADC_IN4)];
    if (adc_force_dma_counter_1 > 4000) {
        adc_force_avg_1 = (uint16_t)(adc_force_sum_1 / (float)(adc_force_dma_counter_1));
        adc_force_sum_1 = 0;

        adc_force_dma_counter_1 = 0;
    }

    adc_force_sum_2 += adc_dma_buffer[static_cast<uint8_t>(AdcChannel::ADC_IN5)];
    adc_force_dma_counter_2++;
    if (adc_force_dma_counter_2 > 4000) {
        adc_force_avg_2 = adc_force_sum_2 / adc_force_dma_counter_2;
        adc_force_sum_2 = 0;
        adc_force_dma_counter_2 = 0;
    }

    HAL_ADC_Start_DMA(&hadc1,
                      (uint32_t*)(void*)&adc_dma_buffer,
                      static_cast<uint8_t>(AdcChannel::ADC_NUMBER_OF_CNANNELS)
    );
}
#endif
