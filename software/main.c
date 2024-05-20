/*
 *
 * Copyright (c) 2024 Andrey Ezhkov
 *
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "pico/binary_info.h"
#include "pico/cyw43_arch.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"

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

//SERVO CONTROL
#define KP 5

#define UDP_SERVER_PORT 12345
#define BEACON_MSG_LEN_MAX 127
#define BEACON_TARGET "255.255.255.255"
#define BEACON_INTERVAL_MS 1000

#define DEVICE_ID 1 // or 1

#if DEVICE_ID == 0
        #define MAX_ANGLE -42
#else
        #define MAX_ANGLE 42 // max angle = 99.47368423
#endif
#define MIN_ANGLE 0
//PWM 
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
int QEM [16] = {0,-1,1,2,1,0,2,-1,-1,2,0,1,2,1,-1,0}; // Quadrature Encoder Matrix // Old * 4 + New //state machine

/* struct axis
 * /brief handles variable for a motor
 *
 * keeps track of its position via count. keeps track of its P control variables via 6 variables
 *
 */
struct axis {
        // Counts the encoder changes in total. Can be used to calculate absolute position since start
        int count;
        int oldState;
        int newState;

        // P control
        uint8_t moving;
        int direction;
        int setPoint;
        int startPoint;
        int error;
        int P;
};
// Motor index to struct
struct axis axes[4];
struct axis *p;

int led;
int input_data[26]; 

/* long map()
 * /brief basic map function
 * 
 */
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
} 


/* void driveMotor()
 * /brief controls a motor based on 3 inputs
 * 
 * this function decides the target motor, considers the enable value and calculates output PWM signals that are then written to the output control pins
 * 
 */
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
                // pwmIn1
		pwm_set_chan_level(femurSlice, PWM_CHAN_A, wrapP * (pwmIn1/100) );
                // pwmIn2
		pwm_set_chan_level(kneeSlice, PWM_CHAN_B, wrapP * (pwmIn2/100) );
        }
}

/* void process_data()
 * /brief decodes the message into tasks for the motors
 * 
 * based on the control message structure this code decodes the input_data[] array (received data) into instructions for each motor
 * 
 */
void process_data(){
        int dir;
        struct axis *j;
        
        if (DEVICE_ID == 0) {
                for (int i = 0; i < 4; i++) {
                        //direction on the left side is  inverted
                        //level inner servo
                        dir = (input_data[(i*3) + 1] == 1) ? 1 : -1; 
                
                        j = axes+i;
                        
            	        j->startPoint = j->count;
                        j->setPoint = j->startPoint + (dir * (10*input_data[(i*3) + 2] + input_data[(i*3) + 3])); 
                        
                        //direction on the left side is inverted
                        if(MAX_ANGLE < j->setPoint && j->setPoint < MIN_ANGLE){
	                        j->moving = true;
                                // DEBUG
                                //if ( opt.verbose && opt.verboseDataRCV){ printf("moving \n"); }
                                printf("moving \n");
                        }
                }
        }
        else {
                for (int i = 0; i < 4; i++) {
                        
                        dir = (input_data[(i*3) + 1 + 13] == 1) ? -1 : 1; 
                
                        j = axes+i;
                        
            	        j->startPoint = j->count;
                        j->setPoint = j->startPoint + (dir * (10*input_data[(i*3) + 2 + 13] + input_data[(i*3) + 3 + 13])); 
                        
                        printf("dir * (10*input_data[(i*3) + 2 + 13] + input_data[(i*3) + 3 + 13]) %d \n", dir * (10*input_data[(i*3) + 2 + 13] + input_data[(i*3) + 3 + 13]) );
                        printf("j->setPoint %d \n", j->setPoint);
                                
                        if(MAX_ANGLE > j->setPoint && j->setPoint > MIN_ANGLE){
                                j->moving = true;
                                printf("moving \n");
                        }
                        
                }
        }
        
}

