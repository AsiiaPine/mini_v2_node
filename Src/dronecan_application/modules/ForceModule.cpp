/***
 * Copyright (C) 2024 Anastasiia Stepanova  <asiiapine@gmail.com>
 *  Distributed under the terms of the GPL v3 license, available in the file
 *LICENSE.
 ***/

#include "ForceModule.hpp"

ForceModule ForceModule::instance = ForceModule();
bool ForceModule::instance_initialized = false;
Logger ForceModule::logger = Logger("ForceModule");

ForceModule& ForceModule::get_instance() {
    if (!instance_initialized) {
        if (instance.init() != 0) {
            logger.log_debug("ADC init error");
        } else {
            instance_initialized = true;
        }
    }
    return instance;
}

int8_t ForceModule::init() {
    status = ModuleStatus::MODULE_OK;
    return CircuitPeriphery::init();
}

void ForceModule::update_params() {
    status = ModuleStatus::MODULE_OK;
    static uint32_t next_upd_ms = 0;
    if (HAL_GetTick() < next_upd_ms) {
        return;
    }
    coeff = paramsGetIntegerValue(IntParamsIndexes::PARAM_FORCE_COEFFICIENT) / 100.f;
    mes_indx = paramsGetIntegerValue(IntParamsIndexes::PARAM_FORCE_MES_INDX);
    send_raw = paramsGetIntegerValue(IntParamsIndexes::PARAM_FORCE_SEND_RAW);
    auto param_1 =
        paramsGetIntegerValue(IntParamsIndexes::PARAM_FORCE_SCALER_1);
    auto param_2 =
        paramsGetIntegerValue(IntParamsIndexes::PARAM_FORCE_SCALER_2);
    offset_1 = paramsGetIntegerValue(IntParamsIndexes::PARAM_FORCE_OFFSET_1);
    offset_2 = paramsGetIntegerValue(IntParamsIndexes::PARAM_FORCE_OFFSET_2);

    force1_scaler = param_1 / 100.0f;
    force2_scaler = param_2 / 100.0f;
    next_upd_ms = crnt_time_ms + 1000;
}

void ForceModule::spin_once() {
    static uint32_t next_pub_ms = 1000;
    crnt_time_ms = HAL_GetTick();
    auto val_1 = CircuitPeriphery::force_1();
    auto val_2 = CircuitPeriphery::force_2();

    if (send_raw && (crnt_time_ms > next_pub_ms)) {
        force_status = {
            .actuator_id = 0,
            .position = (val_1 - offset_1) * force1_scaler,
            .force = val_1*1.0f,
            .power_rating_pct = (uint8_t)(100 * (val_1 * 1.0f / 4095.0f))};

        dronecan_equipment_actuator_status_publish(&force_status, &transfer_id);
        transfer_id++;
        force_status = {
            .actuator_id = 1,
            .position = (val_2 - offset_2)* force2_scaler,
            .force = val_2 * 1.0f,
            .power_rating_pct = (uint8_t)(100 * (val_2 / 4095.0f))};
        dronecan_equipment_actuator_status_publish(&force_status, &transfer_id);
        transfer_id++;
        next_pub_ms = HAL_GetTick() + 10;
    }
    update_params();

    if ((mes_indx && (val_1 < val_2)) || (!mes_indx && (val_1 > val_2))) {
        status = ModuleStatus::MODULE_ERROR;
        return;
    }
    static uint32_t real_force_pub_ms = 0;
    auto mes_force = val_1;
    auto ref_force = val_2;
    auto mes_scale = force1_scaler;
    auto ref_scale = force2_scaler;
    auto mes_offset = offset_1;
    auto ref_offset = offset_2;

    uint16_t real_norm;
    if (crnt_time_ms > real_force_pub_ms) {
        if (!mes_indx) {
            mes_force = val_2;
            ref_force = val_1;
            mes_scale = force2_scaler;
            ref_scale = force1_scaler;
            mes_offset = offset_2;
            ref_offset = offset_1;
        }
        real_norm = ((ref_force - ref_offset) * ref_scale) - mes_scale * (mes_force - mes_offset);
        force_status = {
            .actuator_id = 3,
            .position = real_norm * 1.0f,
            .force = real_norm * coeff};
        dronecan_equipment_actuator_status_publish(&force_status, &transfer_id);
        transfer_id++;
        real_force_pub_ms = HAL_GetTick() + 100;
    }
}
