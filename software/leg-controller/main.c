#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
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

#define MAX_PWM 100
#define MIN_PWM 60

#define MAX_ANGLE 100
#define MIN_ANGLE 0

#define KP 5

//PWM 
uint freqHz = 10000;
uint wrapP = 12500;//311;//402kHz

//QUADRATURE
#define holes 38
// Old * 4 + New
int QEM [16] = {0,1,-1,2,-1,0,2,1,1,2,0,-1,2,-1,1,0};  // Quadrature Encoder Matrix
int oldState0 = 0;
int newState0 = 0;
float position0 = 0;
int oldState1 = 0;
int newState1 = 0;
float position1 = 0;
float step = 2.368421053;//360/(holes*4);

uint motorPins[2][2] = { 
  {1, 0}, //reversed
  {8, 9}
};
  
//MOTOR
void driveMotor(int motorIndex, int driveValue, bool driveEnable);
void servoHoldMotor(int motorIndex, int setPoint);
//BASIC
long map(long x, long in_min, long in_max, long out_min, long outmax);

int hole, prevS;

float errorMargin = 0;

int direction0;
float setPoint0, startPoint0, error0, P0;
bool moving0 = false;
int setPoint1, startPoint1, direction1;
float error1, P1;
bool moving1 = false;

void resetPosition(){
	position0 = 0;
	position1 = 0;
}

void encoderCallback(uint gpio, uint32_t events) {
    if (gpio == FEMUR_ENCODER_A_PIN || gpio == FEMUR_ENCODER_B_PIN){
        if(gpio==FEMUR_ENCODER_A_PIN){
          if (events == GPIO_IRQ_EDGE_RISE){
            gpio_put(13, 1);
          }
          else{
            gpio_put(13, 0);
          }
        }
        if(gpio==FEMUR_ENCODER_B_PIN){
          if (events == GPIO_IRQ_EDGE_RISE){
            gpio_put(7, 1);
          }
          else{
            gpio_put(7, 0);
          }
        }
        
	oldState0 = newState0;
    	newState0 = gpio_get(FEMUR_ENCODER_A_PIN)*2 + gpio_get(FEMUR_ENCODER_B_PIN);
    	position0 += step * QEM[oldState0*4+newState0];
	//hole += -1*QEM[oldState0*4+newState0];
    	//printf("hole: %d\n", hole);
    	printf("Position0: %f\n", position0);
    }    
    if (gpio == KNEE_ENCODER_A_PIN || gpio == KNEE_ENCODER_B_PIN){
    	oldState1 = newState1;
	//The encoder that is clockwise left (this case 10 o'clock) need to be first
	//the secont encoder (pin a) is at 12 o'clock
    	newState1 = gpio_get(KNEE_ENCODER_B_PIN)*2 + gpio_get(KNEE_ENCODER_A_PIN);
    	position1 += step * QEM[oldState1*4+newState1];
    	printf("Position1: %f\n", position1);
    }    
}