/* void udp_receive_callback()
 * /brief handles the received data from UDP
 * 
 * this function is being called every time the poll has detected anything sent via UDP
 * checks if the message is the right length. parses it to an array input_data[]. then switches the state of a LED and frees the buffer
 * 
 */
void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
        if (p != NULL) {
                // Received packet buffer 'p' is not NULL
                // You can process the packet here
                char *packet_data = (char *)p->payload;
                
                if (strlen(packet_data) == 52) {
                        int i;
                        
                        while (packet_data[i] != '\0' && packet_data[i+1] != '\0') {
                                input_data[i/2] = ((packet_data[i] - '0') * 10 + (packet_data[i+1] - '0')) - 30;
                                printf("%d\n", input_data[i/2]);
                                i += 2;
                        }
                        
                        process_data();
                }
                
                // Toggle led
                led = led ^ 1;
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led);

                // Don't forget to free the packet buffer
                pbuf_free(p);
        }
}

/* void on_pwm_wrap()
 * /brief inner control loop and encoder assesment
 * 
 * this function is being called with the frequency of the PWM (10KHz)
 * firstly it clears the interrupt flag
 * then it runs a for loop for the each of the 4 encoders. inside it calculates if there are any changes to the recorded position via the encoder matrix
 * then it runs a for loop for the each of the 4 motors. this is the P control. inside it calculates if there are any changes to the set point of the motor and if
 * it calculates a error and P change which is being sent to the motor via PWM
 * 
 */
void on_pwm_wrap() {
//NO PRINT F INSIDE OF THIS FUNCTION!
//there is not enough time to execute and clogs the program!

        // Clear the interrupt flag that brought us here
        pwm_clear_irq(pwm_gpio_to_slice_num(motorIndexToPins[0][0]));
        
        for (int i = 0; i < 4; i++) {
                p = axes+i;
                
                p->oldState = p->newState;
        	p->newState = gpio_get(motorIndexToPins[i][3])*2 + gpio_get(motorIndexToPins[i][2]);
        	
        	//illegal case check
        	if (QEM[ p->oldState*4 + p->newState ] == 2) {
                        //brake
                        p->moving = false;
                        driveMotor(i, 0, true);
        	}
        	else {  
                	p->count += QEM[p->oldState*4 + p->newState]; 
        	}
        }
        
        for (int i = 0; i < 4; i++) {
        //XXX INTERNAL SERVO CONTROL LOOP NOW WORKS WITH COUNT NOT EXACT ANGLE POSITION
                p = axes+i;
                
	        if ( p->moving ){
         		p->direction = (p->setPoint - p->startPoint != 0) ? ((p->setPoint - p->startPoint > 0) ? 1 : -1) : 0; 
            		if(p->direction != 0){

                                p->error = p->setPoint - p->count;

                                p->P = KP * p->error;
                                
                                if(p->direction == 1){             
                                        if(p->P >= p->setPoint - p->startPoint){
                                                p->P = MAX_PWM;
                                        }
                                        else{
                                                p->P = map(p->P,
                                                0, p->setPoint - p->startPoint,
                                                MIN_PWM, MAX_PWM);
                                        }
                                }
                                else if(p->direction == -1){         
                                        if(p->P <= p->setPoint - p->startPoint){
                                                p->P = MAX_PWM;
                                        }
                                        else{
                                                p->P = map(p->P,
                                                0, p->setPoint - p->startPoint,
                                                MIN_PWM, MAX_PWM);
                                        }
                                        p->P*=-1;
                                }
                                //if ( !opt.disableMovement ){
                                        driveMotor(i, p->P, true); 
                                //}
		        }
		        else{
                          driveMotor(i, 0, true); 
		          p->moving = false;
		        }
         	}
	        else{
                        driveMotor(i, 0, true); 
	        }
 	}
}

/* void initPins()
 * /brief initializes the pins
 * 
 * the function runs a for loop 4 times as for the number of motors. for each motor initializes 2 pins with 10KHz PWM for output control. then initializes 2 more pins as input for the encoder
 * at last creates a PWM interrupt that fires on the PWM frequency and appoints on_pwm_wrap() to be executed on each IRQ
 * 
 */
