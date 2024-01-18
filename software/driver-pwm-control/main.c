#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/binary_info.h"

#define MOTOR_FEMUR_IN1_PIN 0
#define MOTOR_FEMUR_IN2_PIN 1
#define MOTOR_FEMUR_VREF_PIN 3
#define FEMUR_ENCODER_A_PIN 2
#define FEMUR_ENCODER_B_PIN 4

#define MOTOR_KNEE_IN1_PIN 8
#define MOTOR_KNEE_IN2_PIN 9
#define KNEE_ENCODER_A_PIN 11
#define KNEE_ENCODER_B_PIN 12
#define KNEE_ENCODER_3V_PIN 10

// PWM calculations
uint freqHz = 10000;
uint wrapP = 12500;

uint slice_num, slice_num1;
        
void driveMotor(int driveValue, bool driveEnable);

//encoder
#define resolution 60
int degreesPerHole = 360/resolution;
int totalSpinDeg = 0;

int time1 = 0;
double time2 = 0;
double rpm = 0;

void gpio_callback(uint gpio, uint32_t events) {
    if(gpio==2){
      if (events == GPIO_IRQ_EDGE_RISE){
        gpio_put(6, 1);
      }
      else{
        gpio_put(6, 0);
      }
    }
    if(gpio==4){
      if (events == GPIO_IRQ_EDGE_RISE){
        gpio_put(7, 1);
      }
      else{
        gpio_put(7, 0);
      }
    }
}

int main() {

	// set up the built-in led
	gpio_init(25);
    	gpio_set_dir(25, GPIO_OUT);

        // set up pwm on GPIO 0
        gpio_set_function(MOTOR_FEMUR_IN1_PIN, GPIO_FUNC_PWM);
        // get PWM channel for that pin
        slice_num = pwm_gpio_to_slice_num(MOTOR_FEMUR_IN1_PIN);
        // enable PWM on that channel
        pwm_set_enabled(slice_num, true);
        // set wrap point
        pwm_set_wrap(slice_num, wrapP);

        // set up pwm on GPIO 1
        gpio_set_function(MOTOR_FEMUR_IN2_PIN, GPIO_FUNC_PWM);
        // get PWM channel for that pin
        slice_num1 = pwm_gpio_to_slice_num(MOTOR_FEMUR_IN2_PIN);
        // enable PWM on that channel
        pwm_set_enabled(slice_num1, true);
        // set wrap point
        pwm_set_wrap(slice_num1, wrapP);
	
    	// Init UART communication
	stdio_init_all();

	int input = 0;	

        //VREF driver
	gpio_init(3);
    	gpio_set_dir(3, GPIO_OUT);
        gpio_put(3, 1);
        
        //set up the reading pin CHAN A
        gpio_init(2);
        gpio_set_dir(2, GPIO_IN);
        gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
        //digital output check
	gpio_init(6);
    	gpio_set_dir(6, GPIO_OUT);
    	
    	//set up the reading pin CHAN B
        gpio_init(4);
        gpio_set_dir(4, GPIO_IN);
        gpio_set_irq_enabled_with_callback(4, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
        //digital output check
	gpio_init(7);
    	gpio_set_dir(7, GPIO_OUT);
		
	//driveMotor(99, true);
        
        gpio_put(25, 1);
		
    	while (true) {
		printf("Enter angle Xx: \n");
		input = getchar() - 48;
		input *= 10;
		printf("Target angle: %d \n", input);
		driveMotor(input, true);
		//servoDriveMotor(input);
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
