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
unsigned int seed = 0;

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

struct InitFFTParameters   {
    float sample_rate_hz;
    uint8_t n_axes;
    uint16_t window_size;
};

struct InitSignalParameters   {
    float sample_rate_hz = 0;
    float freq_hz = 0;
    float amplitude = 0;
};

struct InitMultiSignalsParameters   {
    uint16_t sample_rate_hz;
    uint8_t n_axes;
    uint8_t n_signals;
};

template <typename T>
struct InitParameters   {
    InitFFTParameters fft_parameters;
    T signals_parameters;
};

struct InitParametersOneSignalWithRes {
    InitParameters<InitSignalParameters> parameters;
    bool result;
};

struct InitParametersMultiSignalsWithRes {
    InitParameters<InitMultiSignalsParameters> parameters;
    bool result;
};

InitParametersOneSignalWithRes ParametrizedInitParams[8] = {
    {{InitFFTParameters{    .sample_rate_hz = 24,   .n_axes   = 1,    .window_size  = 24},
      InitSignalParameters{ .sample_rate_hz = 24,   .freq_hz  = 3,    .amplitude    = 1}}, true},

    {{InitFFTParameters{    .sample_rate_hz = 1000, .n_axes   = 1,    .window_size  = 1000},
      InitSignalParameters{ .sample_rate_hz = 1000, .freq_hz  = 100,  .amplitude    = 10}}, true},

    {{InitFFTParameters{    .sample_rate_hz = 1000, .n_axes   = 1,    .window_size  = 100},
      InitSignalParameters{ .sample_rate_hz = 1000, .freq_hz  = 5,    .amplitude    = 10}}, true},

    {{InitFFTParameters{    .sample_rate_hz = 2000, .n_axes   = 1,    .window_size  = 600},
      InitSignalParameters{ .sample_rate_hz = 2000, .freq_hz  = 100,  .amplitude    = 10}}, true},

    {{InitFFTParameters{    .sample_rate_hz = 2000, .n_axes   = 1,    .window_size  = 2000},
      InitSignalParameters{ .sample_rate_hz = 2000, .freq_hz  = 100,  .amplitude    = 10}}, true},

    {{InitFFTParameters{    .sample_rate_hz = 1024, .n_axes   = 1,    .window_size  = 1024},
      InitSignalParameters{ .sample_rate_hz = 1024, .freq_hz  = 100,  .amplitude    = 1}}, true},

    {{InitFFTParameters{    .sample_rate_hz = 256, .n_axes   = 1,    .window_size  = 256},
      InitSignalParameters{ .sample_rate_hz = 256, .freq_hz  = 100,  .amplitude    = 1}}, true},

    {{InitFFTParameters{    .sample_rate_hz = 512, .n_axes   = 1,    .window_size  = 512},
      InitSignalParameters{ .sample_rate_hz = 512, .freq_hz  = 100,  .amplitude    = 1}}, true},
};

//     {{InitFFTParameters{    .sample_rate_hz = 300,  .n_axes   = 1,    .window_size  = 600},
//       InitSignalParameters{ .sample_rate_hz = 300,  .freq_hz  = 100,  .amplitude    = 1}}, false},

//     {{InitFFTParameters{    .sample_rate_hz = 300,  .n_axes   = 1,    .window_size  = 600},
//       InitSignalParameters{ .sample_rate_hz = 1000, .freq_hz  = 100,  .amplitude    = 1}}, false},

//     {{InitFFTParameters{    .sample_rate_hz = 200,  .n_axes   = 1,    .window_size  = 400},
//       InitSignalParameters{ .sample_rate_hz = 1000, .freq_hz  = 2000, .amplitude    = 1}}, false},

//     {{InitFFTParameters{    .sample_rate_hz = 1000, .n_axes   = 1,    .window_size  = 1024},
//       InitSignalParameters{ .sample_rate_hz = 1000, .freq_hz  = 1000, .amplitude    = 1}}, false},

//     {{InitFFTParameters{    .sample_rate_hz = 1000, .n_axes   = 1,    .window_size  = 1024},
//       InitSignalParameters{.sample_rate_hz  = 10,    .freq_hz = 5,    .amplitude    = 1}}, false},
// };

