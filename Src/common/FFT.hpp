/*
 * Copyright (C) 2022-2024 Dmitry Ponomarev <ponomarevda96@gmail.com>
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef SRC_COMMON_ALGORITHMS_FFT_H_
#define SRC_COMMON_ALGORITHMS_FFT_H_

#include <stdint.h>
#include <string>
#include <iostream>

#include "rfft.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#define M_2PI           6.28318530717958647692
#define M_PI            3.14159265358979323846
#define M_PI_2          1.57079632679489661923
#define M_PI_4          0.78539816339744830962
#define FFT_MAX_SIZE    1024
#define MAX_NUM_PEAKS   3
#define MIN_SNR         1.0f
#define NUM_AXES        3

class FFT {
public:
    bool init(uint16_t window_size, uint16_t num_axes, float sample_rate_hz);

    void update(real_t *input);
    // void update(real_t *input[], uint8_t input_size);
    float fft_min_freq = 0;
    float fft_max_freq = _sample_rate_hz / 2;
    float fft_min_snr = MIN_SNR;
    uint16_t _resolution_hz;
    float peak_frequencies [NUM_AXES][MAX_NUM_PEAKS] {};
    float peak_snr [NUM_AXES][MAX_NUM_PEAKS] {};

private:
    uint16_t size;
    real_t *data_buffer_x{nullptr};
    real_t *data_buffer_y{nullptr};
    real_t *data_buffer_z{nullptr};
    real_t *_hanning_window{nullptr};
    real_t *_fft_input_buffer{nullptr};
    real_t *_fft_outupt_buffer{nullptr};
    real_t *_peak_magnitudes_all{nullptr};
    uint16_t _fft_buffer_index[3] {};
    bool _fft_updated{false};
    uint8_t n_axes;
    uint16_t _sample_rate_hz;

    float estimate_peak_freq(real_t fft[], int peak_index);
    void find_peaks(uint8_t axis);
    bool AllocateBuffers(uint16_t N) {
        data_buffer_x = new real_t[N];
        if (n_axes > 1) {
            data_buffer_y = new real_t[N];
        }
        if (n_axes > 2) {
            data_buffer_z = new real_t[N];
        }
        _hanning_window = new real_t[N];

        _peak_magnitudes_all = new real_t[N];
        _fft_input_buffer = new real_t[N];
        _fft_outupt_buffer = new real_t[N * 2];
        return (data_buffer_x && data_buffer_y && data_buffer_z
            && _hanning_window
            && _fft_input_buffer
            && _fft_outupt_buffer);
    }
    #ifndef HAL_MODULE_ENABLED
        // plan for the r2c transform from fftw3 library.
        fftw_plan rfft_plan;
    #else
        // specification of arm_rfft_instance_q15
        // https://arm-software.github.io/CMSIS_5/DSP/html/group__RealFFT.html
        arm_rfft_instance_q15 _rfft_q15;
    #endif
};

#ifdef __cplusplus
}
#endif

#endif  // SRC_COMMON_ALGORITHMS_FFT_H_
