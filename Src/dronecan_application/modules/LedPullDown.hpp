/***
 * Copyright (C) 2024 Anastasiia Stepanova  <asiiapine@gmail.com>
 *  Distributed under the terms of the GPL v3 license, available in the file LICENSE.
***/ 

#ifndef SRC_MODULES_LED_PULL_DOWN_HPP_
#define SRC_MODULES_LED_PULL_DOWN_HPP_

#include "periphery/led/led.hpp"
#include "uavcan/equipment/actuator/ArrayCommand.h"
#include "uavcan/equipment/esc/RawCommand.h"
#include "periphery/adc/circuit_periphery.hpp"
#include "uavcan/equipment/power/CircuitStatus.h"
#include "params.hpp"
#include "main.h"

enum class CommandType: uint8_t {
    RAW_COMMAND = 0,
    ARRAY_COMMAND = 1,
};

enum class LedConfiguration: uint8_t {
    LEFT = 0,
    RIGHT = 1,
    BOTH = 2,
};

enum class LedState: bool {
    OFF = 0,
    ON = 1,
};

class LedPullModule {
 public:
    static LedPullModule &get_instance();
    void spin_once();

    static bool instance_initialized;
    static bool command_on;
    NodeStatusHealth_t status;
    static void led_on();
    static void led_off();

 private:
    static LedPullModule instance;

    bool is_subscribed = false;
    LedPullModule(){}

    int8_t init();
    void update_params();

    LedPullModule(const LedPullModule &other) = delete;
    void operator=(const LedPullModule &) = delete;

    static void raw_command_callback(CanardRxTransfer* transfer);
    static void array_command_callback(CanardRxTransfer* transfer);
    static void hardpoint_callback(CanardRxTransfer* transfer);
    static void parse_position_raw_command(CanardRxTransfer* transfer);

    static uint32_t ttl_cmd;
    static CommandType pwm_cmd_type;
    static uint32_t next_turn_off_ms;
    static int position_channel;
    static int switch_channel;

    static LedConfiguration led_configuration;
    static LedState led_state;
};

#endif  // SRC_MODULES_LED_PULL_DOWN_HPP_
