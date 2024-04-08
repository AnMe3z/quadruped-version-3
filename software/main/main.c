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

#define MAX_ANGLE 42 // max angle = 99.47368423
#define MIN_ANGLE 0

//PWM 
uint freqHz = 10000;
uint wrapP = 12500;
  
// Arrays for motor variables
// Motor index to pin
// TODO: TO BO ADDED TO AXIS STRUCT
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
int QEM [16] = {0,-1,1,2,1,0,2,-1,-1,2,0,1,2,1,-1,0};
//uint8_t QEM [16] = {0,1,-1,2,-1,0,2,1,1,2,0,-1,2,-1,1,0};  // Quadrature Encoder Matrix
//uint8_t QEM [16] = {0,-1,1,2,1,0,2,-1,-1,2,0,1,2,1,-1,0};  // Quadrature Encoder Matrix inverted
//FIXME
float step = 2.368421053;//360/(holes*4);
	
//SERVO CONTROL
#define KP 5

struct axis {
        int pinA;
        int pinB;
        
        // Counts the encoder changes in total. Can be used to calculate absolute position since start
        int count;
        int oldState;
        int newState;

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

//FUNCTIONS
void initPins();
//MOTOR
void driveMotor(int motorIndex, int driveValue, bool driveEnable);
//BASIC
long map(long x, long in_min, long in_max, long out_min, long outmax);

void resetPosition();

// FIXME: MANUAL ANGLE TESTING
void keyboardControl();

#define UDP_SERVER_PORT 12345

#define UDP_PORT 4444
#define BEACON_MSG_LEN_MAX 127
#define BEACON_TARGET "255.255.255.255"
#define BEACON_INTERVAL_MS 1000

#define DEVICE_ID 0 // or 1

int led;

int input_data[18];

void process_data(){
        int dir;
        struct axis *j;
        if (DEVICE_ID == 0) {
                for (int i = 0; i < 4; i++) {
                        dir = (input_data[(i*3) + 1] == 1) ? -1 : 1; 
                
                        j = axes+i;
                        
            	        j->startPoint = j->count;
                        j->setPoint = j->startPoint + (dir * (10*input_data[(i*3) + 2] + input_data[(i*3) + 3])); 
                        if(MAX_ANGLE > j->setPoint && j->setPoint > MIN_ANGLE){
	                          		j->moving = true;
                        }
                        printf("dir * (10*input_data[(i*3) + 2] + input_data[(i*3) + 3]) %d ", dir * (10*input_data[(i*3) + 2] + input_data[(i*3) + 3]));
                }
        }
        else {
        }
        
}

void udp_receive_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
        if (p != NULL) {
                // Received packet buffer 'p' is not NULL
                // You can process the packet here
                char *packet_data = (char *)p->payload;
                printf("Received UDP message: %s\n", packet_data);
                
                if (strlen(packet_data) == 52) {
                        int i;
                        
                        while (packet_data[i] != '\0' && packet_data[i+1] != '\0') {
                                input_data[i/2] = ((packet_data[i] - '0') * 10 + (packet_data[i+1] - '0')) - 30;
                                printf("%d\n", input_data[i/2]);
                                i += 2;
                        }
                        
                        process_data();
                }
                else {
                        printf("ERROR: Message is not 52 characters long\n");
                }

                //return 0;
                
                led = led ^ 1;
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led);

                // Don't forget to free the packet buffer
                pbuf_free(p);
        }
}

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
                                driveMotor(i, p->P, true); 
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

//FIXME: FOR TESTING PHOTO COUPLES
void encoderCallback(uint gpio, uint32_t events) {   
    if (gpio == motorIndexToPins[0][2] || gpio == motorIndexToPins[0][3]){
        if(gpio==motorIndexToPins[0][2]){
          if (events == GPIO_IRQ_EDGE_RISE){
            gpio_put(20, 1);
          }
          else{
            gpio_put(20, 0);
          }
        }
        if(gpio==motorIndexToPins[0][3]){
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
    	
    	if (cyw43_arch_init()) {
                printf("failed to initialise\n");
                return 1;
        }

        cyw43_arch_enable_sta_mode();

        printf("Connecting to Wi-Fi...\n");
        if (cyw43_arch_wifi_connect_timeout_ms("andrey_shefa", "andreyshefa1", CYW43_AUTH_WPA2_AES_PSK, 30000)) {
                printf("failed to connect.\n");
                return 1;
        } else {
                printf("Connected.\n");
                led = led ^ 1;
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led);
        }

        struct udp_pcb *udp_server_pcb = udp_new();
        if (udp_server_pcb == NULL) {
                printf("Failed to create new UDP PCB.\n");
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
	//driveMotor(3, 100, true);
 
        struct axis *j;
        
    	sleep_ms(2000);
    	while (true) {
    	
        	// FIXME: PHOTO COUPLES TEST
        	//printf("PIN 10: %d \n", gpio_get(10));
        	//printf("PIN 11: %d \n", gpio_get(11));
        	//sleep_ms(100);
        	
        	// FIXME: ENCODER COUNT TEST
      	        //printf("Axis [ 0 ] count: %d \n", axes[0].count);
      	        //printf("Axis [ 1 ] count: %d \n", axes[1].count);
      	        //printf("Axis [ 2 ] count: %d \n", axes[2].count);
      	        //printf("Axis [ 3 ] count: %d \n", axes[3].count);
        	//sleep_ms(100);
        	
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
    	
        cyw43_arch_deinit();
        return 0;
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
        struct axis *p;
        for (int i = 0; i < 4; i++) {
                p = axes+i;
                // Reset the encoder count
                p->count = 0;
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
        
        printf("Axis [ 1 ] count: %d \n", axes[1].count);
        printf("Error: %d \n", axes[1].error);
        printf("P: %d \n", axes[1].P);
        printf("Axis [ 1 ] count: %d \n", axes[1].count);

        printf("Enter direction [0 || 1] (1 = -1): \n");
        input = getchar() - 48;
        input = (input == 0 || input == 1) ? ((input == 0) ? 1 : -1) : 0; 

        printf("Enter target count Xx: \n");
        input = input*(getchar() - 48)*10;
        printf("Enter target count xX: \n");
        input += getchar() - 48;
        printf("Target count: %d \n", input);

        axes[1].startPoint = axes[1].count;
        axes[1].setPoint = axes[1].startPoint + input; 
        if(MAX_ANGLE > axes[1].setPoint && axes[1].setPoint > MIN_ANGLE){
	          		axes[1].moving = true;
        }
        sleep_ms(1000);
        
        //printf("Enter angle motor index [0 || 1]: \n");
        //input = getchar() - 48;

	//if (input == 0) {
        //	printf("FEMUR \n");
	//}
	//sleep_ms(1111);
}
