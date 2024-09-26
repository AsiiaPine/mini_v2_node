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
SPI_HandleTypeDef* hspi = &hspi2;
extern DMA_HandleTypeDef hdma_spi2_rx;
extern DMA_HandleTypeDef hdma_spi2_tx;

volatile uint8_t TXfinished = 1;
volatile uint8_t RXfinished = 1;

static void spi_set_nss(bool nss_state) {
#ifdef SPI2_NSS_GPIO_Port
    auto state = nss_state ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, state);
#endif
}

// void SPI_DMATransmitCallback(DMA_HandleTypeDef *_hdma) {
//     // transactions_ctr++;
//     (void)_hdma;
//     TXfinished = 1;
// }

// void SPI_DMAReceiveCallback(DMA_HandleTypeDef *_hdma) {
//     transactions_ctr++;
//     (void)_hdma;
//     RXfinished = 1;
//     HAL::SPI::callback_function();
// }
// __HAL_SPI_ENABLE_IT(&hspi2, (SPI_IT_TXE | SPI_IT_ERR));

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *_hspi) {
    // if(hspi->Instance == SPI2) {
    (void)_hspi;
    // HAL::SPI::callback_function();
    // TXfinished = 1;
    // spi_set_nss(true);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *_hspi) {
    // if(hspi->Instance == SPI2) {
    (void)_hspi;
    // HAL::SPI::callback_function();
    // RXfinished = 1;
}
// void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *_hspi) {
//     // if(hspi->Instance == SPI2) {
//     (void)_hspi;
//     transactions_ctr++;
//     TXfinished = 1;
//     // RXfinished = 1;
// }

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *_hspi) {
    (void)_hspi;
    // spi_set_nss(true);
    // HAL::SPI::callback_function();
    // TXfinished = 1;
}

// void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *_hspi) {
//     (void)_hspi;
//     HAL_SPI_FlushRxFifo(_hspi);
//     transactions_ctr = 0;
// }

namespace HAL {
void SPI::setup_callback() {
    // HAL_DMA_RegisterCallback(&hdma_spi2_tx, HAL_DMA_XFER_CPLT_CB_ID, &SPI_DMATransmitCallback);
    // HAL_DMA_RegisterCallback(&hdma_spi2_rx, HAL_DMA_XFER_CPLT_CB_ID, &SPI_DMAReceiveCallback);
    // HAL_SPI_RegisterCallback(&hspi2, HAL_SPI_TX_COMPLETE_CB_ID, HAL_SPI_TxCpltCallback);
    // HAL_SPI_RegisterCallback(&hspi2, HAL_SPI_TX_RX_COMPLETE_CB_ID, HAL_SPI_TxRxCpltCallback);
}

void (*SPI::callback_function)() = nullptr;

int8_t SPI::read_registers(std::byte reg_address, std::byte* reg_values, uint8_t size) {
    if (reg_values == nullptr) {
        return -1;
    }

    auto tx_byte = reg_address | SPI_READ;
    // return HAL::SPI::dma_receive(&tx_byte, &reg_values[-1], size + 1);
    return HAL::SPI::transaction(&tx_byte, &reg_values[-1], size + 1);
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
    auto status = HAL_SPI_TransmitReceive(hspi,
                                          reinterpret_cast<uint8_t*>(tx),
                                          reinterpret_cast<uint8_t*>(rx),
                                          size
                                          ,
                                          TRANSMIT_DELAY
    );
    spi_set_nss(true);

    return (status == HAL_OK) ? 0 : -status;
}

int8_t SPI::dma_receive(std::byte* tx, std::byte* rx, uint8_t size) {
    if (tx == nullptr || rx == nullptr) {
        return -1;
    }
    // spi_set_nss(false);
    if (hspi->ErrorCode != HAL_SPI_ERROR_NONE) {
        // hspi->ErrorCode = HAL_SPI_ERROR_NONE;
        return -1;
    }
    // if (hspi->State != HAL_SPI_STATE_READY) {
    //     return -1;
    // }
    if (hdma_spi2_rx.State != HAL_DMA_STATE_READY) {
        int16_t rx_extra[3];
        HAL_SPI_Receive(hspi,
                            reinterpret_cast<uint8_t*>(rx_extra),
                            size,
                            TRANSMIT_DELAY
        );
        return -1;
    }
    auto status = HAL_SPI_TransmitReceive_DMA(hspi, reinterpret_cast<uint8_t*>(tx),
                                        reinterpret_cast<uint8_t*>(rx), (uint16_t)size);
    return (status == HAL_OK) ? 0 : -status;
    // int8_t status = 0;
    // if (hdma_spi2_tx.State == HAL_DMA_STATE_READY) {
    // // if (TXfinished) {
    //     // spi_set_nss(false);
    //     status = HAL_SPI_Transmit_DMA(hspi, reinterpret_cast<uint8_t*>(tx), (uint16_t)size);
    //     TXfinished = status? 0: 1;
    // }
    // if (hdma_spi2_rx.State == HAL_DMA_STATE_READY) {
    // // if (RXfinished) {
    //     status = HAL_SPI_Receive_DMA(hspi, reinterpret_cast<uint8_t*>(rx), (uint16_t)size);
    //     RXfinished = status? 0: 1;
    //     spi_set_nss(false);
    // }
    // return (status == HAL_OK) ? 0 : -status;
    // if (hspi->State == HAL_SPI_STATE_ERROR) {
    //     HAL_SPI_FlushRxFifo(hspi);
    //     // &hspi->State = HAL_SPI_STATE_BUSY_TX;
    //     return 0;
    // }

    // return status;
    // auto status = HAL_SPI_TransmitReceive_DMA(hspi,
    //                                       reinterpret_cast<uint8_t*>(tx),
    //                                       reinterpret_cast<uint8_t*>(rx),
    //                                       size);
}

}  // namespace HAL