class SinSignalGenerator {
public:
    SinSignalGenerator(){}
    explicit SinSignalGenerator(InitSignalParameters signal_parameters) {
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
    float freq_hz;

private:
    float sample_rate_hz;
    float amplitude;
    float phase;
    float secs;
};

class MultiSignalsSinGenerator {
public:
    std::vector<std::vector<SinSignalGenerator>> signals_generator;
    std::vector<uint16_t> dominant_freqs;

    void init() {
        signals_generator.resize(n_axes);
        for (int i = 0; i < n_axes; i++) {
            uint16_t max_amplitude = 0;
            for (int j = 0; j < n_signals; j++) {
                uint16_t freq_hz = rand_r(&seed) % (max_freq - min_freq) + min_freq;
                uint16_t amplitude = rand_r(&seed);
                signals_generator[i][j] = SinSignalGenerator(sample_rate_hz, freq_hz, amplitude);
                if (amplitude > max_amplitude) {
                    max_amplitude = amplitude;
                    dominant_freqs[i] = freq_hz;
                }
            }
        }
    }

    MultiSignalsSinGenerator(uint8_t n_signals, uint8_t n_axes, uint16_t sample_rate_hz) {
        this->n_signals = n_signals;
        this->n_axes = n_axes;
        this->sample_rate_hz = sample_rate_hz;
        init();
    }

    explicit MultiSignalsSinGenerator(InitMultiSignalsParameters parameters) {
        this->n_signals = parameters.n_signals;
        this->n_axes = parameters.n_axes;
        this->sample_rate_hz = parameters.sample_rate_hz;
        init();
    }

    std::vector<float> get_next_samples() {
        std::vector<float> sample;
        sample.resize(n_axes);
        for (int i = 0; i < n_axes; i++) {
            for (int j = 0; j < n_signals; j++) {
                sample[i] +=signals_generator[i][j].get_next_sample();
            }
        }
        return sample;
    }

private:
    uint16_t max_freq = 500;
    uint16_t min_freq = 100;
    uint8_t n_axes;
    uint16_t sample_rate_hz;
    uint8_t n_signals;
};

class TestFFTOnSignalParametrized : public testing::TestWithParam<InitParametersOneSignalWithRes> {
public:
    /* data */
    FFT fft;
    std::vector<SinSignalGenerator> signals_generator;
    InitFFTParameters fft_parameters;
    std::vector<InitSignalParameters> signals_parameters;
    bool result;

    void init() {
        fft = FFT();
        fft.init(fft_parameters.window_size, fft_parameters.n_axes, fft_parameters.sample_rate_hz);
        signals_generator.resize(fft_parameters.n_axes);
        for (int i = 0; i < fft_parameters.n_axes; i++) {
            auto signal_parameters = signals_parameters[i];
            signals_generator[i] = SinSignalGenerator(signal_parameters);
        }
    }

