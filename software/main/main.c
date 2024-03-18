#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "pico/binary_info.h"

#include "quadrature_encoder.pio.h"

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

//ENCODERS
#define holes 38
float step = 2.368421053;//360/(holes*4);
// Motor index to position
float motorIndexToPosition[4] = {0};
// Motor index to encoder pins | pin AB
    	// Base pin to connect the A phase of the encoder.
    	// The B phase must be connected to the next pin
uint motorIndexToEncoderPinAB[4] = {8, 10, 12, 14};
// MOTOR INDEX = SM INDEX (ENCODER STATE MACHINE INDEX)
//PIO
PIO pio = pio0;
	
//SERVO CONTROL
#define KP 5
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

// FIXME: MANUAL ANGLE TESTING
void keyboardControl();

void on_pwm_wrap() {
//NO PRINT F INSIDE OF THIS FUNCTION!
//there is not enough time to execute and clogs the program!

        // Clear the interrupt flag that brought us here
        pwm_clear_irq(pwm_gpio_to_slice_num(motorIndexToPins[0][0]));
        
        // FIXME: POSITION GETTING
        for (int i = 0; i < 4; i++) {
                moving = motorIndexToServoControlVariables[i][0];
                direction = motorIndexToServoControlVariables[i][1];
                setPoint = motorIndexToServoControlVariables[i][2];
                startPoint = motorIndexToServoControlVariables[i][3];
                error = motorIndexToServoControlVariables[i][4];
                P = motorIndexToServoControlVariables[i][5];
                
                motorIndexToPosition[i] = quadrature_encoder_get_count(pio, i);
                motorIndexToPosition[i] *= step;
                	
	        if (moving){
         		direction = (setPoint-startPoint != 0) ? ((setPoint-startPoint > 0) ? 1 : -1) : 0; 
            		if(direction!=0){

                                error = setPoint - motorIndexToPosition[0];

                                P = KP * error;
                                
                                if(direction == 1){             
                                        if(P >= setPoint-startPoint){
                                                P = MAX_PWM;
                                        }
                                        else{
                                                P = map(P,
                                                0, setPoint-startPoint,
                                                MIN_PWM, MAX_PWM);
                                        }
                                }
                                else if(direction == -1){         
                                        if(P <= setPoint-startPoint){
                                                P = MAX_PWM;
                                        }
                                        else{
                                                P = map(P,
                                                0, setPoint-startPoint,
                                                MIN_PWM, MAX_PWM);
                                        }
                                        P*=-1;
                                }
                                // P IS INVERTED; BECAUSE THE ENCODER SIGNALS A AND B ARE SWAPPED
                                driveMotor(0, -P, true); 
		        }
		        else{
		          moving = false;
		        }
         	}
                motorIndexToServoControlVariables[i][1] = direction;
                motorIndexToServoControlVariables[i][4] = error;
                motorIndexToServoControlVariables[i][5] = P;
 	}
}

//FIXME: FOR TESTING PHOTO COUPLES
void encoderCallback(uint gpio, uint32_t events) {   
    if (gpio == motorIndexToPins[1][2] || gpio == motorIndexToPins[1][3]){
        if(gpio==motorIndexToPins[1][2]){
          if (events == GPIO_IRQ_EDGE_RISE){
            gpio_put(18, 1);
          }
          else{
            gpio_put(18, 0);
          }
        }
        if(gpio==motorIndexToPins[1][3]){
          if (events == GPIO_IRQ_EDGE_RISE){
            gpio_put(19, 1);
          }
          else{
            gpio_put(19, 0);
          }
        }
    	
    }    
}

int main() {
   	stdio_init_all();
    	sleep_ms(2000);

	initPins();
	  
        //FIXME: FOR TESTING PHOTO COUPLES
        gpio_init(18);
        gpio_set_dir(18, GPIO_OUT);
        gpio_init(19);
        gpio_set_dir(19, GPIO_OUT);
	//FIXME: MOTOR DRIVER TEST
	driveMotor(0, 100, true);
	//driveMotor(1, 100, true);
	//driveMotor(2, 100, true);
	driveMotor(3, 100, true);
	  
	pio_add_program(pio, &quadrature_encoder_program);
        for (int i = 0; i < 4; i++) {
                quadrature_encoder_program_init(pio, i, motorIndexToEncoderPinAB[i], 0);
        }
 
        resetPosition();
 
    	while (true) {
        	// FIXME: PHOTO COUPLES TEST
        	printf("PIN 10: %d \n", gpio_get(10));
        	printf("PIN 11: %d \n", gpio_get(11));
        	sleep_ms(100);
        	
    	        // FIXME: ENCODER COUNT TEST
        	//motorIndexToPosition[0] = quadrature_encoder_get_count(pio, 0);
        	//printf("position %f\n", motorIndexToPosition[0]*step);
        	
        	// FIXME: KEYBOARD CONTROL TEST
        	//keyboardControl();
        	//sleep_ms(2000);
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

//  ---------------       VARIABLES       ---------------
void resetPosition(){
        for (int i = 0; i < 4; i++) {
                motorIndexToPosition[i] = 0;
        }
        // Resets the position counts in the pio machines
        for (int i = 0; i < 4; i++) {
                // Reset the encoder count
                reset_encoder_count(pio, i);
        }
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
                gpio_init(motorIndexToPins[i][2]);
                gpio_set_dir(motorIndexToPins[i][2], GPIO_IN);
                gpio_set_irq_enabled_with_callback(motorIndexToPins[i][2], GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoderCallback);
                // Set up the reading pin CHAN B
                gpio_init(motorIndexToPins[i][3]);
                gpio_set_dir(motorIndexToPins[i][3], GPIO_IN);
                gpio_set_irq_enabled_with_callback(motorIndexToPins[i][3], GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoderCallback);
        }
    
    //servo control interrupt 
    // the interrupt is with the pwm wrap frequency
    pwm_clear_irq(pwm_gpio_to_slice_num(motorIndexToPins[0][0]));
    pwm_set_irq_enabled(pwm_gpio_to_slice_num(motorIndexToPins[0][0]), true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, on_pwm_wrap);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    pwm_config config = pwm_get_default_config();
}

//-------------------------------

void keyboardControl(){
        int input = 0;
        printf("Enter angle motor index [0 || 1]: \n");
        input = getchar() - 48;

	if (input == 0) {
        	printf("FEMUR \n");
        		
		printf("Enter direction [0 || 1] (1 = -1): \n");
        	input = getchar() - 48;
		
 		input = (input == 0 || input == 1) ? ((input == 0) ? 1 : -1) : 0; 
	
		printf("Enter angle Xx: \n");
        	input = input*(getchar() - 48)*10;
        	
        	printf("Enter angle xX: \n");
        	input += getchar() - 48;

        	printf("Target angle: %d \n", input);
                
		startPoint = motorIndexToPosition[0];
		setPoint = startPoint + input; 
		if(MAX_ANGLE > setPoint && setPoint > MIN_ANGLE){
      	  		moving = true;
		}
		
                motorIndexToServoControlVariables[0][0] = moving;
                motorIndexToServoControlVariables[0][2] = setPoint;
                motorIndexToServoControlVariables[0][3] = startPoint;
	}
	sleep_ms(1111);
}
