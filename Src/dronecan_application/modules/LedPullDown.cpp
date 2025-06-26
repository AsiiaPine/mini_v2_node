#include "LedPullDown.hpp"

LedPullModule LedPullModule::instance = LedPullModule();
bool LedPullModule::instance_initialized = false;
Logger LedPullModule::logger = Logger("LedPullModule");
bool LedPullModule::command_on = false;
int LedPullModule::channel = -1;
CommandType LedPullModule::pwm_cmd_type = CommandType::RAW_COMMAND;
uint32_t LedPullModule::next_turn_off_ms = 0;
uint32_t LedPullModule::ttl_cmd = 1000;
static uint32_t last_message_ms = 0;

LedPullModule& LedPullModule::get_instance() {
    if (!instance_initialized) {
        if (instance.init() != 0) {
            logger.log_debug("LED init error");
        } else {
            instance_initialized = true;
        }
    }

    return instance;
}

int8_t LedPullModule::init() {
    update_params();
    logger.log_info("init");
    command_on = false;

    uavcanSubscribe(UAVCAN_EQUIPMENT_ESC_RAWCOMMAND,            raw_command_callback);
    uavcanSubscribe(UAVCAN_EQUIPMENT_ACTUATOR_ARRAY_COMMAND,    array_command_callback);
    return 0;
}

void LedPullModule::spin_once() {
    if (instance_initialized) {
        status = NodeStatusHealth_t::NODE_STATUS_HEALTH_OK;
    }
    if (command_on) {
        if (HAL_GetTick() > next_turn_off_ms) {
            led_off();
            command_on = false;
        }
    }
    update_params();
}

void LedPullModule::led_on() {
    HAL_GPIO_WritePin(PWMB_OUTPUT_GPIO_Port, PWMB_OUTPUT_Pin, GPIO_PIN_SET);
}

void LedPullModule::led_off() {
    HAL_GPIO_WritePin(PWMB_OUTPUT_GPIO_Port, PWMB_OUTPUT_Pin, GPIO_PIN_RESET);
}

void LedPullModule::update_params() {
    static uint32_t next_upd_ms = 0;
    if (HAL_GetTick() < next_upd_ms) {
        return;
    }
    ttl_cmd = paramsGetIntegerValue(IntParamsIndexes::PARAM_LED_PULL_DOWN_TTL_CMD);
    pwm_cmd_type = static_cast<CommandType>(paramsGetIntegerValue(IntParamsIndexes::PARAM_LED_PULL_DOWN_PWM_CMD_TYPE));
    next_upd_ms = HAL_GetTick() + 1000;
    channel = paramsGetIntegerValue(IntParamsIndexes::PARAM_LED_PULL_DOWN_CHANNEL);
}

void LedPullModule::raw_command_callback(CanardRxTransfer* transfer) {
    if (pwm_cmd_type != CommandType::RAW_COMMAND) {
        return;
    }
    if (channel < 0) {
        return;
    }

    int16_t cmd;
    if (!dronecan_equipment_esc_raw_command_channel_deserialize(transfer, channel, &cmd)) {
        return;
    }

    if (cmd > 4000) {
        command_on = true;
        next_turn_off_ms = HAL_GetTick() + ttl_cmd;
        if (HAL_GetTick() > last_message_ms + ttl_cmd) {
            logger.log_info("on");
            last_message_ms = HAL_GetTick();
        }
        led_on();
    } else {
        command_on = false;
        if (HAL_GetTick() > last_message_ms + ttl_cmd) {
            logger.log_info("off");
            last_message_ms = HAL_GetTick();
        }
        led_off();
    }
}

void LedPullModule::array_command_callback(CanardRxTransfer* transfer) {
    if (pwm_cmd_type != CommandType::ARRAY_COMMAND) {
        return;
    }
    if (channel < 0) {
        return;
    }

    ArrayCommand_t command;
    int8_t ch_num = dronecan_equipment_actuator_arraycommand_deserialize(transfer, &command);
    if (ch_num <= 0) {
        return;
    }
    for (uint8_t j = 0; j < ch_num; j++) {
        if (command.commads[j].actuator_id != channel) {
            continue;
        }
        if (command.commads[j].command_value > 0.5) {
            command_on = true;
            led_on();
            if (HAL_GetTick() > last_message_ms + ttl_cmd) {
                logger.log_info("on");
                last_message_ms = HAL_GetTick();
            }
            next_turn_off_ms = HAL_GetTick() + ttl_cmd;
            break;
        }
        led_off();
        if (HAL_GetTick() > last_message_ms + ttl_cmd) {
            logger.log_info("off");
            last_message_ms = HAL_GetTick();
        }
        command_on = false;
        break;
    }
}
