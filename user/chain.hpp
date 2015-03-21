#ifndef _CHAIN_HPP_
#define _CHAIN_HPP_

#include "spi.h"
#include "dma.h"

#include "conf.hpp"


#define CHAIN_SPI SPI1
#define CHAIN_HSPI hspi1
#define CHAIN_SPI_RESET do{ \
    __SPI1_FORCE_RESET(); \
    __SPI1_RELEASE_RESET(); \
    __SPI1_CLK_ENABLE(); \
} while (0)


extern bool chain_inited;
extern volatile uint16_t chain_buf[chain_buf_n];

void chain_init(void);

// On NSS falling edge:
//  1.  write `chain_buf`
//  2.  call `chain_transfer`
// On NSS rising edge:
//  1.  call `chain_stop`
void chain_transfer(void);
void chain_stop(void);

#endif//_CHAIN_HPP_
