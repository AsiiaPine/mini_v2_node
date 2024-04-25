/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Dmitry Ponomarev <ponomarevda96@gmail.com>
 */

#include "application.hpp"
#include "dronecan.h"
#include "main.h"
#include "params.hpp"
#include "periphery/led/led.hpp"
#include "periphery/iwdg/iwdg.hpp"
#include "modules/ForceModule.hpp"


void application_entry_point() {
    paramsInit((ParamIndex_t)IntParamsIndexes::INTEGER_PARAMS_AMOUNT, NUM_OF_STR_PARAMS, -1, 1);
    // paramsInitRedundantPage(255);
    // paramsChooseRom();
    paramsLoad();

    auto node_id = paramsGetIntegerValue(IntParamsIndexes::PARAM_UAVCAN_NODE_ID);

    const auto node_name = "co.raccoonlab.mini";
    auto node_name_param_idx = static_cast<ParamIndex_t>(IntParamsIndexes::INTEGER_PARAMS_AMOUNT);
    paramsSetStringValue(node_name_param_idx, 19, (const uint8_t*)node_name);
    uavcanSetNodeName(node_name);

    LedPeriphery::reset();

    uavcanInitApplication(node_id);

    ForceModule& force_module = ForceModule::get_instance();
    LedColor color = LedColor::BLUE_COLOR;

    if (!force_module.instance_initialized) {
        color = LedColor::RED_COLOR;
    }
    while(true) {
        LedPeriphery::toggle(color);
        force_module.spin_once();
        if (force_module.status != ModuleStatus::MODULE_OK) {
            color = LedColor::RED_COLOR;
        } else {
            color = LedColor::BLUE_COLOR;
        }
        uavcanSetNodeHealth((NodeStatusHealth_t)force_module.status);
        uavcanSpinOnce();

        WatchdogPeriphery::refresh();
    }
}
