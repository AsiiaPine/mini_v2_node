/***
 * Copyright (C) 2024 Anastasiia Stepanova  <asiiapine96@gmail.com>
 *  Distributed under the terms of the GPL v3 license, available in the file LICENSE.
***/ 

#include "periphery/adc/adc.hpp"

struct adcState {
    uint16_t current;
    uint16_t v_in;
    uint16_t v_5v;
    uint16_t version;
    uint16_t force_1;
    uint16_t force_2;
    uint16_t temperature;
};
adcState state = {};

int8_t AdcPeriphery::init() {
    state.temperature = 3115;
    state.current = 2048;
    state.v_in = 640;
    state.force_1 = 0;
    state.force_2 = 0;

    _is_adc_already_inited = true;
    return 0;
}

uint16_t AdcPeriphery::get(AdcChannel channel) {
    switch (channel)
    {
    case AdcChannel::ADC_VIN:
        return state.v_in;
    case AdcChannel::ADC_CURRENT:
        return state.current;
    #ifdef AdcChannel::ADC_TEMPERATURE
    case AdcChannel::ADC_TEMPERATURE:
        return state.temperature;
    #endif
    case AdcChannel::ADC_IN4:
        return state.force_1;
    case AdcChannel::ADC_IN5:
        return state.force_2;
    default:
        break;
    }
    return 0;
}
