#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "pico/binary_info.h"

//  PIN DEFINITIONS
//  FRONT LEG
#define FRONT_FEMUR_IN1_PIN 0
#define FRONT_FEMUR_IN2_PIN 1
#define FRONT_KNEE_IN1_PIN 2
#define FRONT_KNEE_IN2_PIN 3
#define FRONT_FEMUR_EA_PIN 8
#define FRONT_FEMUR_EB_PIN 9
#define FRONT_KNEE_EA_PIN 10
#define FRONT_KNEE_EB_PIN 11
//  BACK LEG
#define BACK_FEMUR_IN1_PIN 4
#define BACK_FEMUR_IN2_PIN 5
#define BACK_KNEE_IN1_PIN 6
#define BACK_KNEE_IN2_PIN 7
#define BACK_FEMUR_EA_PIN 12
#define BACK_FEMUR_EB_PIN 13
#define BACK_KNEE_EA_PIN 14
#define BACK_KNEE_EB_PIN 15

#define MAX_PWM 100
#define MIN_PWM 60

#define MAX_ANGLE 100
#define MIN_ANGLE 0

//PWM 
uint freqHz = 10000;
uint wrapP = 12500;
  
// Arrays for motor variables
// Motor index to pin
const uint motorIndexToPins[4][4] = { 
  {FRONT_FEMUR_IN1_PIN, FRONT_FEMUR_IN2_PIN, FRONT_FEMUR_EA_PIN, FRONT_FEMUR_EB_PIN}, 
  {FRONT_KNEE_IN1_PIN, FRONT_KNEE_IN2_PIN, FRONT_KNEE_EA_PIN, FRONT_KNEE_EB_PIN}, 
  {BACK_FEMUR_IN1_PIN, BACK_FEMUR_IN2_PIN, BACK_FEMUR_EA_PIN, BACK_FEMUR_EB_PIN}, 
  {BACK_KNEE_IN1_PIN, BACK_KNEE_IN2_PIN, BACK_KNEE_EA_PIN, BACK_KNEE_EB_PIN}
};
// Motor index to old or new state used for calculating position change 
uint motorIndexToOldNewState[4][2] = { 
//    {     oldState  ,     newState    }
  {0, 0}, 
  {0, 0}, 
  {0, 0}, 
  {0, 0}
};
// Motor index to position  
uint motorIndexToPosition[4] = { 
  0, 
  0, 
  0, 
  0
}; 
// Motor index to servo control variables
uint motorIndexToServoControlVariables[4][6] = { 
// {moving0, direction0, setPoint0, startPoint0, error0, P0 }
  {0, 0, 0, 0, 0, 0}, 
  {0, 0, 0, 0, 0, 0}, 
  {0, 0, 0, 0, 0, 0}, 
  {0, 0, 0, 0, 0, 0}
};

int moving, direction, setPoint, startPoint, error, P;
//FUNCTIONS
//MOTOR
void driveMotor(int motorIndex, int driveValue, bool driveEnable);
//BASIC
long map(long x, long in_min, long in_max, long out_min, long outmax);

void resetPosition();

int main() {
   	stdio_init_all();
    	sleep_ms(2000);

	initPins();

	driveMotor(0, 100, true);
 
    	while (true) {
		sleep_ms(2000);	
    	} 
}

void driveMotor(int motorIndex, int driveValue, bool driveEnable){
        // Get target PWM slices
       	uint femurSlice = pwm_gpio_to_slice_num(motorIndexToPins[motorIndex][0]);
        uint kneeSlice = pwm_gpio_to_slice_num(motorIndexToPins[motorIndex][1]);
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
                //TODO: use pwm_set_gpio_level()
		pwm_set_chan_level(femurSlice, PWM_CHAN_A, wrapP * (pwmIn1/100) );
                // pwmIn2
		pwm_set_chan_level(kneeSlice, PWM_CHAN_B, wrapP * (pwmIn2/100) );
        }
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
} 

//  ---------------          INIT          ---------------
void initPins(){

        for (int i = 0; i < 4; i++) {
                // Motor control pins
                // Set up pwm on GPIO pin for IN1
                gpio_set_function(motorIndexToPins[i][0], GPIO_FUNC_PWM);
                // Enable PWM on that channel
                pwm_set_enabled(pwm_gpio_to_slice_num(motorIndexToPins[i][0]), true);
                // Set wrap point
                pwm_set_wrap(pwm_gpio_to_slice_num(motorIndexToPins[i][0]), wrapP);
                
                // Set up pwm on GPIO pin for IN1
                gpio_set_function(motorIndexToPins[i][1], GPIO_FUNC_PWM);
                // Enable PWM on that channel
                pwm_set_enabled(pwm_gpio_to_slice_num(motorIndexToPins[i][1]), true);
                // Set wrap point
                pwm_set_wrap(pwm_gpio_to_slice_num(motorIndexToPins[i][1]), wrapP);
                
                // Encoder pubs
                // Set up the reading pin CHAN A
                //gpio_init(motorIndexToPins[i][2]);
                //gpio_set_dir(motorIndexToPins[i][2], GPIO_IN);
                //gpio_set_irq_enabled_with_callback(motorIndexToPins[i][2], GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoderCallback);
                // Set up the reading pin CHAN B
                //gpio_init(motorIndexToPins[i][3]);
                //gpio_set_dir(motorIndexToPins[i][3], GPIO_IN);
                //gpio_set_irq_enabled_with_callback(motorIndexToPins[i][3], GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoderCallback);
        }
    
    //servo control interrupt 
    // the interrupt is with the pwm wrap frequency
    //pwm_clear_irq(pwm_gpio_to_slice_num(motorIndexToPins[0][0]));
    //pwm_set_irq_enabled(pwm_gpio_to_slice_num(motorIndexToPins[0][0]), true);
    //irq_set_exclusive_handler(PWM_IRQ_WRAP, on_pwm_wrap);
    //irq_set_enabled(PWM_IRQ_WRAP, true);
    //pwm_config config = pwm_get_default_config();
}
