#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"

#define QUADRATURE_A_PIN 2
#define QUADRATURE_B_PIN 4

#define MOTOR_DRIVER_IN1 0
#define MOTOR_DRIVER_IN2 1
#define MOTOR_DRIVER_VREF 3

//PWM 
uint freqHz = 10000;
uint wrapP = 12500;
uint slice_num, slice_num1;

//QUADRATURE
#define holes 38
// Old * 4 + New
int QEM [16] = {0,1,-1,2,-1,0,2,1,1,2,0,-1,2,-1,1,0};  // Quadrature Encoder Matrix
int oldState = 0;
int newState = 0;
float position = 0;
float step = 360/(holes*4);
        
void driveMotor(int driveValue, bool driveEnable);

void gpio_callback(uint gpio, uint32_t events) {
    oldState = newState;
    if(gpio==QUADRATURE_A_PIN){
    }
    if(gpio==QUADRATURE_B_PIN){
    }
    newState = gpio_get(QUADRATURE_A_PIN)*2 + gpio_get(QUADRATURE_B_PIN);
    position += step * QEM[oldState*4+newState];
    printf("position: %d\n", position);
}
int main() {
    stdio_init_all();
    
    
    //motor driver
    // set up pwm on GPIO MOTOR_DRIVER_IN1
    gpio_set_function(MOTOR_DRIVER_IN1, GPIO_FUNC_PWM);
    // get PWM channel for that pin
    slice_num = pwm_gpio_to_slice_num(MOTOR_DRIVER_IN1);
    // enable PWM on that channel
    pwm_set_enabled(slice_num, true);
    // set wrap point
    pwm_set_wrap(slice_num, wrapP);
    // set up pwm on GPIO MOTOR_DRIVER_IN2
    gpio_set_function(MOTOR_DRIVER_IN2, GPIO_FUNC_PWM);
    // get PWM channel for that pin
    slice_num1 = pwm_gpio_to_slice_num(MOTOR_DRIVER_IN2);
    // enable PWM on that channel
    pwm_set_enabled(slice_num1, true);
    // set wrap point
    pwm_set_wrap(slice_num1, wrapP);
    //VREF driver
    gpio_init(MOTOR_DRIVER_VREF);
    gpio_set_dir(MOTOR_DRIVER_VREF, GPIO_OUT);
    gpio_put(MOTOR_DRIVER_VREF, 1);
    
    driveMotor(99, true);
 
    while (true) {
        sleep_ms(100);
        
        printf("%d\n", 696969);
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
