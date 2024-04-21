/***
 * Copyright (C) 2024 Anastasiia Stepanova  <asiiapine@gmail.com>
 *  Distributed under the terms of the GPL v3 license, available in the file LICENSE.
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
    return CircuitPeriphery::init();
}

void ForceModule::spin_once() {
    static uint32_t next_pub_ms = 1000;
    crnt_time_ms = HAL_GetTick();
    auto force_1 = CircuitPeriphery::force_1();
    auto force_2 = CircuitPeriphery::force_2();
    if (crnt_time_ms > next_pub_ms) {
        force_status = {.actuator_id = 0, .position = 0, .power_rating_pct = (uint8_t)(255 * (force_1 * 1.0f/ 4095.0f))};
        // force_status = {.actuator_id = 0, .position = 0, .power_rating_pct = (uint8_t)(100 * (force_1 / 4095.0f))};

        dronecan_equipment_actuator_status_publish(&force_status, &transfer_id);
        transfer_id++;
        force_status = {.actuator_id = 1, .position = 0, .power_rating_pct = (uint8_t)(255 * (force_2 / 4095.0f))};
        dronecan_equipment_actuator_status_publish(&force_status, &transfer_id);
        transfer_id++;
        next_pub_ms += 200;
    }
    char buffer[60];
    if (crnt_time_ms % 1000 == 0) {
        sprintf(buffer, "current: %d, vol_5v %d, vol_vin %d, f1: %d f2: %d", (uint16_t)(CircuitPeriphery::current()),
                                                                            (uint16_t)(CircuitPeriphery::voltage_5v()),
                                                                            (uint16_t)(CircuitPeriphery::voltage_vin()),
                                                                            CircuitPeriphery::force_1(),
                                                                            CircuitPeriphery::force_2());
        logger.log_debug(buffer);
    }
}
