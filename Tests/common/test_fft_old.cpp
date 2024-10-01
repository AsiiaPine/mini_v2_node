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
template <typename T>
auto IsInRange(T lo, T hi) {
    return AllOf(Ge((lo)), Le((hi)));
}
::testing::AssertionResult IsBetweenInclusive(int val, int a, int b)
{
    if((val >= a) && (val <= b))
        return ::testing::AssertionSuccess();
    else
        return ::testing::AssertionFailure()
               << val << " is outside the range " << a << " to " << b;
}

struct InitFFTParamType   {
    float sample_rate_hz;
    uint8_t n_axes;
    uint16_t window_size;
};

struct InitOneSignParamType   {
    float sample_rate_hz = 0;
    float freq_hz = 0;
    float amplitude = 0;
};

class SinSignalGenerator {
public:
    SinSignalGenerator(){}
    explicit SinSignalGenerator(InitOneSignParamType signal_parameters) {
        this->sample_rate_hz = signal_parameters.sample_rate_hz;
        this->freq_hz = signal_parameters.freq_hz;
        this->amplitude = signal_parameters.amplitude;
        this->phase = 0;
        this->secs = 0;
    }
    SinSignalGenerator(float sample_rate_hz, float freq_hz, float amplitude) {
        this->sample_rate_hz = sample_rate_hz;
        this->freq_hz = freq_hz;
        this->amplitude = amplitude;
        this->phase = 0;
        this->secs = 0;
    }
    float get_next_sample() {
        float angle = 2 * M_PI * freq_hz * secs + phase;
        auto angle_rads = angle * 3.14159274 / 180;
        // auto sin_312 = sinf(angle_rads);
        auto sin = sinf(angle_rads);
        float sample = (float)amplitude * sin;
        secs += 1 / sample_rate_hz;
        return sample;
    }

private:
    float sample_rate_hz;
    float freq_hz;
    float amplitude;
    float phase;
    float secs;
};


class TetsFFTBase : public ::testing::Test {
public:
    /* data */
    FFT fft;
    std::vector<SinSignalGenerator> signals_generator;
public:
    TetsFFTBase(/* args */);
    void init(InitFFTParamType fft_parameters, InitOneSignParamType* signals_parameters) {
        fft.init(fft_parameters.window_size, fft_parameters.n_axes, fft_parameters.sample_rate_hz);
        signals_generator.resize(fft_parameters.n_axes);
        for (int i = 0; i < fft_parameters.n_axes; i++) {
            auto signal_parameters = signals_parameters[i];
            signals_generator[i] = SinSignalGenerator(signal_parameters);
        }
    }
    ~TetsFFTBase();
};

TetsFFTBase::TetsFFTBase(/* args */) {
}

TetsFFTBase::~TetsFFTBase() {
}

template <typename T>
struct InitParamType   {
    InitFFTParamType fft_parameters;
    T signals_parameters;
};

struct InitParamOneSignalWithRes {
    InitParamType<InitOneSignParamType> parameters;
    bool result;
};

class TestFFTOneSignalParametrized : public TetsFFTBase, public
                ::testing::WithParamInterface<InitParamOneSignalWithRes> {
};

TEST_P(TestFFTOneSignalParametrized, CheckOnSeveralSignalsWindow) {
    auto parameters = GetParam();
    InitParamType init_parameters = parameters.parameters;
    bool result = parameters.result;
    InitFFTParamType fft_parameters = init_parameters.fft_parameters;

    InitOneSignParamType signal_parameters = init_parameters.signals_parameters;
    // expand signal_parameters to multiple axes if needed
    InitOneSignParamType signals_parameters[fft_parameters.n_axes];
    for (int i = 0; i < fft_parameters.n_axes; i++) {
        signals_parameters[i] = signal_parameters;
    }
    init(fft_parameters, signals_parameters);

    float input[fft_parameters.n_axes];
    for (int i = 0; i < fft_parameters.window_size + 10; i++) {
        for (int j = 0; j < fft_parameters.n_axes; j++) {
            input[j] = signals_generator[j].get_next_sample();
        }
        fft.update(input);
    }

    printf("fft resolution: %f\n", fft._resolution_hz);
    if (result) {
        for (int i = 0; i < fft_parameters.n_axes; i++) {
            EXPECT_NEAR(fft.peak_frequencies[i][0],
                        signals_parameters[i].freq_hz, 3 * fft._resolution_hz);
        }
    } else {
        for (int i = 0; i < fft_parameters.n_axes; i++) {
            ASSERT_FALSE(IsBetweenInclusive(fft.peak_frequencies[i][0],
                         signals_parameters[i].freq_hz - fft._resolution_hz,
                         signals_parameters[i].freq_hz + fft._resolution_hz));
        }
    }
    // EXPECT_NEAR(fft.peak_frequencies[0][0], signals_parameters[0].freq_hz, fft._resolution_hz);
    // EXPECT_NEAR(fft.peak_snr[0][0], 1.0f, ABS_ERR);
}

