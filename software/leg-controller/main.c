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

#define KP 5

//PWM 
uint freqHz = 10000;
uint wrapP = 12500;//311;//402kHz

//QUADRATURE
#define holes 38
// Old * 4 + New
int QEM [16] = {0,1,-1,2,-1,0,2,1,1,2,0,-1,2,-1,1,0};  // Quadrature Encoder Matrix
int oldState = 0;
int newState = 0;
float position = 0;
float step = 2.368421053;//360/(holes*4);

uint motorPins[2][2] = { 
  {0, 1}, 
  {8, 9}
};
  
//MOTOR
void driveMotor(int motorIndex, int driveValue, bool driveEnable);
void servoHoldMotor(int motorIndex, int setPoint);
//BASIC
long map(long x, long in_min, long in_max, long out_min, long outmax);

void gpio_callback(uint gpio, uint32_t events) {
    oldState = newState;
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
    newState = gpio_get(FEMUR_ENCODER_A_PIN)*2 + gpio_get(FEMUR_ENCODER_B_PIN);
    position += step * QEM[oldState*4+newState];
    //printf("Position: %f\n", position);
}

int setPoint, startPoint, direction;
float error, P;
bool moving = false;

void on_pwm_wrap() {
      	//printf("interrupt \n");
        // Clear the interrupt flag that brought us here
        pwm_clear_irq(pwm_gpio_to_slice_num(MOTOR_FEMUR_IN1_PIN));
	if (moving){			
 		direction = (setPoint != 0) ? ((setPoint > 0) ? 1 : -1) : 0;
  
   	 	printf("direction: %d \n", direction);
   	 	printf("setPoint: %d \n", setPoint);
   	 	printf("startPoint: %d \n", startPoint);
   	 	printf("POSITION: %f \n", position);
  		setPoint*=direction;
    
   	 	error = setPoint - direction*position;
   	 	printf("error: %f \n", error);
   	 	P = KP * error;
   	 	if(P >= setPoint){
   	         	P = MAX_PWM;
   	 	}
   	 	else{
   		     	P = map(P, startPoint-startPoint, setPoint-startPoint, MIN_PWM, MAX_PWM);
   		}
    		P*=direction;
 		printf("P: %f \n", P);
 		driveMotor(0, P, true); 
		
		if (error<=step*2){
			moving = false;
			//brake
 			printf("BRAKE\n");
			driveMotor(0, 0, true);
  			startPoint = position; 
		}
 	}
}

int main() {
    stdio_init_all();
    
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
    gpio_set_irq_enabled_with_callback(FEMUR_ENCODER_A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    //set up the reading pin CHAN B
    gpio_init(FEMUR_ENCODER_B_PIN);
    gpio_set_dir(FEMUR_ENCODER_B_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(FEMUR_ENCODER_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

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
    gpio_set_irq_enabled_with_callback(KNEE_ENCODER_A_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    //set up the reading pin CHAN B
    gpio_init(KNEE_ENCODER_B_PIN);
    gpio_set_dir(KNEE_ENCODER_B_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(KNEE_ENCODER_B_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
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
    
    const uint PIN_AB = 10;

    //servo control interrupt 
    pwm_clear_irq(pwm_gpio_to_slice_num(MOTOR_FEMUR_IN1_PIN));
    pwm_set_irq_enabled(pwm_gpio_to_slice_num(MOTOR_FEMUR_IN1_PIN), true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, on_pwm_wrap);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 32.f);
    pwm_init(pwm_gpio_to_slice_num(MOTOR_FEMUR_IN1_PIN), &config, true);
    
    //driveMotor(1, 100, true); 
    
    while (true) {
        printf("Enter angle Xx: \n");
        input = getchar() - 48;
        input *= 10;
        printf("Target angle: %d \n", input);

	setPoint = startPoint + input;        
	moving = true;

        sleep_ms(2000);
        printf("ADSFWFS \n");
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
		pwm_set_chan_level(femurSlice, PWM_CHAN_A, wrapP * (pwmIn1/100) );
                // pwmIn2
		pwm_set_chan_level(kneeSlice, PWM_CHAN_B, wrapP * (pwmIn2/100) );
        }
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
} 
