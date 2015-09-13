#include "stm32f1xx_hal.h"
#include "rtl.h"

#include "gpio.h"
#include "tim.h"
#include "spi.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "conf.hpp"
#include "misc.hpp"
#include "chain.hpp"
#include "analog.hpp"

extern "C" void retarget_init(void);
__task void main_task();


// TIM1-4: incremental encoder interface
void enc_inc_init();

// SPI2: absolute encoder interface
volatile uint16_t enc_abs_val;
void enc_abs_init();
void enc_abs_start();
void enc_abs_stop();


// data payload
void chain_load_data() {
#ifndef CHAIN_DUMMY_DATA
    chain_buf[0] = enc_abs_val;
    chain_buf[1] = TIM1->CNT;
    chain_buf[2] = TIM2->CNT;
    chain_buf[3] = TIM3->CNT;
    chain_buf[4] = TIM4->CNT;
    chain_buf[5] = adc_val[0];
    chain_buf[6] = adc_val[1];
#else
    static const uint16_t dummy[chain_buf_n] = CHAIN_DUMMY_DATA;
    memcpy((void*)chain_buf, dummy, chain_buf_n*sizeof(*dummy));
#endif
}

static void reset_jtag() {
    // Properly release all JTAG pins
    // See last post in <https://my.st.com/public/STe2ecommunities/mcu/Lists/cortex_mx_stm32/STM32%20F103xx%20using%20PB3>
    __HAL_AFIO_REMAP_SWJ_DISABLE();
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
    DBGMCU->CR &=~ DBGMCU_CR_TRACE_IOEN;
}

extern "C" void $Super$$main(void);
extern "C" void $Sub$$main() {
    $Super$$main();
    retarget_init();
    //reset_jtag();
    os_sys_init_prio(main_task, 0x80);
}
__task void main_task() {
    puts(
        "\r\n\r\n"
        "### RNL3 sensor hub \r\n"
        "### Build: " __DATE__ " " __TIME__ "\r\n"
        "\r\n"
    );

    enc_inc_init();
    enc_abs_init();
    enc_abs_start();
    adc_init();
    adc_start();
    chain_init();

    os_itv_set(100);
    while (1) {
        static int n = 0;
        if (++n == 100) n = 0;
        printf("%02d: %04X\r\n", n, adc_val[0]);
        os_itv_wait();
    }
}

void enc_inc_init() {
    // enable all encoder interface timers
    TIM1->CR1 |= TIM_CR1_CEN;
    TIM2->CR1 |= TIM_CR1_CEN;
    TIM3->CR1 |= TIM_CR1_CEN;
    TIM4->CR1 |= TIM_CR1_CEN;
}

void enc_abs_init() {
    // enable SPI for correct clock idle polarity (CPOL)
    SPI2->CR1 |= SPI_CR1_SPE;
}
void enc_abs_start() {
    HAL_NVIC_DisableIRQ(DMA1_Channel4_IRQn);
    HAL_NVIC_DisableIRQ(DMA1_Channel5_IRQn);
    HAL_SPI_Receive_DMA(&hspi2, (uint8_t*)&enc_abs_val, 1);
}
void enc_abs_stop() {
    HAL_SPI_DMAStop(&hspi2);
}


extern "C" void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    if (pin == GPIO_PIN_4) {
        // chain trigger interrupt handler
        if (chain_inited) {
            if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == 0) {
                chain_load_data();
                chain_transfer();
            } else {
                chain_stop();
            }
        }
    }
}

