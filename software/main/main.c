#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "pico/binary_info.h"
#include "pico/cyw43_arch.h"

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

// TIMER IRQ
#define ALARM_NUM 0
#define ALARM_IRQ TIMER_IRQ_0

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
// Old * 4 + New
//state machine
uint8_t QEM [16] = {0,1,-1,2,-1,0,2,1,1,2,0,-1,2,-1,1,0};  // Quadrature Encoder Matrix
//FIXME
float step = 2.368421053;//360/(holes*4);
	
//SERVO CONTROL
#define KP 5

struct axis {
        int pinA;
        int pinB;
        
        // Counts the encoder changes in total. Can be used to calculate absolute position since start
        uint8_t count;
        uint8_t oldState;
        uint8_t newState;

        uint8_t moving;
        uint8_t direction;
        uint8_t setPoint;
        uint8_t startPoint;
        uint8_t error;
        uint8_t P;
}
// Motor index to struct
struct axis axes[4];

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
        
        struct axis *p;
        p->moving
        for (int i = 0; i < 4; i++) {
                p = axes+i;
                  
                oldState = newState;
        	newState = gpio_get(motorIndexToPins[i][3])*2 + gpio_get(motorIndexToPins[i][2]);
        	
        	//illegal case check
        	if (QEM[ oldState*4 + newState ] == 2) {
                    //brake
                    moving = false;
                    driveMotor(i, 0, true);
        	}
        	else {
                	position += QEM[oldState0*4+newState0];
        	}
        }
        
        // FIXME: POSITION GETTING
        for (int i = 0; i < 4; i++) {
                int d = 0;
                while(!pio_sm_is_rx_fifo_empty(pio, i))
                        d +=  pio_sm_get(pio, i);
                
                //uses get blocking
                //motorIndexToPosition[i] = quadrature_encoder_get_count(pio, i) * step;
        }
        
        
        for (int i = 0; i < 4; i++) {
                moving = motorIndexToServoControlVariables[i][0];
                direction = motorIndexToServoControlVariables[i][1];
                setPoint = motorIndexToServoControlVariables[i][2];
                startPoint = motorIndexToServoControlVariables[i][3];
                error = motorIndexToServoControlVariables[i][4];
                P = motorIndexToServoControlVariables[i][5];
                
                motorIndexToPosition[i] = quadrature_encoder_get_count(pio, i); // FIXME:
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
                                //(FALSE) P IS INVERTED; BECAUSE THE ENCODER SIGNALS A AND B ARE SWAPPED
                                driveMotor(i, P, true); 
		        }
		        else{
                          //driveMotor(i, 0, true); 
		          moving = false;
		        }
         	}
	        else{
                        driveMotor(i, 0, true); 
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
            gpio_put(20, 1);
          }
          else{
            gpio_put(20, 0);
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
    	
    	//LED
        //if (cyw43_arch_init()) {
        //    printf("Wi-Fi init failed");
        //    return -1;
        //}
        //cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        
	//gpio_init(25);
    	//gpio_set_dir(25, GPIO_OUT);
    	//gpio_put(25, 1);

	initPins();
	  
        //FIXME: FOR TESTING PHOTO COUPLES
        gpio_init(20);
        gpio_set_dir(20, GPIO_OUT);
        gpio_init(19);
        gpio_set_dir(19, GPIO_OUT);
	//FIXME: MOTOR DRIVER TEST
	//pwm_set_chan_level(pwm_gpio_to_slice_num(0), PWM_CHAN_A, wrapP * (70/100) );
	//pwm_set_chan_level(pwm_gpio_to_slice_num(1), PWM_CHAN_B, wrapP * (40/100) );
	//driveMotor(0, 100, true);
	//driveMotor(1, 100, true);
	//driveMotor(2, 100, true);
	driveMotor(3, 100, true);
	  
	pio_add_program(pio, &quadrature_encoder_program);
        for (int i = 0; i < 4; i++) {
                quadrature_encoder_program_init(pio, i, motorIndexToEncoderPinAB[i], 0);
        }
 
        resetPosition();
        
        // Start the timer IRQ
        alarm_fired = false;
        // Set alarm freq at 15 Hz
        alarm_in_us(66667);
 
      	//FIXME: SERVO CONTROL TEST
	startPoint = motorIndexToPosition[0];
	setPoint = startPoint + 90; 
	if(MAX_ANGLE > setPoint && setPoint > MIN_ANGLE){
    	  		moving = true;
	}
              motorIndexToServoControlVariables[1][0] = moving;
              motorIndexToServoControlVariables[1][2] = setPoint;
              motorIndexToServoControlVariables[1][3] = startPoint;
                
    	while (true) {
        	// FIXME: PHOTO COUPLES TEST
        	//printf("PIN 10: %d \n", gpio_get(10));
        	//printf("PIN 11: %d \n", gpio_get(11));
        	//sleep_ms(100);
        	
    	        // FIXME: ENCODER COUNT TEST
        	//motorIndexToPosition[0] = quadrature_encoder_get_count(pio, 0);
        	//printf("position %f\n", motorIndexToPosition[0]*step);
        	
        	// FIXME: KEYBOARD CONTROL TEST
        	//keyboardControl();
        	//sleep_ms(2000);
        	
        	// FIXME: FEMNUR TEST
        	//driveMotor(2, -100, true);
        	//sleep_ms(350);
        	//driveMotor(2, 0, true);
        	//driveMotor(2, 100, true);
        	//sleep_ms(350);
        	//driveMotor(2, 0, true);
        	
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
//  ---------------         TIMER         ---------------
static uint64_t get_time(void) {
    // Reading low latches the high value
    uint32_t lo = timer_hw->timelr;
    uint32_t hi = timer_hw->timehr;
    return ((uint64_t) hi << 32u) | lo;
}
static void alarm_in_us(uint32_t delay_us) {
    // Enable the interrupt for our alarm (the timer outputs 4 alarm irqs)
    hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM);
    // Set irq handler for alarm irq
    irq_set_exclusive_handler(ALARM_IRQ, alarm_irq);
    // Enable the alarm irq
    irq_set_enabled(ALARM_IRQ, true);
    // Enable interrupt in block and at processor

    // Alarm is only 32 bits so if trying to delay more
    // than that need to be careful and keep track of the upper
    // bits
    uint64_t target = timer_hw->timerawl + delay_us;

    // Write the lower 32 bits of the target time to the alarm which
    // will arm it
    timer_hw->alarm[ALARM_NUM] = (uint32_t) target;
}
static void alarm_irq(void) {
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM);

    // Assume alarm 0 has fired
    alarm_fired = true;
    
    //-------------------------
    //for (int i = 0; i < 4; i++) {
    //        motorIndexToPosition[i] = quadrature_encoder_get_count(pio, i) * step;
    //}
    //printf("front left knee angle: %f \n", motorIndexToPosition[1]);
    //printf("Timer IRQ \n");
    //-------------------------
    
    // Set timer again
    alarm_fired = false;
    // Set alarm freq at 15 Hz
    alarm_in_us(66667);
}
