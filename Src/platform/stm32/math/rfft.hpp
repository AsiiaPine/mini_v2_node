#ifndef SRC_PLATFORM_STM32_MATH_RFFT_HPP_
#define SRC_PLATFORM_STM32_MATH_RFFT_HPP_

#include "arm_math.h"
#include "arm_const_structs.h"
typedef q15_t real_t;

void init_rfft(uint16_t window_size, arm_rfft_instance_q15 _rfft_q15) {
    switch (window_size) {
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
}
rfft_one_cycle
#endif  // SRC_PLATFORM_STM32_MATH_RFFT_HPP_
