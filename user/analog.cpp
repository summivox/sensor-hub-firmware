#include "analog.hpp"

#include "arm_math.h"

#include "misc.hpp"


q15_t adc_val[adc_ch_n];

static q15_t adc_raw[adc_decimation_m*2][adc_ch_n];
static q15_t adc_raw_tr[adc_ch_n][adc_decimation_m];

static arm_matrix_instance_q15
    adc_raw_m1 = {adc_decimation_m, adc_ch_n, adc_raw[0]},
    adc_raw_m2 = {adc_decimation_m, adc_ch_n, adc_raw[adc_decimation_m]},
    adc_raw_tr_m = {adc_ch_n, adc_decimation_m, adc_raw_tr[0]};

// decimation (downsampling) filter:
//  const size_t filt_n;
//  const q15_t filt_coeffs[filt_n];
// decimation factor == `adc_decimation_m` (one block in -> one sample out)
#include "filter.h"
static q15_t filt_state[adc_ch_n][filt_n + adc_decimation_m - 1];
static arm_fir_decimate_instance_q15 filt[adc_ch_n];

#define EACH(ch) for (int ch = 0 ; ch < adc_ch_n ; ++ch)

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
}

void adc_start() {
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_raw, adc_ch_n*adc_decimation_m*2);
    DBG0 = 1;
}

extern "C" void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
    arm_mat_trans_q15(&adc_raw_m1, &adc_raw_tr_m);
    EACH(ch)
        arm_fir_decimate_fast_q15
            ( &filt[ch]
            , adc_raw_tr[ch] //src
            , &adc_val[ch] //dest
            , adc_decimation_m
            );
}
extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    arm_mat_trans_q15(&adc_raw_m2, &adc_raw_tr_m);
    EACH(ch)
        arm_fir_decimate_fast_q15
            ( &filt[ch]
            , adc_raw_tr[ch] //src
            , &adc_val[ch] //dest
            , adc_decimation_m
            );
}

#undef EACH
