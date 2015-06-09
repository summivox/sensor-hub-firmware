#ifndef _CONF_HPP_
#define _CONF_HPP_

//#define DUMMY_SPI_MASTER

const int chain_buf_n = 8;
//#define CHAIN_DUMMY_DATA {0xDEAD, 0xBEEF, 0xA55A, 0x5AA5, 0x7007, 0x0000, 0xFFFF, 0x1111}

const int adc_ch_n = 3;
const int adc_decimation_bits = 5;
const int adc_decimation = 1<<adc_decimation_bits;
const int adc_decimation_rank = 2;


#endif//_CONF_HPP_
