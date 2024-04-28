#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/binary_info.h"

// encoder
#define resolution 64
int degreesPerHole = 360/resolution;
int totalSpinDeg = 0;

// PWM calculations
uint freqHz = 10000;
uint wrapP = 12500;

uint slice_num, slice_num1;
        
void driveMotor(int driveValue, bool driveEnable);
void servoDriveMotor(int degrees);

int time1 = 0;
double time2 = 0;
double rpm = 0;
int liveDegrees = 0;

void gpio_callback(uint gpio, uint32_t events) {
    if(gpio==2){
	  printf("Interrupt\n");
	  //get degrees
          totalSpinDeg += degreesPerHole;
	  printf("Total spin: %d \n", totalSpinDeg);
	  //get rpm
	  if (time1 != 0){  
	    time2 = to_ms_since_boot (get_absolute_time());
	    //the calculations are so cut because i couldn't make it in one expression
	    rpm = (float) 1 / resolution;
	    time2 = (float) (time2-time1) / 60000;
	    rpm = (float) rpm / time2;
	    printf("RPM: %f \n", rpm);
          }
          time1 = to_ms_since_boot (get_absolute_time());
    }
}

int main() {

	// set up the built-in led
	gpio_init(25);
    	gpio_set_dir(25, GPIO_OUT);

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
	
    	// Init UART communication
	stdio_init_all();
	printf("--- INIT ---");

	int input = 0;	
	
        //VREF driver
	gpio_init(3);
    	gpio_set_dir(3, GPIO_OUT);
        gpio_put(3, 1);
        
        //set up the reading pin
        gpio_init(2);
        gpio_set_dir(2, GPIO_IN);
        //gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
        gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
		
    	while (true) {
        	gpio_put(25, 1);

		printf("Enter angle Xx: \n");
		input = getchar() - 48;
		input *= 10;
		printf("Target angle: %d \n", input);
		//driveMotor(input, true);
		servoDriveMotor(input);
		
        	sleep_ms(500);
		
		
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
		//printf("pwmIn1: %d \n", pwmIn1);
		//printf("pwmIn2: %d \n", pwmIn2);
                // pwmIn1
		pwm_set_chan_level(slice_num, PWM_CHAN_A, wrapP * (pwmIn1/100) );
                // pwmIn2
		pwm_set_chan_level(slice_num1, PWM_CHAN_B, wrapP * (pwmIn2/100) );
        }
}

void servoDriveMotor(int degrees){
  // get direction
  int direction = (degrees != 0) ? ((degrees > 0) ? 1 : -1) : 0;
  printf("direction: %d \n", direction);
  if(direction != 0){
    //calculate target degrees
    int targetDegrees = totalSpinDeg + degrees;
    printf("targetDegrees: %d \n", targetDegrees);
    
    
    driveMotor(direction*100, true);  
    //while !target degrees move
    int a = 1;
    int diff = totalSpinDeg - direction*targetDegrees;
    while(diff!=0){
      diff = totalSpinDeg - direction*targetDegrees;
      printf("diff: %d \n", diff);
      sleep_ms(5);
    }
    
    //brake
    driveMotor(0, true);
  }
}
