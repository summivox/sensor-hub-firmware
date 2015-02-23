#include "analog.hpp"

#include "misc.hpp"
#include "cic.hpp"


#define EACH(ch) for (int ch = 0 ; ch < adc_ch_n ; ++ch)


// Output (only latest is kept)
volatile q15_t adc_val[adc_ch_n];

// Data organization:
//  ADC scans channels continuously => `adc_raw[sample#][channel#]`
//  which can be treated as a double buffer. After each buffer is filled
//  with new data, each channel is fed into its decimation filter.

static volatile q15_t adc_raw[adc_decimation*2][adc_ch_n] ALIGN32;
static q15_t * const adc_raw_0 = (q15_t*)adc_raw[0];
static q15_t * const adc_raw_1 = (q15_t*)adc_raw[adc_decimation];

CIC<12, adc_decimation_bits, adc_decimation_rank> cic[adc_ch_n];

void adc_init() {
    HAL_ADCEx_Calibration_Start(&hadc1);
    DBG0 = 1;
}

void adc_start() {
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_raw, adc_ch_n*adc_decimation*2);
    DBG0 = 0;
}

static void average(q15_t* p) {
    EACH(ch) adc_val[ch] = cic[ch](&p[ch], adc_ch_n);
}

extern "C" void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
    // first half of the double buffer is ready
    DBG0 = 1;
    average(adc_raw_0);
    DBG0 = 0;
}
extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    // second half of the double buffer is ready
    DBG0 = 1;
    average(adc_raw_1);
    DBG0 = 0;
}