void on_pwm_wrap() {
//NO PRINT F INSIDE OF THIS FUNCTION!
//there is not enough time to execute and clogs the program!

        // Clear the interrupt flag that brought us here
        pwm_clear_irq(pwm_gpio_to_slice_num(MOTOR_FEMUR_IN1_PIN));
	if (moving0){
 		direction0 = (setPoint0-startPoint0 != 0) ? ((setPoint0-startPoint0 > 0) ? 1 : -1) : 0; 
    		if(direction0!=0){
		  
                        //setPoint0*=direction0;

                        error0 = setPoint0 - position0;// direction0*position0;

                        P0 = KP * error0;
                        
                        if(direction0 == 1){             
                                if(P0 >= setPoint0-startPoint0){
                                        P0 = MAX_PWM;
                                }
                                else{
                                        P0 = map(P0,
                                        0, setPoint0-startPoint0,
                                        MIN_PWM, MAX_PWM);
                                }
                        }
                        else if(direction0 == -1){         
                                if(P0 <= setPoint0-startPoint0){
                                        P0 = MAX_PWM;
                                }
                                else{
                                        P0 = map(P0,
                                        0, setPoint0-startPoint0,
                                        MIN_PWM, MAX_PWM);
                                }
                                P0*=-1;
                        }
                        
                        //printf("stP: %d sP%d dir %d er %f P %f\n", startPoint0, setPoint0, direction0, error0, P0);

                        driveMotor(0, P0, true); 

                        if (error0*direction0<=step){
                                printf("brake\n");
                                moving0 = false;
                                //brake
                                driveMotor(0, 0, true);
                        }
		}
		else{
		  moving0 = false;
		}
 	}
	if (moving1){
 		direction1 = (setPoint1-startPoint1 != 0) ? ((setPoint1-startPoint1 > 0) ? 1 : -1) : 0; 
    		if(direction1!=0){
		  
                        //setPoint0*=direction0;

                        error1 = setPoint1 - position1;// direction0*position0;

                        P1 = KP * error1;
                        
                        if(direction1 == 1){             
                                if(P1 >= setPoint1-startPoint1){
                                        P1 = MAX_PWM;
                                }
                                else{
                                        P1 = map(P1,
                                        0, setPoint1-startPoint1,
                                        MIN_PWM, MAX_PWM);
                                }
                        }
                        else if(direction1 == -1){         
                                if(P1 <= setPoint1-startPoint1){
                                        P1 = MAX_PWM;
                                }
                                else{
                                        P1 = map(P1,
                                        0, setPoint1-startPoint1,
                                        MIN_PWM, MAX_PWM);
                                }
                                P1*=-1;
                        }
                        
                        //printf("stP: %d sP%d dir %d er %f P %f\n", startPoint0, setPoint0, direction0, error0, P0);

                        driveMotor(1, P1, true); 

                        if (error1*direction1<=step){
                                printf("brake\n");
                                moving1 = false;
                                //brake
                                driveMotor(1, 0, true);
                        }
		}
		else{
		  moving1 = false;
		}	
 	}
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    
    //MOTOR DRIVER 1
    // set up pwm on GPIO MOTOR_DRIVER_IN1
    gpio_set_function(MOTOR_FEMUR_IN1_PIN, GPIO_FUNC_PWM);
    // enable PWM on that channel
    pwm_set_enabled(pwm_gpio_to_slice_num(MOTOR_FEMUR_IN1_PIN), true);
    // set wrap point
    pwm_set_wrap(pwm_gpio_to_slice_num(MOTOR_FEMUR_IN1_PIN), wrapP);
    
    // set up pwm on GPIO MOTOR_DRIVER_IN2
    gpio_set_function(MOTOR_FEMUR_IN2_PIN, GPIO_FUNC_PWM);
    // enable PWM on that channel
    pwm_set_enabled(pwm_gpio_to_slice_num(MOTOR_FEMUR_IN2_PIN), true);
    // set wrap point
    pwm_set_wrap(pwm_gpio_to_slice_num(MOTOR_FEMUR_IN2_PIN), wrapP);
    
    //VREF driver
    gpio_init(MOTOR_FEMUR_VREF_PIN);
    gpio_set_dir(MOTOR_FEMUR_VREF_PIN, GPIO_OUT);
    gpio_put(MOTOR_FEMUR_VREF_PIN, 1);
    
    //ENCODER 1
    //set up the reading pin CHAN A
    gpio_init(FEMUR_ENCODER_A_PIN);
    gpio_set_dir(FEMUR_ENCODER_A_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(FEMUR_ENCODER_A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoderCallback);
    //set up the reading pin CHAN B
    gpio_init(FEMUR_ENCODER_B_PIN);
    gpio_set_dir(FEMUR_ENCODER_B_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(FEMUR_ENCODER_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoderCallback);

    //MOTOR DRIVER 2
    // set up pwm on GPIO MOTOR_DRIVER_IN1
    gpio_set_function(MOTOR_KNEE_IN1_PIN, GPIO_FUNC_PWM);
    // get PWM channel for that pin
    //uint slice_num2 = pwm_gpio_to_slice_num(MOTOR_KNEE_IN1_PIN);
    // enable PWM on that channel
    pwm_set_enabled(pwm_gpio_to_slice_num(MOTOR_KNEE_IN1_PIN), true);
    // set wrap point
    pwm_set_wrap(pwm_gpio_to_slice_num(MOTOR_KNEE_IN1_PIN), wrapP);

    // set up pwm on GPIO MOTOR_DRIVER_IN2
    gpio_set_function(MOTOR_KNEE_IN2_PIN, GPIO_FUNC_PWM);
    // get PWM channel for that pin
    //uint slice_num3 = pwm_gpio_to_slice_num(MOTOR_KNEE_IN2_PIN);
    // enable PWM on that channel
    pwm_set_enabled(pwm_gpio_to_slice_num(MOTOR_KNEE_IN2_PIN), true);
    // set wrap point 
    pwm_set_wrap(pwm_gpio_to_slice_num(MOTOR_KNEE_IN2_PIN), wrapP);
    
    //ENCODER 2
    gpio_init(KNEE_ENCODER_3V_PIN);
    gpio_set_dir(KNEE_ENCODER_3V_PIN, GPIO_OUT);
    gpio_put(KNEE_ENCODER_3V_PIN, 1);
    //set up the reading pin CHAN A
    gpio_init(KNEE_ENCODER_A_PIN);
    gpio_set_dir(KNEE_ENCODER_A_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(KNEE_ENCODER_A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoderCallback);
    //set up the reading pin CHAN B
    gpio_init(KNEE_ENCODER_B_PIN);
    gpio_set_dir(KNEE_ENCODER_B_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(KNEE_ENCODER_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoderCallback);
    //3V pin
    gpio_init(KNEE_ENCODER_3V_PIN);
    gpio_set_dir(KNEE_ENCODER_3V_PIN, GPIO_OUT);
    gpio_put(KNEE_ENCODER_3V_PIN, 1);
    //digital output check
    gpio_init(13);
    gpio_set_dir(13, GPIO_OUT);
    //digital output check
    gpio_init(7);
    gpio_set_dir(7, GPIO_OUT);
    
    int input = 0;

    //servo control interrupt 
    // the interrupt is with the pwm wrap frequency
    pwm_clear_irq(pwm_gpio_to_slice_num(MOTOR_FEMUR_IN1_PIN));
    pwm_set_irq_enabled(pwm_gpio_to_slice_num(MOTOR_FEMUR_IN1_PIN), true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, on_pwm_wrap);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    pwm_config config = pwm_get_default_config();
    //no need to init the pwm slice again (breaks the pwm)
    //DEBUG ONLY
    //pwm_config_set_clkdiv(&config, 32.f);
    //pwm_init(pwm_gpio_to_slice_num(MOTOR_FEMUR_IN1_PIN), &config, true);

    //sleep_ms(3000);
    //driveMotor(0, 70, true); 
    //driveMotor(1, 100, true); 
   
	resetPosition();
 
    while (true) {
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
        	
		startPoint0 = position0;
		setPoint0 = startPoint0 + input; 
		if(MAX_ANGLE > setPoint0 && setPoint0 > MIN_ANGLE){
      	  		moving0 = true;
		}
	}
	else if(input == 1) {
        	printf("KNEE \n");
        	
		printf("Enter direction [0 || 1] (1 = -1): \n");
        	input = getchar() - 48;
		
 		input = (input == 0 || input == 1) ? ((input == 0) ? 1 : -1) : 0; 
	
		printf("Enter angle Xx: \n");
        	input = input*(getchar() - 48)*10;
        	
        	printf("Enter angle xX: \n");
        	input += getchar() - 48;

        	printf("Target angle: %d \n", input);
        	
		startPoint1 = position1;
		setPoint1 = startPoint1 + input; 
		if(MAX_ANGLE > setPoint1 && setPoint1 > MIN_ANGLE){
      	  		moving1 = true;
		}
	}

	sleep_ms(1111);
    } 
}

void driveMotor(int motorIndex, int driveValue, bool driveEnable){
        // Get target PWM slices
        uint femurSlice = pwm_gpio_to_slice_num(motorPins[motorIndex][0]);
        uint kneeSlice = pwm_gpio_to_slice_num(motorPins[motorIndex][1]);
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
