/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Dmitry Ponomarev <ponomarevda96@gmail.com>
 */

#include "dronecan_module.hpp"
#include <algorithm>
#include "params.hpp"
#include "dronecan.h"
#include "peripheral/adc/circuit_periphery.hpp"

REGISTER_MODULE(DronecanModule)

bool DronecanModule::node_id_duplicated = false;
int8_t DronecanModule::node_id = 0;

DronecanModule::DronecanModule() : Module(0, Protocol::DRONECAN) {
}

void DronecanModule::init() {
    auto node_name_param_idx = static_cast<ParamIndex_t>(IntParamsIndexes::INTEGER_PARAMS_AMOUNT);
    const auto current_node_name = (const char*)paramsGetStringValue(node_name_param_idx);
    if(strlen(current_node_name) == 0) {
        const auto& [board_name, name_length] = CircuitPeriphery::get_board_name();
        paramsSetStringValue(node_name_param_idx, name_length, (const uint8_t*)board_name);
    }
    uavcanSetNodeName((const char*)paramsGetStringValue(node_name_param_idx));

    int param_node_id_value = paramsGetIntegerValue(IntParamsIndexes::PARAM_UAVCAN_NODE_ID);
    node_id = std::clamp(param_node_id_value, 1, 126);
    int8_t res = uavcanInitApplication(node_id);

    health = (res >= 0) ? Status::OK : Status::FATAL_MALFANCTION;
    mode = Mode::STANDBY;
    uavcanSubscribe(UAVCAN_EXPAND(UAVCAN_PROTOCOL_NODE_STATUS), *node_status_callback);
}

void DronecanModule::spin_once() {
    uavcanSetNodeHealth(static_cast<NodeStatusHealth_t>(ModuleManager::get_global_status()));
    auto global_mode = (uint8_t)(ModuleManager::get_global_mode());
    if (global_mode > 0 ) {
        global_mode -= 1;
    }
    if (node_id_duplicated) {
        health = Status::FATAL_MALFANCTION;
        node_id_duplicated = false;
    }
    uavcanSetNodeStatusMode(static_cast<NodeStatusMode_t>(global_mode));
    uavcanSetVendorSpecificStatusCode(ModuleManager::get_vssc());
    uavcanSpinOnce();
}

void DronecanModule::node_status_callback(CanardRxTransfer * transfer) {
    if (transfer->source_node_id == node_id) {
        node_id_duplicated = true;
        return;
    }
}
