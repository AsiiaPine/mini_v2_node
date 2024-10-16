/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Dmitry Ponomarev <ponomarevda96@gmail.com>
 */

#ifndef SRC_DRONECAN_APPLICATION_DRONECAN_DRONECAN_MODULE_HPP_
#define SRC_DRONECAN_APPLICATION_DRONECAN_DRONECAN_MODULE_HPP_

#include "module.hpp"
#include "subscriber.hpp"


#ifdef __cplusplus
extern "C" {
#endif

class DronecanModule : public Module {
public:
    DronecanModule();

    void init() override;
    static bool node_id_duplicated;
    static int8_t node_id;
protected:
    void spin_once() override;
    static void node_status_callback(CanardRxTransfer *transfer);
};

#ifdef __cplusplus
}
#endif

#endif  // SRC_DRONECAN_APPLICATION_DRONECAN_DRONECAN_MODULE_HPP_
