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

extern "C" void user_main(void);
extern "C" void retarget_init(void);
__task void main_task();
extern "C" void HAL_GPIO_EXTI_Callback(uint16_t pin);

// TIM1-4: incremental encoder interface
void encoder_init();

// SPI2: absolute encoder (RM22) interface
uint16_t RM22_cnt;
void spi2_init();

void user_main() {
    retarget_init();
    os_sys_init_prio(main_task, 0x80);
}

void chain_load_data();
void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    DBG0 = 1;
    if (pin == GPIO_PIN_4) {
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == 0) {
            chain_load_data();
            chain_transfer();
        } else {
            chain_stop();
        }
    }
    DBG0 = 0;
}

__task void main_task() {
    puts("\r\n\r\n=== RNL2 sensor hub ===\r\n\r\n");

    encoder_init();
    spi2_init();
    chain_init();

    DBG1 = 1;
    os_dly_wait(1000);

    os_itv_set(1);
    while (1) {
        DBG1 = 0;
        HAL_SPI_Receive_DMA(&hspi2, (uint8_t*)&RM22_cnt, 1);
        SHORT_DELAY(2000);
        HAL_SPI_DMAStop(&hspi2);
        SHORT_DELAY(50);
        DBG1 = 1;
        os_itv_wait();
    }
}

void encoder_init() {
    TIM1->CR1 |= TIM_CR1_CEN;
    TIM2->CR1 |= TIM_CR1_CEN;
    TIM3->CR1 |= TIM_CR1_CEN;
    TIM4->CR1 |= TIM_CR1_CEN;
}

void spi2_init() {
    SPI2->CR1 |= SPI_CR1_SPE; //set up idle clock state
}

void chain_load_data() {
    chain_buf[0] = 0xDEAD;
    chain_buf[1] = 0xBEEF;
    chain_buf[2] = TIM1->CNT;
    chain_buf[3] = TIM2->CNT;
    chain_buf[4] = TIM3->CNT;
    chain_buf[5] = TIM4->CNT;
    chain_buf[6] = 0xFF00;
    chain_buf[7] = 0x7000;
}
