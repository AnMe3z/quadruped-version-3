#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "pico/binary_info.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

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

//  MOTOR INDEXES
#define FRONT_FEMUR 0
#define FRONT_KNEE 1
#define BACK_FEMUR 2
#define BACK_KNEE 3

//  MIN/MAX PWM MOTOR 
#define MAX_PWM 100
#define MIN_PWM 60

//  JOINT ROM
#define MAX_ANGLE 100
#define MIN_ANGLE 0

// UDP CONNECTION
#define DEVICE_ID 111
#define UDP_PORT 4444
#define BEACON_MSG_LEN_MAX 127
#define BEACON_TARGET "255.255.255.255"
#define BEACON_INTERVAL_MS 1000

// TIMER IRQ
#define ALARM_NUM 0
#define ALARM_IRQ TIMER_IRQ_0

//  PROPORTIONAL CONTROL COEFFICIENT
#define KP 5

//  QUADRATURE
#define holes 38
//  State machine
int QEM [16] = {0,1,-1,2,-1,0,2,1,1,2,0,-1,2,-1,1,0};  // Quadrature Encoder Matrix | Old * 4 + New
float step = 2.368421053;

//  PWM frequency and timer wrap point
uint freqHz = 10000;
uint wrapP = 12500;

// Alarm interrupt handler
static volatile bool alarm_fired;

PIO pio = pio0;

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
  
// Functions
// Pin init
void initPins();
// Motor control
void driveMotor(int motorIndex, int driveValue, bool driveEnable);
// Reset position variables
void resetPosition();
// Basic utils
long map(long x, long in_min, long in_max, long out_min, long outmax);
// Timer interrupt
static uint64_t get_time(void);
static void alarm_irq(void);
static void alarm_in_us(uint32_t delay_us);
// PWM interrupt
void on_pwm_wrap() {
        // NO PRINT F INSIDE OF THIS FUNCTION!
        // There is not enough time to execute and clogs the program!

        // Clear the interrupt flag that brought us here
        pwm_clear_irq(pwm_gpio_to_slice_num(MOTOR_FEMUR_IN1_PIN));
        
        for (int i = 0; i < 4; i++) {
                moving = motorIndexToServoControlVariables[i][0];
                direction = motorIndexToServoControlVariables[i][1];
                setPoint = motorIndexToServoControlVariables[i][2];
                startPoint = motorIndexToServoControlVariables[i][3];
                error = motorIndexToServoControlVariables[i][4];
                P = motorIndexToServoControlVariables[i][5];
                
                if (moving){
         		direction = (setPoint-startPoint != 0) ? ((setPoint-startPoint > 0) ? 1 : -1) : 0; 
            		if(direction != 0){

                                error = setPoint - position;

                                P = KP * error;
                                
                                if(direction == 1){             
                                        if(P >= setPoint - startPoint){
                                                P = MAX_PWM;
                                        }
                                        else{
                                                P = map(P,
                                                0, setPoint-startPoint,
                                                MIN_PWM, MAX_PWM);
                                        }
                                }
                                else if(direction == -1){         
                                        if(P <= setPoint - startPoint){
                                                P = MAX_PWM;
                                        }
                                        else{
                                                P0 = map(P,
                                                0, setPoint-startPoint,
                                                MIN_PWM, MAX_PWM);
                                        }
                                        P*=-1;
                                }

                                driveMotor(i, P, true); 
		        }
		        else{
		          moving = false;
		        }
		        
		        //write back data
		        motorIndexToServoControlVariables[i][0] = moving;
                        motorIndexToServoControlVariables[i][1] = direction;
                        motorIndexToServoControlVariables[i][2] = setPoint;
                        motorIndexToServoControlVariables[i][3] = startPoint;
                        motorIndexToServoControlVariables[i][4] = error;
                        motorIndexToServoControlVariables[i][5] = P;

 	        }
        }
        
        moving = 0;
        direction = 0;
        setPoint = 0;
        startPoint = 0;
        error = 0;
        P = 0;
	
}

