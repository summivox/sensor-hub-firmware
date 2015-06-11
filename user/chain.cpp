#include "chain.hpp"

bool chain_inited = false;
volatile uint16_t chain_buf[chain_buf_n];

/*static*/ DMA_HandleTypeDef *chain_hdma_tx, *chain_hdma_rx;
static uint32_t CR1 = 0;

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

    // save SPI config
    CR1 = CHAIN_SPI->CR1;

    // initialize DMA pointers
    HAL_DMA_Start(chain_hdma_tx, (uint32_t)chain_buf, (uint32_t)&CHAIN_SPI->DR, chain_buf_n);
    HAL_DMA_Start(chain_hdma_rx, (uint32_t)&CHAIN_SPI->DR, (uint32_t)chain_buf, chain_buf_n);
    chain_stop();

    chain_inited = true;
}

void chain_transfer() {
    // enable SPI DMA request and start SPI
    CHAIN_SPI->CR2 = (SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
    CHAIN_SPI->CR1 |= SPI_CR1_SPE;
}

void chain_stop() {
    // stop SPI and disable SPI DMA request
    CHAIN_SPI->CR1 &=~ SPI_CR1_SPE;
    CHAIN_SPI->CR2 = 0;

    // reset DMA
    __HAL_DMA_DISABLE(chain_hdma_tx);
    __HAL_DMA_DISABLE(chain_hdma_rx);
    chain_hdma_tx->Instance->CNDTR = chain_buf_n;
    chain_hdma_rx->Instance->CNDTR = chain_buf_n;
    __HAL_DMA_ENABLE(chain_hdma_tx);
    __HAL_DMA_ENABLE(chain_hdma_rx);

    // reset SPI
    CHAIN_SPI_RESET;
    CHAIN_SPI->CR1 = CR1;
}
