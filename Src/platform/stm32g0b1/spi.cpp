/**
 * This program is free software under the GNU General Public License v3.
 * See <https://www.gnu.org/licenses/> for details.
 * Author: Dmitry Ponomarev <ponomarevda96@gmail.com>
 */

#include "peripheral/spi/spi.hpp"
#include <array>
#include <cstring>
#include "main.h"

static constexpr uint32_t TRANSMIT_DELAY = 10;
static constexpr std::byte SPI_READ{0x80};

extern SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef* hspi_n = &hspi2;

volatile uint8_t TXfinished = 0;
static uint32_t transactions_ctr = 0;
void SPI_DMATransmitReceiveCplt(SPI_HandleTypeDef *hspi) {
    transactions_ctr++;
    (void)hspi;
    TXfinished = 1;
}

namespace HAL {

uint32_t get_transaction_cntr() {
    return transactions_ctr;
}

static void spi_set_nss(bool nss_state) {
#ifdef SPI2_NSS_GPIO_Port
    auto state = nss_state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, state);
#endif
}

int8_t SPI::read_registers(std::byte reg_address, std::byte* reg_values, uint8_t size) {
    if (reg_values == nullptr) {
        return -1;
    }

    auto tx_byte = reg_address | SPI_READ;
    return HAL::SPI::dma_receive(&tx_byte, &reg_values[-1], size + 1);
    // return HAL::SPI::transaction(&tx_byte, &reg_values[-1], size + 1);
}

int8_t SPI::read_register(std::byte reg_address, std::byte* reg_value) {
    if (reg_value == nullptr) {
        return -1;
    }

    auto tx_byte = reg_address | SPI_READ;
    // return HAL::SPI::dma_receive(&tx_byte, &reg_value[-1], 2);

    return HAL::SPI::transaction(&tx_byte, &reg_value[-1], 2);
}

int8_t SPI::write_register(std::byte reg_address, std::byte reg_value) {
    HAL_Delay(10);
    std::array<std::byte, 2> tx_buffer = {reg_address, reg_value};
    std::array<std::byte, 2> rx_buffer = {std::byte{0}, std::byte{0}};
    return HAL::SPI::transaction(tx_buffer.data(), rx_buffer.data(), tx_buffer.size());
}

int8_t SPI::transaction(std::byte* tx, std::byte* rx, uint8_t size) {
    if (tx == nullptr || rx == nullptr) {
        return -1;
    }

    spi_set_nss(false);
    memset(rx, 0x00, size);
    auto status = HAL_SPI_TransmitReceive(hspi_n,
                                          reinterpret_cast<uint8_t*>(tx),
                                          reinterpret_cast<uint8_t*>(rx),
                                          size,
                                          TRANSMIT_DELAY
    );
    spi_set_nss(true);

    return (status == HAL_OK) ? 0 : -status;
}

int8_t SPI::dma_receive(std::byte* tx, std::byte* rx, uint8_t size) {
    if (TXfinished == 0) {
        return 0;
    }
    transactions_ctr++;

    // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_SPI_Receive_DMA(hspi_n, reinterpret_cast<uint8_t*>(rx), size);
    auto status = HAL_SPI_Transmit(hspi_n, reinterpret_cast<uint8_t*>(tx), 1, 100);
    TXfinished = 0;
    /*Wait until the data is transmitted*/
    // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    return status;
}

}  // namespace HAL
