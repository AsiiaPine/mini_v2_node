/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Dmitry Ponomarev <ponomarevda96@gmail.com>
 */

#ifndef SRC_PERIPHERY_SPI_SPI_HPP_
#define SRC_PERIPHERY_SPI_SPI_HPP_

#include <cstdint>
#include <cstddef>
#include <utility>

namespace HAL {

int get_transaction_cntr();

class SPI {
public:
    /**
     * @brief Read a single register
     * @param[in] reg_address - a value withing [0, 127]
     * @param[out] reg_value - a register value will be written here on successfull transacation
     * @return 0 on success, negative error otherwise
     * @note The function automatically adds SPI_READ flag
     */
    static int8_t read_register(std::byte reg_address, std::byte* reg_value);

    /**
     * @brief Read multiple registers
     * @param[in] reg_address - a value withing [0, 127]
     * @param[out] reg_values - a register value will be written here on successfull transacation
     * @param[in] size - how many registers to read
     * @return 0 on success, negative error otherwise
     * @note The function automatically adds SPI_READ flag
     */
    static int8_t read_registers(std::byte reg_address, std::byte* reg_values, uint8_t size);

    static int8_t write_register(std::byte reg_address, std::byte reg_value);

    /**
     * @brief This function automatically controls the NSS pin
     * @return 0 on success, negative error otherwise
     */
    static int8_t transaction(std::byte* tx, std::byte* rx, uint8_t size);
    static int8_t dma_receive(std::byte* tx, std::byte* rx, uint8_t size);
    static void reset_finished();
    static uint8_t finished_receive();
    static void (*callback_function)();
    // undefined reference to `HAL::SPI::callback_function'

    template <typename T>
    static void set_callback(T&& callback) {
        callback_function = std::forward<T>(callback);
    }
    static void setup_callback();
};

}  // namespace HAL

#endif  // SRC_PERIPHERY_SPI_SPI_HPP_
