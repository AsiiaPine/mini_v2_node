/* These definitions need to be changed depending on the floating-point precision */
#pragma once
#ifndef SRC_PLATFORM_UBUNTU_MATH_RFFT_HPP_
#define SRC_PLATFORM_UBUNTU_MATH_RFFT_HPP_

typedef double real_t;
#include <fftw3.h>
// #include <rfftw.h>
#include <iostream>
#include <vector>
#include <complex>

#define M_2PI           6.28318530717958647692

#ifdef __cplusplus
extern "C" {
#endif

// for fftw3.0
inline fftw_plan init_rfft(real_t* hanning_window, real_t* in, real_t* out, uint16_t N) {
    for (int n = 0; n < N; n++) {
        const float hanning_value = 0.5f * (1.f - cos(M_2PI * n / (N - 1)));
        hanning_window[n] = hanning_value;
    }
    int N_out = N / 2 + 1;  // Output size for r2c transform
    // Allocate input and output arrays
    in = fftw_alloc_real(N);
    out = fftw_alloc_real(N_out);
    // Create plan
    return fftw_plan_r2r_1d(N, in, out, FFTW_R2HC, FFTW_ESTIMATE);
}

/*
The function apply_hanning_window applies the Hanning window to the input array.
@param in: The input array.
@param out: The output array.
@param hanning_window: The Hanning window.
@param N: The size of the input array.
*/
inline void apply_hanning_window(real_t* in, real_t* out, real_t* hanning_window, int N) {
    for (int i = 0; i < N; i++) {
        out[i] = hanning_window[i] * in[i];
    }
}

/*
The function written based on fftw3 library.
@param plan: The plan for the r2c transform.
*/
inline void rfft_one_cycle(fftw_plan plan, real_t* in, real_t* out) {
    fftw_execute_r2r(plan, in, out);
}

// // for fftw2.1.5
// /*
// The function init_rfft creates a plan for the r2c transform from fftw3 library.
// @param N: The size of the input array.
// @return: The plan for the r2c transform.
// */
// fftw_plan init_rfft(real_t* hanning_window, int N) {
//     for (int n = 0; n < N; n++) {
//         const float hanning_value = 0.5f * (1.f - cos(M_2PI * n / (N - 1)));
//         hanning_window[n] = hanning_value;
//     }
//     // int N_out = N / 2 + 1;  // Output size for r2c transform
//     // Allocate input and output arrays
//     // in = fftw_alloc_real(N);
//     // out = fftw_alloc_complex(N_out);

//     // Create plan
//     return rfftw_create_plan(N, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
// }
// /*
// The function apply_hanning_window applies the Hanning window to the input array.
// @param in: The input array.
// @param out: The output array.
// @param hanning_window: The Hanning window.
// @param N: The size of the input array.
// */
// void apply_hanning_window(real_t* in, real_t* out, real_t* hanning_window, int N) {
//     for (int i = 0; i < N; i++) {
//         out[i] = hanning_window[i] * in[i];
//     }
// }

// void rfft_one_cycle(fftw_plan plan, real_t* in, real_t* out) {
//     rfftw_one(plan, in, out);
//     // std::vector<double> x(in, in + N);
//     // std::copy(x.begin(), x.end(), in);
//     // fftw_execute(plan);
//     // std::vector<double> y(out, out + N);
//     // std::copy(out, out + N, y.begin());
// }
#ifdef __cplusplus
}
#endif

#endif  // SRC_PLATFORM_UBUNTU_MATH_RFFT_HPP_