void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    // This function will be called when a UDP packet is received

    if (p != NULL) {
        // The pbuf (p) contains the received data.
        char *received_data = (char*)p->payload;

        cJSON *message_json = cJSON_Parse(received_data);
        if (message_json == NULL) {
            // Free the JSON object
            cJSON_Delete(message_json);
            // Don't forget to free the pbuf
            pbuf_free(p);
            return;
        }

        cJSON *receivers = cJSON_GetObjectItem(message_json, "receivers");
        if (receivers == NULL) {
            // Free the JSON object
            cJSON_Delete(message_json);
            // Don't forget to free the pbuf
            pbuf_free(p);
            return;
        }

        cJSON *receiver;
        cJSON_ArrayForEach(receiver, receivers) {
            cJSON *receiver_id = cJSON_GetObjectItem(receiver, "receiver_id");
            if (receiver_id == NULL) {
                // Free the JSON object
                cJSON_Delete(message_json);
                // Don't forget to free the pbuf
                pbuf_free(p);
                return;
            }

            // Check if the receiver_id matches the ID of this device
            if (strcmp(receiver_id->valuestring, DEVICE_ID) == 0) {
                cJSON *motor_commands = cJSON_GetObjectItem(receiver, "motor_commands");
                if (motor_commands == NULL) {
                    // Free the JSON object
                    cJSON_Delete(message_json);
                    // Don't forget to free the pbuf
                    pbuf_free(p);
                    return;
                }

                cJSON *motor_command;
                cJSON_ArrayForEach(motor_command, motor_commands) {
                    cJSON *motor_id = cJSON_GetObjectItem(motor_command, "motor_id");
                    if (motor_id == NULL) {
                        // Free the JSON object
                        cJSON_Delete(message_json);
                        // Don't forget to free the pbuf
                        pbuf_free(p);
                        return;
                    }

                    cJSON *degree = cJSON_GetObjectItem(motor_command, "degree");
                    if (degree == NULL) {
                        // Free the JSON object
                        cJSON_Delete(message_json);
                        // Don't forget to free the pbuf
                        pbuf_free(p);
                        return;
                    }

                    // Convert the motor_id and degree to integers
                    int motor_id_int = atoi(motor_id->valuestring);
                    int degree_int = degree->valueint;

		    int startPoint = motorIndexToPosition[motor_id_int];
                    motorIndexToServoControlVariables[motor_id_int][0] = 1;
                    motorIndexToServoControlVariables[motor_id_int][2] = startPoint + degree_int;
                    motorIndexToServoControlVariables[motor_id_int][3] = startPoint;
                }
            }
        }

        // Free the JSON object
        cJSON_Delete(message_json);
        // Don't forget to free the pbuf
        pbuf_free(p);
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    
    initPins();
    
    pio_add_program(pio, &quadrature_encoder_program);
    for (int i = 0; i < 4; i++) {
            quadrature_encoder_program_init(pio, i, motorIndexToPins[i][2], 0);
    }
   
    resetPosition();
 
    // Create a new UDP control block
    struct udp_pcb* pcb = udp_new();
    // Declare target listen adder
    ip_addr_t addr;
    ipaddr_aton(BEACON_TARGET, &addr);
    
    if (pcb == NULL) {
        printf("udp_new failed!\n");
        return -1;
    }

    // Bind the control block to any IP address and the port you want to listen on
    if (udp_bind(pcb, IP_ADDR_ANY, MY_PORT) != ERR_OK) {
        printf("udp_bind failed!\n");
        return -1;
    }

    // Set the receive callback for the control block
    udp_recv(pcb, udp_receive_callback, NULL);
 
    while (true) {
        tight_loop_contents();
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

//  ---------------         MOTORS         ---------------
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
		pwm_set_chan_level(femurSlice, PWM_CHAN_A, wrapP * (pwmIn1/100) );
		pwm_set_chan_level(kneeSlice, PWM_CHAN_B, wrapP * (pwmIn2/100) );
        }
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
    
    for (int i = 0; i < 4; i++) {
            motorIndexToPosition[i] = quadrature_encoder_get_count(pio, i) * step;
    }
    
    // Set timer again
    alarm_fired = false;
    // Set alarm freq at 15 Hz
    alarm_in_us(66667);
}

//  ---------------       VARIABLES       ---------------
void resetPosition(){
        for (int i = 0; i < 4; i++) {
                motorIndexToPosition[i] = 0;
        }
}

//  ---------------         UTILS         ---------------
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
} 


