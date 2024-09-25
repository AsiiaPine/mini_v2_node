/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Dmitry Ponomarev <ponomarevda96@gmail.com>
 */

#include "imu.hpp"
#include "params.hpp"
#include "peripheral/spi/spi.hpp"

REGISTER_MODULE(ImuModule)

void ImuModule::init() {
    initialized = imu.initialize();
    mode = Module::Mode::STANDBY;
    fft.init(400, 3, 1000.0/period_ms);
    // imu.real_accel_and_gyro();
}

void ImuModule::update_params() {
    enabled = static_cast<bool>(paramsGetIntegerValue(PARAM_IMU_ENABLE));
    health = (!enabled || initialized) ? Module::Status::OK : Module::Status::MAJOR_FAILURE;
    if (enabled) {
        if (!initialized) {
            initialized = imu.initialize();
        }
        mode = initialized ? Mode::STANDBY : Mode::INITIALIZATION;
    }
}

void ImuModule::spin_once() {
    if (!enabled || !initialized) {
        return;
    }

    std::array<int16_t, 3> mag_raw;
    if (imu.read_magnetometer(&mag_raw) >= 0) {
        mag.publish();
    }

    bool updated{false};
    // auto status = 0;

    // auto status = imu.read_accelerometer(Mpu9250::acceleration);
    std::array<int16_t, 3> accel_raw = {0, 0, 0};
    std::array<int16_t, 3> gyro_raw = {0, 0, 0};
    // imu.real_accel_and_gyro(&accel_raw, &gyro_raw);

    // std::array<int16_t, 3> gyro_raw = *Mpu9250::gyroscope;
    // if (status >= 0) {
    static std::vector<float> gyro;
    if (imu.read_gyroscope(&gyro_raw) >= 0) {
        gyro = {raw_gyro_to_rad_per_second(gyro_raw[0]),
                raw_gyro_to_rad_per_second(gyro_raw[1]),
                raw_gyro_to_rad_per_second(gyro_raw[2])};
        pub.msg.rate_gyro_latest[0] = gyro[0];
        pub.msg.rate_gyro_latest[1] = gyro[1];
        pub.msg.rate_gyro_latest[2] = gyro[2];
        updated = true;
    }

    auto status = imu.read_accelerometer(&accel_raw);
    if (status >= 0) {
        static real_t accel[3] = {
                raw_accel_to_meter_per_square_second(accel_raw[0]),
                raw_accel_to_meter_per_square_second(accel_raw[1]),
                raw_accel_to_meter_per_square_second(accel_raw[2])};
        pub.msg.accelerometer_latest[0] = accel[0];
        pub.msg.accelerometer_latest[1] = accel[1];
        pub.msg.accelerometer_latest[2] = accel[2];
        fft.update(accel);
        updated = true;
    }

    static uint32_t prev_time;
    if (prev_time + 100 < HAL_GetTick()) {
        char buffer[90];
        if (updated) {
            pub.msg.timestamp = HAL_GetTick() * 1000;
            pub.publish();
        }
        imu.get_tr();
        prev_time = HAL_GetTick();
        snprintf(buffer, sizeof(buffer), "peak freq: %f %f %f, peak SNR: %f %f %f",
                fft.peak_frequencies[0][0],
                fft.peak_frequencies[1][0],
                fft.peak_frequencies[2][0],
                fft.peak_snr[0][0],
                fft.peak_snr[1][0],
                fft.peak_snr[2][0]);
        logger.log_debug(buffer);
    }
}
