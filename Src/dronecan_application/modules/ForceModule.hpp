/***
 * Copyright (C) 2024 Anastasiia Stepanova  <asiiapine@gmail.com>
 *  Distributed under the terms of the GPL v3 license, available in the file LICENSE.
***/ 

#ifndef SRC_MODULES_FORCE_MODULE_HPP_
#define SRC_MODULES_FORCE_MODULE_HPP_

#include "periphery/led/led.hpp"
#include "uavcan/equipment/actuator/Status.h"
#include "periphery/adc/circuit_periphery.hpp"
#include "logger.hpp"

class ForceModule {
public:
    static ForceModule &get_instance();
    void spin_once();

    static bool instance_initialized;

private:
    static ForceModule instance;

    ActuatorStatus_t force_status = {};

    uint8_t transfer_id  = 0;
    uint32_t crnt_time_ms;

    static Logger logger;

    int8_t init();
    ForceModule(){}
    
    ForceModule(const ForceModule &other) = delete;
    void operator=(const ForceModule &) = delete;
};

#endif  // SRC_MODULES_FORCE_MODULE_HPP_
