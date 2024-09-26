#ifndef SRC_PLATFORM_STM32_MATH_RFFT_HPP_
#define SRC_PLATFORM_STM32_MATH_RFFT_HPP_

#include "platform/stm32/math/arm_math.h"
#include "platform/stm32/math/arm_const_structs.h"
typedef q15_t real_t;
#define M_2PI           6.28318530717958647692

/*
The function specifies arm_rfft_instance_q15 from CMSIS-DSP library based on the window size.
@param window_size: The size of the input array.
@param hanning_window: Pointer to the Hanning window container.
@param in: used fo incompatability with ubuntu version
@param out: used fo incompatability with ubuntu version
@param N: The size of the Hanning window.
@return: The plan for the r2c transform.
*/
inline arm_rfft_instance_q15 init_rfft(real_t* hanning_window, real_t* in,
                                        real_t* out, uint16_t N) {
    (void)in;
    (void)out;
    arm_rfft_instance_q15 _rfft_q15;
    switch (N) {
        case 256:
            _rfft_q15.fftLenReal = 256;
            _rfft_q15.twidCoefRModifier = 32U;
            _rfft_q15.pCfft = &arm_cfft_sR_q15_len128;
            break;

        case 512:
            _rfft_q15.fftLenReal = 512;
            _rfft_q15.twidCoefRModifier = 16U;
            _rfft_q15.pCfft = &arm_cfft_sR_q15_len256;
            break;

        case 1024:
            _rfft_q15.fftLenReal = 1024;
            _rfft_q15.twidCoefRModifier = 8U;
            _rfft_q15.pCfft = &arm_cfft_sR_q15_len512;
            break;
        default:
            _rfft_q15.fftLenReal = 256;
            _rfft_q15.twidCoefRModifier = 32U;
            _rfft_q15.pCfft = &arm_cfft_sR_q15_len128;
    }
    _rfft_q15.pTwiddleAReal = (real_t *) realCoefAQ15;
    _rfft_q15.pTwiddleBReal = (real_t *) realCoefBQ15;
    _rfft_q15.ifftFlagR = 0;
    _rfft_q15.bitReverseFlagR = 1;
    for (int n = 0; n < N; n++) {
        const float hanning_value = 0.5f * (1.f - cos(M_2PI * n / (N - 1)));
        hanning_window[n] = hanning_value;
        arm_float_to_q15(&hanning_value, &hanning_window[n], 1);
    }
    return _rfft_q15;
}

/*
The function is written based on CMSIS-DSP library.
@param in: The input array.
@param out: The output array.
@param hanning_window: The Hanning window.
@param N: The size of the Hanning window.
*/
inline void apply_hanning_window(real_t* in, real_t* out, real_t* hanning_window, int N) {
    arm_mult_q15(in, hanning_window, out, N);
}

/*
The function is written based on CMSIS-DSP library.
@param _rfft_q15: The plan for the r2c transform.
@param in: The input array.
@param out: The output array.
*/
inline void rfft_one_cycle(arm_rfft_instance_q15 _rfft_q15, real_t* in, real_t* out) {
    arm_rfft_q15(&_rfft_q15, in, out);
}

inline void convert_real_to_float(real_t* in, float* out, uint16_t N) {
    arm_q15_to_float(in, out, N);
}

#endif  // SRC_PLATFORM_STM32_MATH_RFFT_HPP_
