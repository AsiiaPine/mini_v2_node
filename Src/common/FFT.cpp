
/*
 * Copyright (C) 2022-2024 Dmitry Ponomarev <ponomarevda96@gmail.com>
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "FFT.hpp"
#include <cstring>

bool FFT::init(uint16_t window_size, uint16_t num_axes, float sample_rate_hz) {
    if (window_size > FFT_MAX_SIZE) {
        return false;
    }
    size = window_size;
    n_axes = num_axes;
    _sample_rate_hz = sample_rate_hz;
    _resolution_hz =  sample_rate_hz / size;
    for (int n = 0; n < size; n++) {
        const float hanning_value = 0.5f * (1.f - cos(M_2PI * n / (size - 1)));
        #ifndef HAL_MODULE_ENABLED
            _hanning_window[n] = hanning_value;
        #else
            arm_float_to_q15(&hanning_value, &_hanning_window[n], 1);
        #endif
    }
    bool buffers_allocated = AllocateBuffers(window_size);
    #ifndef HAL_MODULE_ENABLED
        rfft_plan = init_rfft(size);
        // rfft_plan = init_rfft(_fft_input_buffer, _fft_outupt_buffer, size);
    #else
        // _fft_input_buffer = new real_t[window_size];
        // _fft_outupt_buffer = new real_t[window_size * 2];
        init_rfft(size, _rfft_q15);

    #endif
    return buffers_allocated;
}


void FFT::update(real_t *input) {
    real_t *data_buffer[] {data_buffer_x, data_buffer_y, data_buffer_z};

    for (int axis = 0; axis < n_axes; axis++) {
        uint16_t &buffer_index = _fft_buffer_index[axis];

        // for (int n = 0; n < input_size; n++) {
            if (buffer_index < size) {
                // convert int16_t -> real_t (scaling isn't relevant)
                data_buffer[axis][buffer_index] = input[axis] / 2;
                // data_buffer[axis][buffer_index] = input[axis][n] / 2;
                buffer_index++;
                continue;
            }

            // if we have enough samples begin processing, but only one FFT per cycle
            if (_fft_updated) {
                continue;
            }
            #ifndef HAL_MODULE_ENABLED
                // windowing
                for (int i = 0; i < size; i++) {
                    _fft_input_buffer[i] = _hanning_window[i] * data_buffer[axis][i];
                }
                // fft
                rfft_one_cycle(rfft_plan, _fft_input_buffer, _fft_outupt_buffer, size);
            #else
                // windowing
                arm_mult_q15(data_buffer[axis], _hanning_window, _fft_input_buffer, size);
                // fft
                arm_rfft_q15(&_rfft_q15, _fft_input_buffer, _fft_outupt_buffer);
            #endif
            _fft_updated = true;
            find_peaks(axis);

            // reset
            // shift buffer (3/4 overlap)
            const int overlap_start = size / 4;
            memmove(&data_buffer[axis][0], &data_buffer[axis][overlap_start],
                    sizeof(real_t) * overlap_start * 3);
            buffer_index = overlap_start * 3;
        // }
    }
}

void FFT::find_peaks(uint8_t axis) {
    const float resolution_hz = _sample_rate_hz / size;

    // sum total energy across all used buckets for SNR
    float bin_mag_sum = 0;
    float peak_magnitude[MAX_NUM_PEAKS] {};
    static float peak_frequencies_prev[MAX_NUM_PEAKS] {};
    uint16_t raw_peak_index[MAX_NUM_PEAKS] {};
    uint16_t num_peaks_found = 0;
    // FFT output buffer is ordered as follows:
    // [real[0], imag[0], real[1], imag[1], real[2], imag[2] ... real[(N/2)-1], imag[(N/2)-1]
    for (uint16_t fft_index = 2; fft_index < size; fft_index += 2) {
        const float real = _fft_outupt_buffer[fft_index];
        const float imag = _fft_outupt_buffer[fft_index + 1];

        const float fft_magnitude = sqrtf(real * real + imag * imag);

        int bin_index = fft_index / 2;

        _peak_magnitudes_all[bin_index] = fft_magnitude;
        bin_mag_sum += fft_magnitude;
    }
    for (int i = 0; i < MAX_NUM_PEAKS; i++) {
        float largest_peak = 0;
        int largest_peak_index = 0;

        // Identify i'th peak bin
        for (int bin_index = 1; bin_index < size; bin_index++) {
            const float freq_hz = bin_index * resolution_hz;

            if ((_peak_magnitudes_all[bin_index] > largest_peak)
                && (freq_hz >= fft_min_freq)
                && (freq_hz <= fft_max_freq)) {
                largest_peak = _peak_magnitudes_all[bin_index];
                largest_peak_index = bin_index;
            }
        }

        if (largest_peak_index > 1) {
            raw_peak_index[i] = largest_peak_index;
            peak_magnitude[i] = _peak_magnitudes_all[largest_peak_index];

            // remove peak + sides (included in frequency estimate later)
            _peak_magnitudes_all[largest_peak_index - 1] = 0;
            _peak_magnitudes_all[largest_peak_index]     = 0;
            _peak_magnitudes_all[largest_peak_index + 1] = 0;
        }
    }
    for (int peak_new = 0; peak_new < MAX_NUM_PEAKS; peak_new++) {
        if (raw_peak_index[peak_new] == 0) {
            continue;
        }
        const float adjusted_bin = 0.5f *
                        estimate_peak_freq(_fft_outupt_buffer, 2 * raw_peak_index[peak_new]);

        if (!std::isfinite(adjusted_bin)) {
            continue;
        }
        const float freq_adjusted = resolution_hz * adjusted_bin;
        // snr is in dB
        const float snr = 10.f * log10f((size - 1) * peak_magnitude[peak_new] /
                        (bin_mag_sum - peak_magnitude[peak_new]));

        if (!std::isfinite(freq_adjusted)
            || (snr < MIN_SNR)
            || (freq_adjusted < fft_min_freq)
            || (freq_adjusted < fft_max_freq)) {
                continue;
        }
        // only keep if we're already tracking this frequency or
        // if the SNR is significant
        for (int peak_prev = 0; peak_prev < MAX_NUM_PEAKS; peak_prev++) {
            bool peak_close = (fabsf(freq_adjusted - peak_frequencies_prev[peak_prev])
                                < (resolution_hz * 0.25f));
            bool snr_acceptable = (snr > fft_min_snr);
            if (!snr_acceptable && !peak_close) {
                continue;
            }
            // keep
            peak_frequencies[axis][num_peaks_found] = freq_adjusted;
            peak_snr[axis][num_peaks_found] = snr;

            // remove
            if (peak_close) {
                peak_frequencies_prev[peak_prev] = NAN;
            }
            num_peaks_found++;
            break;
        }
    }
}

static constexpr float tau(float x) {
    // tau(x) = 1/4 * log(3*x*x + 6*x + 1) - sqrt(6)/24*log((x + 1 - sqrt(2/3))/(x + 1 + sqrt(2/3)))
    float addend_1 = 1/4 * log(3 * x * x + 6 * x + 1);
    float multiplier_2 = sqrt(6) / 24;
    float addend_2 = log((x + 1 - sqrt(2 / 3)) / (x + 1 + sqrt(2 / 3)));
    return addend_1 - multiplier_2 * addend_2;
}

float FFT::estimate_peak_freq(real_t fft[], int peak_index) {
    if (peak_index < 2) {
        return NAN;
    }

    // find peak location using Quinn's Second Estimator (2020-06-14: http://dspguru.com/dsp/howtos/how-to-interpolate-fft-peak/)
    float real[3] { (float)fft[peak_index - 2],
                    (float)fft[peak_index],
                    (float)fft[peak_index + 2]};
    float imag[3] { (float)fft[peak_index - 2 + 1],
                    (float)fft[peak_index + 1],
                    (float)fft[peak_index + 2 + 1]};

    static constexpr int k = 1;

    const float divider = (real[k] * real[k] + imag[k] * imag[k]);

    // ap = (X[k + 1].r * X[k].r + X[k+1].i * X[k].i) / (X[k].r * X[k].r + X[k].i * X[k].i)
    float ap = (real[k + 1] * real[k] + imag[k + 1] * imag[k]) / divider;

    // dp = -ap / (1 – ap)
    float dp = -ap  / (1.f - ap);

    // am = (X[k - 1].r * X[k].r + X[k – 1].i * X[k].i) / (X[k].r * X[k].r + X[k].i * X[k].i)
    float am = (real[k - 1] * real[k] + imag[k - 1] * imag[k]) / divider;

    // dm = am / (1 – am)
    float dm = am / (1.f - am);

    // d = (dp + dm) / 2 + tau(dp * dp) – tau(dm * dm)
    float d = (dp + dm) / 2.f + tau(dp * dp) - tau(dm * dm);

    // k’ = k + d
    return peak_index + 2.f * d;
}
