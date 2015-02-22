#include "chain.hpp"


uint16_t chain_buf[chain_buf_n];

static DMA_HandleTypeDef *chain_hdma_tx, *chain_hdma_rx;

// Steps of a complete transfer cycle (after interrupt trigger):
//  1.  Enable SPI DMA request
//  2.  Start SPI
//  3.  (transfers)
//  4.  Stop SPI
//  5.  Disable SPI DMA request
//  6.  Reset DMA
void chain_init() {
    // shortcut to SPI and DMA
    chain_hdma_tx = CHAIN_HSPI.hdmatx;
    chain_hdma_rx = CHAIN_HSPI.hdmarx;

    // initialize DMA pointers
    HAL_DMA_Start(chain_hdma_tx, (uint32_t)chain_buf, (uint32_t)&CHAIN_SPI->DR, chain_buf_n);
    HAL_DMA_Start(chain_hdma_rx, (uint32_t)&CHAIN_SPI->DR, (uint32_t)chain_buf, chain_buf_n);
    __HAL_DMA_DISABLE(chain_hdma_tx);
    __HAL_DMA_DISABLE(chain_hdma_rx);
    CHAIN_SPI->CR2 |= (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
    chain_stop();
}

void chain_transfer() {
    // enable DMA request and start SPI
    CHAIN_SPI->CR2 = (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
    CHAIN_SPI->CR1 |= SPI_CR1_SPE;
}

void chain_stop() {
    // stop SPI and disable DMA request
    CHAIN_SPI->CR1 &=~ SPI_CR1_SPE;
    CHAIN_SPI->CR2 = 0;

    // reset DMA
    __HAL_DMA_DISABLE(CHAIN_HSPI.hdmatx);
    __HAL_DMA_DISABLE(CHAIN_HSPI.hdmarx);
    chain_hdma_tx->Instance->CNDTR = chain_buf_n;
    chain_hdma_rx->Instance->CNDTR = chain_buf_n;
    __HAL_DMA_ENABLE(CHAIN_HSPI.hdmatx);
    __HAL_DMA_ENABLE(CHAIN_HSPI.hdmarx);
}