    void SetUp() override {
        auto parameters = GetParam();
        InitParameters<InitSignalParameters> init_parameters = parameters.parameters;
        result = parameters.result;
        fft_parameters = init_parameters.fft_parameters;
        // Print the FFT parameters
        std::cout << "FFT Parameters: " << std::endl;
        std::cout << "  Sample Rate (Hz): " << fft_parameters.sample_rate_hz << std::endl;
        std::cout << "  Window Size: " << fft_parameters.window_size << std::endl;
        std::cout << "  Number of Axes: " << static_cast<int>(fft_parameters.n_axes) << std::endl;

        // expand signal_parameters to multiple axes if needed
        signals_parameters.resize(fft_parameters.n_axes);
        for (int i = 0; i < fft_parameters.n_axes; i++) {
            signals_parameters[i] = init_parameters.signals_parameters;
        }

        // Print the signal parameters
        std::cout << "Signal Parameters: " << std::endl;
        std::cout << "  Sample Rate (Hz): " << init_parameters.signals_parameters.sample_rate_hz
                    << std::endl;
        std::cout << "  Frequency (Hz): " << init_parameters.signals_parameters.freq_hz
                    << std::endl;
        std::cout << "  Amplitude: " << init_parameters.signals_parameters.amplitude
                    << std::endl;

        init();
    }
};

TEST_P(TestFFTOnSignalParametrized, CheckOnWindow) {
    float input[fft_parameters.n_axes];
    for (int i = 0; i < fft_parameters.window_size + 10; i++) {
        for (int j = 0; j < fft_parameters.n_axes; j++) {
            input[j] = signals_generator[j].get_next_sample();
        }
        fft.update(input);
    }
    for (int j = 0; j < fft_parameters.n_axes; j++) {
        printf("AXIS: %d\n", j);
        for (int i = 0; i < MAX_NUM_PEAKS; i++) {
            printf("%d:\n freq:\t%f\n", i,
                        fft.peak_frequencies[j][i]);
            printf(" snr:\t%f\n", fft.peak_snr[j][i]);
        }
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
}

// TEST_P(TestFFTOnSignalParametrized, CheckOnFewWindows) {
//     float input[fft_parameters.n_axes];
//     auto n_updates = 10 * fft_parameters.window_size;
//     // auto n_updates = (2 + rand_r(&seed) % 10) * fft_parameters.window_size;
//     for (int i = 0; i < n_updates + 10; i++) {
//         for (int j = 0; j < fft_parameters.n_axes; j++) {
//             input[j] = signals_generator[j].get_next_sample();
//         }
//         fft.update(input);
//     }
//     for (int j = 0; j < fft_parameters.n_axes; j++) {
//         printf("AXIS: %d\n", j);
//         for (int i = 0; i < MAX_NUM_PEAKS; i++) {
//             printf("%d:\n freq:\t%f\n", i,
//                         fft.peak_frequencies[j][i]);
//             printf(" snr:\t%f\n", fft.peak_snr[j][i]);
//         }
//         printf("real freq %d: %f\n", j, signals_generator[j].freq_hz);
//     }

//     printf("fft resolution: %f\n", fft._resolution_hz);
//     if (result) {
//         for (int i = 0; i < fft_parameters.n_axes; i++) {
//             EXPECT_NEAR(fft.peak_frequencies[i][0],
//                         signals_parameters[i].freq_hz, 3 * fft._resolution_hz);
//         }
//     } else {
//         for (int i = 0; i < fft_parameters.n_axes; i++) {
//             ASSERT_FALSE(IsBetweenInclusive(fft.peak_frequencies[i][0],
//                          signals_parameters[i].freq_hz - fft._resolution_hz,
//                          signals_parameters[i].freq_hz + fft._resolution_hz));
//         }
//     }
// }

INSTANTIATE_TEST_SUITE_P(TestFFTOnSignalParametrized,
                         TestFFTOnSignalParametrized,
                         ::testing::ValuesIn(
                             ParametrizedInitParams)
                         );


class TestFFTOnMultiSignalsParametrized : public ::testing::Test,
                ::testing::WithParamInterface<InitParametersMultiSignalsWithRes> {
public:
    /* data */
    FFT fft;
    MultiSignalsSinGenerator signals_generator;
    bool result;
    InitFFTParameters fft_parameters;
    InitMultiSignalsParameters multi_signals_parameters;
    // std::vector<float> input;
    // std::vector<float> output;
    // void init() {
    //     input.resize(fft_parameters.n_axes);
    //     output.resize(fft_parameters.n_axes);
    //     for (int i = 0; i < fft_parameters.n_axes; i++) {
    //         input[i] = 0;
    //         output[i] = 0;
    //     }
    // }
    void SetUp() override {
        auto parameters = GetParam();
        InitParameters<InitMultiSignalsParameters> init_parameters = parameters.parameters;
        result = parameters.result;
        fft_parameters = init_parameters.fft_parameters;
        multi_signals_parameters = init_parameters.signals_parameters;
        signals_generator = MultiSignalsSinGenerator(multi_signals_parameters);
    }
};

// TEST_P(TestFFTOnMultiSignalsParametrized, CheckOnSeveralSignalsUpdateOnline) {
// }

// TEST_P(TestFFTOnSignalParametrized, CheckOnSeveralSignalsUpdateOnline) {
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


int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
