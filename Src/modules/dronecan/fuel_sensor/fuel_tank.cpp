/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 * Author: Dmitry Ponomarev <ponomarevda96@gmail.com>
 */

#include "fuel_tank.hpp"
#include <math.h>
#include "main.h"
#include <cstdio>

REGISTER_MODULE(VtolFuelTank)


VtolFuelTank::VtolFuelTank() : Module(10, Protocol::DRONECAN) {
}

void VtolFuelTank::init() {
    if (auto sub_id = raw_command_sub.init(&raw_command_callback); sub_id < 0) {
        health = Status::FATAL_MALFANCTION;
        return;
    }

    mode = Mode::STANDBY;
}

void VtolFuelTank::update_params() {
    if (health >= Status::FATAL_MALFANCTION) {
        return;
    }

    params.tank_id = paramsGetIntegerValue(PARAM_FUEL_TANK_ID);
    params.empty_tank_raw = paramsGetIntegerValue(PARAM_FUEL_TANK_EMPTY);
    params.full_tank_enc_raw = paramsGetIntegerValue(PARAM_FUEL_TANK_FULL);
    params.min_magnitude = paramsGetIntegerValue(PARAM_FUEL_TANK_MIN_MAGNITUDE);
    params.volume_cm3 = paramsGetIntegerValue(PARAM_FUEL_TANK_VOLUME);
    params.ptc_step_1 = paramsGetIntegerValue(PARAM_FUEL_TANK_STEP_1);
    params.ptc_step_2 = paramsGetIntegerValue(PARAM_FUEL_TANK_STEP_2);
    params.ptc_step_3 = paramsGetIntegerValue(PARAM_FUEL_TANK_STEP_3);
    params.ptc_step_4 = paramsGetIntegerValue(PARAM_FUEL_TANK_STEP_4);
    params.ptc_step_5 = paramsGetIntegerValue(PARAM_FUEL_TANK_STEP_5);
    params.ptc_step_6 = paramsGetIntegerValue(PARAM_FUEL_TANK_STEP_6);
    params.ptc_step_7 = paramsGetIntegerValue(PARAM_FUEL_TANK_STEP_7);
    params.ptc_step_8 = paramsGetIntegerValue(PARAM_FUEL_TANK_STEP_8);

    if (params.empty_tank_raw == params.full_tank_enc_raw) {
        params.empty_tank_raw = 0;
        params.full_tank_enc_raw = 3000;
    }

    if (params.full_tank_enc_raw > params.empty_tank_raw) {
        state.min_value = params.empty_tank_raw;
        state.max_value = params.full_tank_enc_raw;
    } else {
        state.min_value = params.full_tank_enc_raw;
        state.max_value = params.empty_tank_raw;
    }

    state.range = params.full_tank_enc_raw - params.empty_tank_raw;
}

uint8_t VtolFuelTank::check_fuel_step(uint16_t raw_pct) {
    if (raw_pct < 460) {
        return 1;
    }
    if (raw_pct < 760) {
        return 2;
    }
    if (raw_pct < 1000) {
        return 3;
    }
    if (raw_pct < 1300) {
        return 4;
    }
    if (raw_pct < 1500) {
        return 5;
    }
    if (raw_pct < 1700) {
        return 6;
    }
    if (raw_pct < 1900) {
        return 7;
    }
    return 8;
}

void VtolFuelTank::spin_once() {
    uint32_t crnt_time_ms = HAL_GetTick();

    // FATAL_MALFANCTION refers to a critical internal error.
    // There is no hope and there is nothing we can do, so let's skip the execution.
    if (health >= Status::FATAL_MALFANCTION) {
        return;
    }

    // MINOR_FAILURE refers to a not successfull reading from as5600.
    // Loosing data is not good, but it may happen. We don't need high frequently readings anyway.
    // Measurements with low magnitude are skipped because they are noisy.
    state.raw_angle = (float)CircuitPeriphery::fuel_state();
    if (state.raw_angle < 100 || state.raw_angle > 2200) {
        if (health == Status::OK) {
            health = Status::MINOR_FAILURE;
        }
        return;
    }

    // // Once reading is done, let's clear all error flags and process the data
    health = Status::OK;
    measurement_deadline = crnt_time_ms + MEASUREMENT_TTL;

    // uint8_t window_size = arm_deadline_ms < crnt_time_ms ? FILTER_SMOOTH : FILTER_FAST;
    // movingAverage(&state.filtered_angle, state.raw_angle, window_size);
    // state.filtered_angle = std::clamp(state.filtered_angle, state.min_value, state.max_value);
    state.filtered_angle = state.raw_angle;
    state.step = check_fuel_step(state.raw_angle);
    switch (state.step) {
    case 1:
        state.available_fuel_pct = params.ptc_step_1;
        break;
    case 2:
        state.available_fuel_pct = params.ptc_step_2;
        break;
    case 3:
        state.available_fuel_pct = params.ptc_step_3;
        break;
    case 4:
        state.available_fuel_pct = params.ptc_step_4;
        break;
    case 5:
        state.available_fuel_pct = params.ptc_step_5;
        break;
    case 6:
        state.available_fuel_pct = params.ptc_step_6;
        break;
    case 7:
        state.available_fuel_pct = params.ptc_step_7;
        break;
    case 8:
        state.available_fuel_pct = params.ptc_step_8;
        break;
    default:
        break;
    }
    state.available_fuel_cm3 = state.available_fuel_pct * params.volume_cm3 * 0.01f;

    // Publish data with required publish rate
    if (crnt_time_ms >= next_publish_time_ms) {
        publish_dronecan_fuel_tank();
        next_publish_time_ms = crnt_time_ms + PUBLISH_PERIOD;
    }
}

void VtolFuelTank::publish_dronecan_fuel_tank() {
    FuelTankStatus_t tank_info{
        .reserved = 0,
        .available_fuel_volume_percent = state.available_fuel_pct,
        .available_fuel_volume_cm3 = state.available_fuel_cm3,
        .fuel_consumption_rate_cm3pm = (float)state.step,
        .fuel_temperature = state.raw_angle,
        .fuel_tank_id = params.tank_id,
    };

    dronecan_equipment_ice_fuel_tank_status_publish(&tank_info, &transfer_id);
    transfer_id++;
}

void VtolFuelTank::raw_command_callback(const RawCommand_t& msg) {
    if (msg.size == 0) {
        return;
    }

    bool is_vehicle_armed = false;
    for (uint8_t i = 0; i < msg.size; i++) {
        if (msg.raw_cmd[i] >= 0) {
            is_vehicle_armed = true;
            break;
        }
    }

    arm_deadline_ms = is_vehicle_armed ? HAL_GetTick() + CMD_TTL_MS : 0;
}
