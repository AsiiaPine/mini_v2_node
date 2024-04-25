/***
 * Copyright (C) 2024 Anastasiia Stepanova  <asiiapine@gmail.com>
 *  Distributed under the terms of the GPL v3 license, available in the file LICENSE.
***/ 

#ifndef SRC_MODULES_FORCE_MODULE_HPP_
#define SRC_MODULES_FORCE_MODULE_HPP_

#include "periphery/led/led.hpp"
#include "uavcan/equipment/actuator/Status.h"
#include "periphery/adc/circuit_periphery.hpp"
#include "params.hpp"
#include "logger.hpp"

enum class ModuleStatus: uint8_t {
    MODULE_OK        = 0,   // The module is functioning properly
    MODULE_WARN      = 1,   // The module encountered a minor failure
    MODULE_ERROR     = 2,   // The module encountered a major failure
    MODULE_CRITICAL  = 3,   // The module suffered a fatal malfunction
};

class ForceModule {
public:
    static ForceModule &get_instance();
    void spin_once();

    ModuleStatus status;
    static bool instance_initialized;

private:
    static ForceModule instance;
    void update_params();
    ActuatorStatus_t force_status = {};
    float force1_scaler;
    float force2_scaler;

    int16_t offset_1;
    int16_t offset_2;
    bool send_raw;
    bool mes_indx;
    float coeff;
    uint8_t transfer_id  = 0;
    uint32_t crnt_time_ms;

    static Logger logger;

    int8_t init();
    ForceModule(){}
    
    ForceModule(const ForceModule &other) = delete;
    void operator=(const ForceModule &) = delete;
};

#endif  // SRC_MODULES_FORCE_MODULE_HPP_
