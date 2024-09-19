/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Dmitry Ponomarev <ponomarevda96@gmail.com>
 */

#include "mpu9250.hpp"
#include <cstddef>
#include "peripheral/spi/spi.hpp"
#include "main.h"

// Register Map for Gyroscope and Accelerometer
enum class Mpu9250Resgiter : uint8_t {
    ACCEL_XOUT_H = 0x3B,
    GYRO_XOUT_H = 0x43,
    WHO_AM_I = 0x75,
};

// Register Map for Magnetometer (AK8963)
enum class MagnetometerResgiter : uint8_t {
    REG_MAG_XOUT_L = 0x03,
};

constexpr auto MPU9250_WHO_AM_I_ID = std::byte(0x71);   // REG_WHO_AM_I expected value

bool Mpu9250::initialize() {
    // HAL::SPI::set_callback(real_accel_and_gyro);
    // HAL::SPI::setup_callback();
    std::byte who_am_i_value{0};
    auto reg = std::byte(Mpu9250Resgiter::WHO_AM_I);
    HAL::SPI::read_register(reg, &who_am_i_value);
    // uint8_t i = 0;
    // while (who_am_i_value != MPU9250_WHO_AM_I_ID && i < 10) {
    //     i++;
    //     HAL_Delay(10);
    // }
    initialized = who_am_i_value == MPU9250_WHO_AM_I_ID;
    return initialized;
}

void Mpu9250::get_tr() {
    transaction_ctnr = HAL::get_transaction_cntr();
}

int8_t Mpu9250::read_accelerometer(std::array<int16_t, 3>* accel) const {
    std::array<std::byte, 6> buffer;
    auto reg = std::byte(Mpu9250Resgiter::ACCEL_XOUT_H);
    if (auto res = HAL::SPI::read_registers(reg, buffer.data(), 6); res < 0) {
        return res;
    }

    (*accel)[0] = (uint16_t)buffer[0] << 8 | (uint16_t)buffer[1];
    (*accel)[1] = (uint16_t)buffer[2] << 8 | (uint16_t)buffer[3];
    (*accel)[2] = (uint16_t)buffer[4] << 8 | (uint16_t)buffer[5];

    return 0;
}

int8_t Mpu9250::read_gyroscope(std::array<int16_t, 3>* gyro) const {
    std::array<std::byte, 6> buffer;
    auto reg = std::byte(Mpu9250Resgiter::GYRO_XOUT_H);
    if (auto res = HAL::SPI::read_registers(reg, buffer.data(), buffer.size()); res < 0) {
        return res;
    }

    (*gyro)[0] = (uint16_t)buffer[0] << 8 | (uint16_t)buffer[1];
    (*gyro)[1] = (uint16_t)buffer[2] << 8 | (uint16_t)buffer[3];
    (*gyro)[2] = (uint16_t)buffer[4] << 8 | (uint16_t)buffer[5];

    return 0;
}

void Mpu9250::real_accel_and_gyro(std::array<int16_t, 3>* accel, std::array<int16_t, 3>* gyro)
const {
    static bool is_accel_waiting = true;
    if (is_accel_waiting) {
        read_gyroscope(gyro);
        is_accel_waiting = false;
    } else {
        read_accelerometer(accel);
        is_accel_waiting = true;
    }
    // static uint32_t last_tick = 0;
    // if (HAL_GetTick() < last_tick + 100) {
    //     return;
    // }
    // std::array<std::byte, 14> buffer;
    // auto reg = std::byte(Mpu9250Resgiter::ACCEL_XOUT_H);
    // if (auto res = HAL::SPI::read_registers(reg, buffer.data(), 14); res < 0) {
    //     return;
    // }
    // (*acceleration)[0] = (uint16_t)buffer[0] << 8 | (uint16_t)buffer[1];
    // (*acceleration)[1] = (uint16_t)buffer[2] << 8 | (uint16_t)buffer[3];
    // (*acceleration)[2] = (uint16_t)buffer[4] << 8 | (uint16_t)buffer[5];
    // // (*temp)[0] = (uint16_t)buffer[6] << 8 | (uint16_t)buffer[7];
    // // (*temp)[1] = (uint16_t)buffer[8] << 8 | (uint16_t)buffer[9];

    // (*gyroscope)[0] = (uint16_t)buffer[10] << 8 | (uint16_t)buffer[11];
    // (*gyroscope)[1] = (uint16_t)buffer[12] << 8 | (uint16_t)buffer[13];
    // (*gyroscope)[2] = (uint16_t)buffer[14] << 8 | (uint16_t)buffer[15];
}

int8_t Mpu9250::read_magnetometer(std::array<int16_t, 3>* mag) const {
    (void)mag;
    return -1;  // not supported yet
}
