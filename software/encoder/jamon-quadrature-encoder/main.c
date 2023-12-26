//  SPDX-FileCopyrightText: 
//2022 Jamon Terrell <github@jamonterrell.com>
//2024 Arda Alıcı <ardayaozel@hotmail.com>
//  SPDX-License-Identifier: MIT

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"
#include "quadrature.pio.h"

#define QUADRATURE_A_PIN 2
#define QUADRATURE_B_PIN 4

// PWM calculations
uint freqHz = 10000;
uint wrapP = 12500;

uint slice_num, slice_num1;
        
void driveMotor(int driveValue, bool driveEnable);

int main() {
    stdio_init_all();
    
    //quadrature
    PIO encoderPIO = pio0;

    uint offsetA = pio_add_program(encoderPIO, &quadratureA_program);
    uint smA = pio_claim_unused_sm(encoderPIO, true);

    uint offsetB = pio_add_program(encoderPIO, &quadratureB_program);
    uint smB = pio_claim_unused_sm(encoderPIO, true);

    quadratureA_program_init(encoderPIO, smA, offsetA, QUADRATURE_A_PIN, QUADRATURE_B_PIN);
    quadratureB_program_init(encoderPIO, smB, offsetB, QUADRATURE_A_PIN, QUADRATURE_B_PIN);
    
    //motor driver
    // set up pwm on GPIO 0
    gpio_set_function(0, GPIO_FUNC_PWM);
    // get PWM channel for that pin
    slice_num = pwm_gpio_to_slice_num(0);
    // enable PWM on that channel
    pwm_set_enabled(slice_num, true);
    // set wrap point
    pwm_set_wrap(slice_num, wrapP);
    // set up pwm on GPIO 1
    gpio_set_function(1, GPIO_FUNC_PWM);
    // get PWM channel for that pin
    slice_num1 = pwm_gpio_to_slice_num(1);
    // enable PWM on that channel
    pwm_set_enabled(slice_num1, true);
    // set wrap point
    pwm_set_wrap(slice_num1, wrapP);
    //VREF driver
    gpio_init(3);
    gpio_set_dir(3, GPIO_OUT);
    gpio_put(3, 1);
    
    driveMotor(99, true);
 
    while (true) {
        sleep_ms(100);

        pio_sm_exec_wait_blocking(encoderPIO, smA, pio_encode_in(pio_x, 32));
        pio_sm_exec_wait_blocking(encoderPIO, smB, pio_encode_in(pio_x, 32));

        int32_t countA = pio_sm_get_blocking(encoderPIO, smA);
        int32_t countB = pio_sm_get_blocking(encoderPIO, smB);

        int32_t x = countA + countB;

        //uint x = pio_sm_get_blocking(pio, sm);
        printf("%d\n", x);
    }
}

void driveMotor(int driveValue, bool driveEnable){
        // Check if the motor should be moved
        if(driveEnable){
                // Calculate input 1 & 2
                double pwmIn1, pwmIn2;
                if(driveValue == 0){
                        // Brake
                        pwmIn1 = 100;
                        pwmIn2 = 100;
                }
                else if(driveValue > 0){
                        pwmIn1 = driveValue;
                        pwmIn2 = 0;
                }
                else{
                        pwmIn1 = 0;
                        pwmIn2 = driveValue*-1;
                }
                // Write to pins
		printf("pwmIn1: %d \n", pwmIn1);
		printf("pwmIn2: %d \n", pwmIn2);
                // pwmIn1
		pwm_set_chan_level(slice_num, PWM_CHAN_A, wrapP * (pwmIn1/100) );
                // pwmIn2
		pwm_set_chan_level(slice_num1, PWM_CHAN_B, wrapP * (pwmIn2/100) );
        }
}
