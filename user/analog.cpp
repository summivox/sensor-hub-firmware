#include "analog.hpp"

#include "arm_math.h"

#include "misc.hpp"




// Output (only latest is kept)
q15_t adc_val[adc_ch_n] ALIGN32;


// Data organization:
//  ADC scans channels continuously => `adc_raw[sample#][channel#]`
//  which can be treated as a double buffer. After each buffer is filled
//  with new data, transpose => `adc_raw_tr[channel#][sample#]` and feed
//  into decimation filters (independent state for each channel).
//
//  Or, just average each group (for speed)

static q15_t adc_raw[adc_decimation_m*2][adc_ch_n] ALIGN32;
static q15_t adc_raw_tr[adc_ch_n][adc_decimation_m] ALIGN32;

static arm_matrix_instance_q15
    adc_mat_1 = {adc_decimation_m, adc_ch_n, adc_raw[0]},
    adc_mat_2 = {adc_decimation_m, adc_ch_n, adc_raw[adc_decimation_m]},
    adc_mat_tr = {adc_ch_n, adc_decimation_m, adc_raw_tr[0]};


// Decimation (downsampling) filter:
//  const size_t filt_n;
//  const q15_t filt_coeffs[filt_n];
// 
// block size == decimation factor (1 block => 1 sample)

#include "filter.h"
static q15_t filt_state[adc_ch_n][filt_n + adc_decimation_m - 1] ALIGN32;
static arm_fir_decimate_instance_q15 filt[adc_ch_n];

#define EACH(ch) for (int ch = adc_ch_n ; ch --> 0 ; )

void adc_init() {
    HAL_ADCEx_Calibration_Start(&hadc1);
    EACH(ch)
        arm_fir_decimate_init_q15
            ( &filt[ch]
            , filt_n
            , adc_decimation_m
            , (q15_t*)filt_coeffs
            , filt_state[ch]
            , adc_decimation_m
            );
    DBG0 = 1;
}

void adc_start() {
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_raw, adc_ch_n*adc_decimation_m*2);
    DBG0 = 0;
}

extern "C" void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
    // first half of the double buffer is ready
    DBG0 = 1;

#if ADC_FILTER_ENABLE
    // actual decimation filter (slow)
    arm_mat_trans_q15(&adc_mat_1, &adc_mat_tr);
    EACH(ch)
        arm_fir_decimate_fast_q15
            ( &filt[ch]
            , adc_raw_tr[ch] //src
            , &adc_val[ch] //dest
            , adc_decimation_m
            );
#else
    // naive average
    memset(adc_val, 0, sizeof(adc_val));
    for (int i = adc_decimation_m ; i --> 0 ; --i)
        EACH(ch)
            adc_val[ch] += adc_raw[i][ch];
    EACH(ch)
        adc_val[ch] /= adc_decimation_m;
#endif

    DBG0 = 0;
}
extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    // second half of the double buffer is ready
    DBG0 = 1;

#if ADC_FILTER_ENABLE
    // actual decimation filter (slow)
    arm_mat_trans_q15(&adc_mat_2, &adc_mat_tr);
    EACH(ch)
        arm_fir_decimate_fast_q15
            ( &filt[ch]
            , adc_raw_tr[ch] //src
            , &adc_val[ch] //dest
            , adc_decimation_m
            );
#else
    // naive average
    memset(adc_val, 0, sizeof(adc_val));
    for (int i = adc_decimation_m*2 ; i --> adc_decimation_m ; --i)
        EACH(ch)
            adc_val[ch] += adc_raw[i][ch];
    EACH(ch)
        adc_val[ch] /= adc_decimation_m;
#endif

    DBG0 = 0;
}
