/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Dmitry Ponomarev <ponomarevda96@gmail.com>
 */

#include "arming.hpp"
#include <storage.h>
#include "can_driver.h"
#include "common/algorithms.hpp"
#include <params.hpp>
#include <logger.hpp>

Logger logger = Logger("ArmingModule");


bool ArmingModule::is_armed = false;
uint32_t ArmingModule::arm_start_time = 0;

REGISTER_MODULE(ArmingModule)

void ArmingModule::init() {
    health = Status::OK;
    prev_eng_time = paramsGetIntegerValue(IntParamsIndexes::PARAM_STATS_ENG_TIME);
    mode = Module::Mode::STANDY;
}

void ArmingModule::spin_once() {
    global_mode = ModuleManager::get_global_mode();
    if (global_mode == Mode::ENGAGED && !is_armed) {
        is_armed = true;
        arm_start_time = HAL_GetTick();
        return;
    }
    if (is_armed && global_mode != Mode::ENGAGED) {
        is_armed = false;
        auto cur_eng_time = (HAL_GetTick() - arm_start_time) / 1000;
        prev_eng_time = cur_eng_time + prev_eng_time;
        paramsSetIntegerValue(IntParamsIndexes::PARAM_STATS_ENG_TIME, int(prev_eng_time));
        paramsSave();
    }
}
