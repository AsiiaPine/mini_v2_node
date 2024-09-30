/*
 * Copyright (C) 2022-2024 Dmitry Ponomarev <ponomarevda96@gmail.com>
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef SRC_COMMON_ALGORITHMS_FFT_H_
#define SRC_COMMON_ALGORITHMS_FFT_H_

#include "rfft.hpp"
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FFT_MAX_SIZE    1024
#define MAX_NUM_PEAKS   3
#define MIN_SNR         1.0f
#define NUM_AXES        3

class FFT {
public:
    bool init(uint16_t window_size, uint16_t num_axes, float sample_rate_hz);

    void update(float *input);
    // void update(real_t *input[], uint8_t input_size);
    float fft_min_freq = 0;
    float fft_max_freq;
    // float fft_min_snr = MIN_SNR;
    float _resolution_hz;
    float peak_frequencies [NUM_AXES][MAX_NUM_PEAKS] {};
    float peak_snr [NUM_AXES][MAX_NUM_PEAKS] {};

private:
    uint16_t size;

    // real_t *data_buffer_x{nullptr};
    // real_t *data_buffer_y{nullptr};
    // real_t *data_buffer_z{nullptr};
    real_t *_hanning_window{nullptr};
    real_t *_fft_output_buffer{nullptr};
    real_t *_fft_input_buffer{nullptr};
    std::vector<real_t> data_buffer_x;
    std::vector<real_t> data_buffer_y;
    std::vector<real_t> data_buffer_z;
    // std::vector<real_t> _hanning_window;
    // std::vector<real_t> _fft_input_buffer;
    // std::vector<real_t> _fft_output_buffer;
    std::vector<float> _peak_magnitudes_all;
    uint16_t _fft_buffer_index[3] {};
    bool _fft_updated{false};
    uint8_t n_axes;
    float _sample_rate_hz;

    float estimate_peak_freq(float fft[], int peak_index);
    void find_peaks(uint8_t axis);
    bool AllocateBuffers(uint16_t N) {
        data_buffer_x.resize(N);
        if (n_axes > 1) {
            data_buffer_y.resize(N);
        }
        if (n_axes > 2) {
            data_buffer_z.resize(N);
        }
        // _fft_input_buffer = allocate_buffer(N);
        // _hanning_window = allocate_buffer(N);
        // _fft_output_buffer = allocate_buffer(N * 2);
        _peak_magnitudes_all.resize(N);
        return (data_buffer_x.data() && data_buffer_y.data() && data_buffer_z.data()
                && _peak_magnitudes_all.data());
            // && _hanning_window.data() && _peak_magnitudes_all);
    }
    #ifdef HAL_MODULE_ENABLED
        // specification of arm_rfft_instance_q15
        // https://arm-software.github.io/CMSIS_5/DSP/html/group__RealFFT.html
        arm_rfft_instance_q15 rfft_spec;
    #else
        // plan for the r2c transform from fftw3 library.
        fftw_plan rfft_spec;
    #endif
};

#ifdef __cplusplus
}
#endif

#endif  // SRC_COMMON_ALGORITHMS_FFT_H_
