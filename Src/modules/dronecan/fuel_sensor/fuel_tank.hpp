/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Anastasiia Stepanova <asiiapine@gmail.com>
 * Author: Dmitry Ponomarev <ponomarevda96@gmail.com>
 */

#ifndef SRC_MODULES_FUEL_TANK_HPP_
#define SRC_MODULES_FUEL_TANK_HPP_

#include "params.hpp"
#include <stdint.h>
#include "uavcan/equipment/ice/FuelTankStatus.h"
// #include "drivers/as5600/as5600.hpp"
#include "peripheral/adc/circuit_periphery.hpp"
#include "uavcan/equipment/esc/RawCommand.h"
#include "algorithms.hpp"
#include "common/module.hpp"
#include "publisher.hpp"
#include "subscriber.hpp"
#include "common/logging.hpp"

class VtolFuelTank : public Module {
public:
    /**
     * @brief Fuel tank should me measured 10 times per second, an average value should be
     * published once per second.
     */
    VtolFuelTank();

    /**
     * @brief Initialize the required i2c peripheral and AS5600 sensor.
     */
    void init() override;

    /**
     * @brief Apply all the configuration parameters
     */
    void update_params() override;

    /**
     * @brief Measure data with 10 Hz and publish with 1 Hz if data is avaliable
     */
    void spin_once() override;

private:
    void configure();

    struct Parameters {
        uint16_t volume_cm3;
        uint16_t empty_tank_raw;
        uint16_t full_tank_enc_raw;
        uint8_t min_magnitude;
        uint8_t tank_id;
    };

    struct InternalState {
        float min_value;
        float max_value;
        float range;
        float raw_angle;
        float filtered_angle;
        float available_fuel_cm3;
        int16_t magnitude;
        uint8_t available_fuel_pct;
    };
    uint32_t measurement_deadline{0};

    Parameters params;
    InternalState state;
    Logging logger{"FUEL"};

    static inline uint32_t arm_deadline_ms{0};   

    // DroneCAN related things
    static void raw_command_callback(const RawCommand_t& msg);
    void publish_dronecan_fuel_tank();
    static inline DronecanSubscriber<RawCommand_t> raw_command_sub;
    uint32_t next_publish_time_ms{1000};
    uint8_t transfer_id{0};

    // Driver instance

    static constexpr const uint32_t CMD_TTL_MS{1000};
    static constexpr const uint32_t REINITIALIZATION_TIMEOUT{5000};
    static constexpr const uint32_t MEASUREMENT_TTL{1000};
    static constexpr const uint32_t PUBLISH_PERIOD{1000};
    static constexpr const uint8_t FILTER_SMOOTH{15};
    static constexpr const uint8_t FILTER_FAST{5};
};


#endif  // SRC_MODULES_FUEL_TANK_HPP_
