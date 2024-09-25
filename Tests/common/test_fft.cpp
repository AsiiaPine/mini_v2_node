/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 */

#include <gtest/gtest.h>
#include <cmath>  // For std::fabs
#include <algorithm>  // For std::clamp
#include "FFT.hpp"

static constexpr auto ABS_ERR = 0.1f;

class SinSignalGenerator {
public:
    SinSignalGenerator(float sample_rate_hz, float freq_hz, float amplitude) {
        this->sample_rate_hz = sample_rate_hz;
        this->freq_hz = freq_hz;
        this->amplitude = amplitude;
        this->phase = 0;
    }
    int16_t get_next_sample() {
        int16_t sample = amplitude * std::sin(2 * M_PI * freq_hz * phase);
        phase += 1 / sample_rate_hz;
        return sample;
    }
private:
    float sample_rate_hz;
    float freq_hz;
    float amplitude;
    float phase;
};

class TetsFFTBase : public ::testing::Test {
private:
    /* data */

public:
    TetsFFTBase(/* args */);
    ~TetsFFTBase();
};

TetsFFTBase::TetsFFTBase(/* args */) {
}

TetsFFTBase::~TetsFFTBase() {
}
struct InitParameters   {
    float sample_rate_hz;
    float freq_hz;
    float amplitude;
};

class TestFFTOnSignalParametrized : public TetsFFTBase, public
                ::testing::WithParamInterface<InitParameters> {
};

TEST_P(TestFFTOnSignalParametrized, CheckOnSeveralSignalsUpdateAfterFilling) {
    auto parameters = GetParam();
    SinSignalGenerator signal_generator(parameters.sample_rate_hz,
                                        parameters.freq_hz,
                                        parameters.amplitude);
    FFT fft;
    fft.init(1024, 3, parameters.sample_rate_hz);
    int16_t *input[3] {0, 0, 0};
    for (int i = 0; i < 1024; i++) {
        input[0][i] = signal_generator.get_next_sample();
        input[1][i] = signal_generator.get_next_sample();
        input[2][i] = signal_generator.get_next_sample();
    }
    fft.update(input, 1024);
    EXPECT_NEAR(fft.peak_frequencies[0], parameters.freq_hz, ABS_ERR);
    EXPECT_NEAR(fft.peak_snr[0], 1.0f, ABS_ERR);
}

TEST_P(TestFFTOnSignalParametrized, CheckOnSeveralSignalsUpdateOnline) {
    auto parameters = GetParam();
    SinSignalGenerator signal_generator(parameters.sample_rate_hz,
                                        parameters.freq_hz,
                                        parameters.amplitude);
    FFT fft;
    fft.init(1024, 3, parameters.sample_rate_hz);
    int16_t *input[3] {0, 0, 0};
    for (int i = 0; i < 1024; i++) {
        input[0][i] = signal_generator.get_next_sample();
        input[1][i] = signal_generator.get_next_sample();
        input[2][i] = signal_generator.get_next_sample();
        fft.update(input, 1);
    }
    EXPECT_NEAR(fft.peak_frequencies[0], parameters.freq_hz, ABS_ERR);
    EXPECT_NEAR(fft.peak_snr[0], 1.0f, ABS_ERR);
}

INSTANTIATE_TEST_SUITE_P(TestFFTOnSignalParametrized,
                         TestFFTOnSignalParametrized,
                         ::testing::Values(
                             InitParameters{1000, 100, 1},
                             InitParameters{1000, 1000, 1},
                             InitParameters{1000, 2000, 1},
                             InitParameters{1000, 3000, 1})
                         );

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