void initPins(){

        for (int i = 0; i < 4; i++) {
                // Motor control pins
                // Set up pwm on GPIO pin for IN1
                gpio_set_function(motorIndexToPins[i][0], GPIO_FUNC_PWM);
                // Enable PWM on that channel
                pwm_set_enabled(pwm_gpio_to_slice_num(motorIndexToPins[i][0]), true);
                // Set wrap point
                pwm_set_wrap(pwm_gpio_to_slice_num(motorIndexToPins[i][0]), wrapP);
                
                // Set up pwm on GPIO pin for IN2
                gpio_set_function(motorIndexToPins[i][1], GPIO_FUNC_PWM);
                // Enable PWM on that channel
                pwm_set_enabled(pwm_gpio_to_slice_num(motorIndexToPins[i][1]), true);
                // Set wrap point
                pwm_set_wrap(pwm_gpio_to_slice_num(motorIndexToPins[i][1]), wrapP);
                
                // Encoder pubs
                // Set up the reading pin CHAN A
                gpio_init(motorIndexToPins[i][2]);
                gpio_set_dir(motorIndexToPins[i][2], GPIO_IN);
                // Set up the reading pin CHAN B
                gpio_init(motorIndexToPins[i][3]);
                gpio_set_dir(motorIndexToPins[i][3], GPIO_IN);
        }
    
        //servo control interrupt 
        // the interrupt is with the pwm wrap frequency
        pwm_clear_irq(pwm_gpio_to_slice_num(motorIndexToPins[0][0]));
        pwm_set_irq_enabled(pwm_gpio_to_slice_num(motorIndexToPins[0][0]), true);
        irq_set_exclusive_handler(PWM_IRQ_WRAP, on_pwm_wrap);
        irq_set_enabled(PWM_IRQ_WRAP, true);
        pwm_config config = pwm_get_default_config();
}

/* int init_wireless()
 * /brief initializes the wireless interface
 * 
 * calls cyw43_arch_init() which inits the wireless hardware and returns an error code. 
 * then enables the wireless ap via cyw43_arch_enable_sta_mode(). cyw43_arch_wifi_connect_timeout_ms() tries to connect to a wifi network with a name and a password.
 * an LED lights up if its connected. the code then creates an udp instance via udp_new().
 * finally a connection is made to broadcast (255.255.255.255) and the protocol is being binded. udp_receive_callback() is being appointed to be called when there is udp activity.
 * 
 */
int init_wireless(){
        if (cyw43_arch_init()) {
                printf("failed to initialise\n");
                return 1;
        }

        cyw43_arch_enable_sta_mode();

        if (cyw43_arch_wifi_connect_timeout_ms("andrey_shefa", WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
                return 1;
        } else {
                led = led ^ 1;
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led);
        }

        struct udp_pcb *udp_server_pcb = udp_new();
        if (udp_server_pcb == NULL) {
                return -1;
        }

        ip_addr_t ipaddr;
        IP4_ADDR(&ipaddr, 255,255,255,255); // Broadcast address

        err_t err = udp_bind(udp_server_pcb, &ipaddr, UDP_SERVER_PORT);
        if (err != ERR_OK) {
                printf("Failed to bind UDP server PCB.\n");
                return -1;
        }

        udp_recv(udp_server_pcb, udp_receive_callback, NULL);
}

/* int main()
 * /brief starts the program, calls stdio_init_all(), init_wireless(), initPins()
 * 
 * the main loop is with the lowest computing priority. it's function is to non-stop loop the cyw43_arch_poll() function which polls the wireless. 
 * the main loop has no constant 'polling rate' because of its low priority and therefore it uses all left processor time from the high priority code in the PWM IRQ 
 */
int main() {
   	stdio_init_all();
    	
    	init_wireless();

	initPins();
	
    	while (true) {
                cyw43_arch_poll();
    	} 
    	
        cyw43_arch_deinit();
        return 0;
}


