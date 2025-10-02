#include "LedPullDown.hpp"
#include "logger.hpp"

LedPullModule LedPullModule::instance = LedPullModule();
bool LedPullModule::instance_initialized = false;
bool LedPullModule::command_on = false;
CommandType LedPullModule::pwm_cmd_type = CommandType::RAW_COMMAND;
uint32_t LedPullModule::next_turn_off_ms = 0;
uint32_t LedPullModule::ttl_cmd = 1000;
static uint32_t start_time = 0;
LedConfiguration LedPullModule::led_configuration = LedConfiguration::BOTH;
LedState LedPullModule::led_state = LedState::OFF;
int LedPullModule::position_channel = -1;
int LedPullModule::switch_channel = -1;

DronecanLogger logger = DronecanLogger("led_pull");

LedPullModule& LedPullModule::get_instance() {
    if (!instance_initialized) {
        if (instance.init() != 0) {
        } else {
            instance_initialized = true;
        }
    }

    return instance;
}

int8_t LedPullModule::init() {
    update_params();
    command_on = false;
    start_time = HAL_GetTick();
    return 0;
}

void LedPullModule::spin_once() {
    static uint32_t last_blink = 0;
    if (HAL_GetTick() - start_time < 4000) {
        if (HAL_GetTick() - last_blink < 1000) {
            return;
        }
        last_blink = HAL_GetTick();
        if (led_state == LedState::ON) {
            led_off();
        } else {
            led_on();
        }
        return;
    }
    if (!is_subscribed) {
        uavcanSubscribe(UAVCAN_EQUIPMENT_ESC_RAWCOMMAND,            raw_command_callback);
        uavcanSubscribe(UAVCAN_EQUIPMENT_ACTUATOR_ARRAY_COMMAND,    array_command_callback);
        is_subscribed = true;
    }
    if (instance_initialized) {
        status = NodeStatusHealth_t::NODE_STATUS_HEALTH_OK;
    }
    if (led_state == LedState::ON) {
        if (HAL_GetTick() > next_turn_off_ms) {
            led_off();
        }
    }
    update_params();
}
// 3.3 -- on
void LedPullModule::led_on() {
    switch (led_configuration) {
        case LedConfiguration::LEFT:
            HAL_GPIO_WritePin(PWMA_OUTPUT_GPIO_Port, PWMA_OUTPUT_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(PWMB_OUTPUT_GPIO_Port, PWMB_OUTPUT_Pin, GPIO_PIN_RESET);
            break;
        case LedConfiguration::RIGHT:
            HAL_GPIO_WritePin(PWMA_OUTPUT_GPIO_Port, PWMA_OUTPUT_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(PWMB_OUTPUT_GPIO_Port, PWMB_OUTPUT_Pin, GPIO_PIN_SET);
            break;
        default:
            HAL_GPIO_WritePin(PWMA_OUTPUT_GPIO_Port, PWMA_OUTPUT_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(PWMB_OUTPUT_GPIO_Port, PWMB_OUTPUT_Pin, GPIO_PIN_SET);
    }
    command_on = true;
    led_state = LedState::ON;
}

void LedPullModule::led_off() {
    HAL_GPIO_WritePin(PWMA_OUTPUT_GPIO_Port, PWMA_OUTPUT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PWMB_OUTPUT_GPIO_Port, PWMB_OUTPUT_Pin, GPIO_PIN_RESET);
    command_on = false;
    led_state = LedState::OFF;
}

void LedPullModule::update_params() {
    static uint32_t next_upd_ms = 0;
    if (HAL_GetTick() < next_upd_ms) {
        return;
    }
    ttl_cmd = paramsGetIntegerValue(IntParamsIndexes::PARAM_LED_PULL_DOWN_TTL_CMD);
    pwm_cmd_type = static_cast<CommandType>(
                paramsGetIntegerValue(IntParamsIndexes::PARAM_LED_PULL_DOWN_PWM_CMD_TYPE));
    next_upd_ms = HAL_GetTick() + 1000;
    switch_channel = paramsGetIntegerValue(IntParamsIndexes::PARAM_LED_PULL_DOWN_SWITCH_CHANNEL);
    position_channel = paramsGetIntegerValue(
                                            IntParamsIndexes::PARAM_LED_PULL_DOWN_POSITION_CHANNEL);
}

void LedPullModule::parse_position_raw_command(CanardRxTransfer* transfer) {
    led_configuration = LedConfiguration::BOTH;
    int16_t position_cmd;
    if (!dronecan_equipment_esc_raw_command_channel_deserialize(transfer,
                                                        position_channel, &position_cmd)) {
        return;
    }
    if (position_cmd < 2500) {
        led_configuration = LedConfiguration::BOTH;
        return;
    }
    if (position_cmd > 2500 && position_cmd < 5000) {
        led_configuration = LedConfiguration::LEFT;
        return;
    }
    led_configuration = LedConfiguration::RIGHT;
    return;
}

void LedPullModule::raw_command_callback(CanardRxTransfer* transfer) {
    if (pwm_cmd_type != CommandType::RAW_COMMAND) {
        return;
    }

    if (switch_channel < 0) {
        return;
    }

    parse_position_raw_command(transfer);

    int16_t on_cmd;
    if (!dronecan_equipment_esc_raw_command_channel_deserialize(transfer,
                                                        switch_channel, &on_cmd)) {
        return;
    }

    if (on_cmd > 4000) {
        led_state = LedState::ON;
        next_turn_off_ms = HAL_GetTick() + ttl_cmd;
        led_on();
    } else {
        led_state = LedState::OFF;
        led_off();
    }
}

void LedPullModule::array_command_callback(CanardRxTransfer* transfer) {
    if (pwm_cmd_type != CommandType::ARRAY_COMMAND) {
        return;
    }
    if (switch_channel < 0) {
        return;
    }

    ArrayCommand_t command;
    int8_t ch_num = dronecan_equipment_actuator_arraycommand_deserialize(transfer, &command);
    if (ch_num <= 0) {
        return;
    }
    for (uint8_t j = 0; j < ch_num; j++) {
        if (command.commads[j].actuator_id == switch_channel ) {
            if (command.commads[j].command_value > 0.5) {
                led_state = LedState::ON;
                next_turn_off_ms = HAL_GetTick() + ttl_cmd;
                continue;
            }
            led_state = LedState::OFF;
            continue;
        }
        if (command.commads[j].actuator_id == position_channel) {
            if (command.commads[j].command_value < 0.33) {
                led_configuration = LedConfiguration::BOTH;
                continue;
            }
            if (command.commads[j].command_value > 0.66) {
                led_configuration = LedConfiguration::RIGHT;
                continue;
            }
            led_configuration = LedConfiguration::LEFT;
            continue;
        }
    }
    if (led_state == LedState::ON) {
        led_on();
    } else {
        led_off();
    }
}
