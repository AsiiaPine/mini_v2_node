
/*
 * Copyright (C) 2022-2024 Dmitry Ponomarev <ponomarevda96@gmail.com>
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cstring>
#include "FFT.hpp"
// #include "logger.hpp"

// static Logger logger = Logger("FFT");

bool FFT::init(uint16_t window_size, uint16_t num_axes, float sample_rate_hz) {
    if (window_size > FFT_MAX_SIZE) {
        return false;
    }
    bool buffers_allocated = AllocateBuffers(window_size);
    rfft_spec = init_rfft(&_hanning_window, &_fft_input_buffer,
                          &_fft_output_buffer, window_size);
    // rfft_spec = init_rfft(_hanning_window.data(), _fft_input_buffer, _fft_output_buffer, size);
    size = window_size;
    n_axes = num_axes;
    _sample_rate_hz = sample_rate_hz;
    _resolution_hz =  sample_rate_hz / size;
    fft_max_freq = _sample_rate_hz / 2;
    printf("fft _sample_rate_hz: %f\n", _sample_rate_hz);
    printf("fft window_size: %d\n", window_size);
    return buffers_allocated;
}


void FFT::update(float *input) {
    real_t conv_input[n_axes];
    convert_float_to_real_t(input, conv_input, n_axes);
    real_t *data_buffer[] {data_buffer_x.data(), data_buffer_y.data(), data_buffer_z.data()};
    for (uint8_t axis = 0; axis < n_axes; axis++) {
        uint16_t &buffer_index = _fft_buffer_index[axis];

        // for (int n = 0; n < input_size; n++) {
            if (buffer_index < size) {
                // convert int16_t -> real_t (scaling isn't relevant)
                data_buffer[axis][buffer_index] = conv_input[axis] / 2;
                // data_buffer[axis][buffer_index] = input[axis][n] / 2;
                buffer_index++;
                continue;
            }
            apply_hanning_window(data_buffer[axis], _fft_input_buffer,
                                 _hanning_window, size);
                                //  _hanning_window.data(), size);
            rfft_one_cycle(rfft_spec, _fft_input_buffer, _fft_output_buffer);
            _fft_updated = true;
            find_peaks(axis);

            // reset
            // shift buffer (3/4 overlap)
            const int overlap_start = size / 4;
            memmove(&data_buffer[axis][0], &data_buffer[axis][overlap_start],
                    sizeof(real_t) * overlap_start * 3);
            buffer_index = overlap_start * 3;
            _fft_updated = false;
    }
}

void FFT::find_peaks(uint8_t axis) {
    // static float resolution_hz = _sample_rate_hz / size;

    // sum total energy across all used buckets for SNR
    float bin_mag_sum = 0;
    uint16_t num_peaks_found = 0;

    float peak_magnitude[MAX_NUM_PEAKS] {};
    uint16_t raw_peak_index[MAX_NUM_PEAKS] {};

    static float peak_frequencies_prev[MAX_NUM_PEAKS] {};
    for (int i = 0; i < MAX_NUM_PEAKS; i++) {
        peak_frequencies_prev[i] = peak_frequencies[axis][i];
    }

    float fft_output_buffer_float[size];
    convert_real_t_to_float(_fft_output_buffer, fft_output_buffer_float, size);

    for (uint16_t fft_index = 0; fft_index < size/2; fft_index ++) {
        real_t real_imag[2] = {0, 0};
        get_real_imag_by_index(_fft_output_buffer, real_imag, size, fft_index);
        float real_f, imag_f;
        convert_real_t_to_float(&real_imag[0], &real_f, 1);
        convert_real_t_to_float(&real_imag[1], &imag_f, 1);
        const float fft_magnitude = sqrtf(real_f * real_f);
        float frhz = float(_resolution_hz);
        float fbi = float(fft_index);
        float freq_hz = frhz * fbi;
        // const float fft_magnitude = sqrtf(real_f * real_f + imag_f * imag_f);
        printf("%d:\tf: %f\tm: %f\n", fft_index, freq_hz, fft_magnitude);
        _peak_magnitudes_all[fft_index] = fft_magnitude;
        bin_mag_sum += fft_magnitude;
        // int bin_index = fft_index;
        // _peak_magnitudes_all[bin_index] = fft_magnitude;
    }

    // for (uint16_t fft_index = 2; fft_index < size; fft_index += 2) {
    //     float real = fft_output_buffer_float[fft_index];
    //     float imag = fft_output_buffer_float[fft_index + 1];
    //     const float fft_magnitude = sqrtf(real * real + imag * imag);
    //     int bin_index = fft_index / 2;

    //     _peak_magnitudes_all[bin_index] = fft_magnitude;
    //     bin_mag_sum += fft_magnitude;
    // }

    for (uint8_t i = 0; i < MAX_NUM_PEAKS; i++) {
        float largest_peak = 0;
        uint16_t largest_peak_index = 0;

        // Identify i'th peak bin
        for (uint16_t bin_index = 1; bin_index < size/2; bin_index++) {
            float frhz = float(_resolution_hz);
            float fbi = float(bin_index);
            float freq_hz = frhz * fbi;

            if ((_peak_magnitudes_all[bin_index] > largest_peak)
                && (freq_hz >= fft_min_freq)
                && (freq_hz <= fft_max_freq)) {
                largest_peak_index = bin_index;
                largest_peak = _peak_magnitudes_all[bin_index];
            }
        }

        if (largest_peak_index > 0) {
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
        real_t adjusted_bin = 0.5f *
                        estimate_peak_freq(fft_output_buffer_float, 2 * raw_peak_index[peak_new]);
        float adjusted_bin_float;
        convert_real_t_to_float(&adjusted_bin, &adjusted_bin_float, 1);
        if (!std::isfinite(adjusted_bin_float)) {
            continue;
        }
        float freq_adjusted = _resolution_hz * adjusted_bin_float;
        float freq_hz = float(_resolution_hz) * raw_peak_index[peak_new];
        printf("%d:\n freq:\t%f \t adjusted:\t%f\n", peak_new, freq_hz, freq_adjusted);
        // snr is in dB
        auto div = (size - 1) * peak_magnitude[peak_new];
        auto div2 = bin_mag_sum - peak_magnitude[peak_new];
        (void)div;
        (void)div2;
        const float snr = 10.f * log10f((size - 1) * peak_magnitude[peak_new] /
                        (bin_mag_sum - peak_magnitude[peak_new]));

        if (!std::isfinite(freq_adjusted)
            || (snr < MIN_SNR)
            || (freq_adjusted < fft_min_freq)
            || (freq_adjusted > fft_max_freq)) {
                continue;
        }

        // only keep if we're already tracking this frequency or
        // if the SNR is significant
        for (int peak_prev = 0; peak_prev < MAX_NUM_PEAKS; peak_prev++) {
            bool peak_close = (fabsf(freq_adjusted - peak_frequencies_prev[peak_prev])
                                < (_resolution_hz * 0.25f));
            // bool snr_acceptable = (snr > fft_min_snr);
            if (!peak_close && peak_frequencies_prev[peak_prev] > 0) {
            // if (!snr_acceptable && !peak_close) {
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
        // printf("%d:\n freq:\t%f\n", num_peaks_found,
        //             peak_frequencies[axis][num_peaks_found-1]);
        // printf(" snr:\t%f\n", peak_snr[axis][num_peaks_found-1]);
    }
}

static constexpr float tau(float x) {
    // tau(x) = 1/4 * log(3*x*x + 6*x + 1) - sqrt(6)/24*log((x + 1 - sqrt(2/3))/(x + 1 + sqrt(2/3)))
    float addend_1 = 1/4 * logf(3 * x * x + 6 * x + 1);
    float multiplier_2 = sqrtf(6) / 24;
    float addend_2 = logf((x + 1 - sqrtf(2 / 3)) / (x + 1 + sqrtf(2 / 3)));
    return addend_1 - multiplier_2 * addend_2;
}

float FFT::estimate_peak_freq(float fft[], int peak_index) {
    if (peak_index < 2) {
        return NAN;
    }

    // find peak location using Quinn's Second Estimator (2020-06-14: http://dspguru.com/dsp/howtos/how-to-interpolate-fft-peak/)
    float real[3] { fft[peak_index - 2],
                    fft[peak_index],
                    fft[peak_index + 2]};
    float imag[3] { fft[peak_index - 2 + 1],
                    fft[peak_index + 1],
                    fft[peak_index + 2 + 1]};

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
