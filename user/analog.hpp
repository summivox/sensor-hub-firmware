#ifndef _ANALOG_HPP_
#define _ANALOG_HPP_

// NOTE: files renamed to `analog` to avoid name clash with auto-generated peripheral configuration file

#include <stdint.h>
#include "arm_math.h"

#include "adc.h"
#include "dma.h"

#include "conf.hpp"

extern q15_t adc_val[adc_ch_n];

void adc_init();
void adc_start();

#endif//_ANALOG_HPP_