// TEST_P(TestFFTOneSignalParametrized, CheckOnSeveralSignalsUpdateOnline) {
//     auto parameters = GetParam();
//     SinSignalGenerator signal_generator(parameters.sample_rate_hz,
//                                         parameters.freq_hz,
//                                         parameters.amplitude);
//     FFT fft;
//     fft.init(1024, 3, parameters.sample_rate_hz);
//     float input[3] {0, 0, 0};
//     for (int i = 0; i < 1024; i++) {
//         input[0] = signal_generator.get_next_sample();
//         // input[1] = signal_generator.get_next_sample();
//         // input[2] = signal_generator.get_next_sample();
//         fft.update(input);
//     }
//     EXPECT_NEAR(fft.peak_frequencies[0][0], parameters.freq_hz, ABS_ERR);
//     EXPECT_NEAR(fft.peak_snr[0][0], 1.0f, ABS_ERR);
// }

InitParamOneSignalWithRes OneSignalTestParams[7] = {
    {{InitFFTParamType{    .sample_rate_hz = 10, .n_axes = 1,    .window_size = 5},
      InitOneSignParamType{ .sample_rate_hz = 10, .freq_hz = 5, .amplitude = 1}},  true},

    {{InitFFTParamType{    .sample_rate_hz = 300, .n_axes = 1,     .window_size = 600},
      InitOneSignParamType{ .sample_rate_hz = 300, .freq_hz = 100, .amplitude = 1}},  false},

    {{InitFFTParamType{    .sample_rate_hz = 2000, .n_axes = 1,     .window_size = 600},
      InitOneSignParamType{ .sample_rate_hz = 2000, .freq_hz = 100, .amplitude = 1}},  true},

    {{InitFFTParamType{    .sample_rate_hz = 300, .n_axes = 1,     .window_size = 600},
      InitOneSignParamType{ .sample_rate_hz = 1000, .freq_hz = 100, .amplitude = 1}},  false},

    {{InitFFTParamType{    .sample_rate_hz = 200,  .n_axes = 1,     .window_size = 400},
      InitOneSignParamType{ .sample_rate_hz = 1000, .freq_hz = 2000, .amplitude = 1}}, false},

    {{InitFFTParamType{    .sample_rate_hz = 1000, .n_axes = 1,     .window_size = 1024},
      InitOneSignParamType{ .sample_rate_hz = 1000, .freq_hz = 1000, .amplitude = 1}}, false},

    {{InitFFTParamType{    .sample_rate_hz = 1000, .n_axes = 1,     .window_size = 1024},
      InitOneSignParamType{.sample_rate_hz = 10,    .freq_hz = 5,    .amplitude = 1}},  false},
};

unsigned int seed = 0;
class MultiSignalsSinGenerator {
public:
    std::vector<SinSignalGenerator> signals_generator;
private:
    uint8_t max_freq = 500;
    uint8_t min_freq = 100;
    uint8_t n_axes;
    uint16_t sample_rate_hz;

    MultiSignalsSinGenerator(uint8_t n_axes, uint16_t sample_rate_hz) {
        this->n_axes = n_axes;
        this->sample_rate_hz = sample_rate_hz;
        signals_generator.resize(n_axes);
        for (int i = 0; i < n_axes; i++) {
            uint8_t freq_hz = rand_r(&seed) % (max_freq - min_freq) + min_freq;
            signals_generator[i] = SinSignalGenerator(sample_rate_hz, freq_hz, 1);
        }
    }
};


INSTANTIATE_TEST_SUITE_P(TestFFTOneSignalParametrized,
                         TestFFTOneSignalParametrized,
                         ::testing::ValuesIn(
                             OneSignalTestParams)
                         );

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
