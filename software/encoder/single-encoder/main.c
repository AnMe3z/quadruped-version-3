#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/binary_info.h"

#define resolution 62
int degreesPerHole = 360/resolution;
int totalSpinDeg = 0;

int time1 = 0;
double time2 = 0;
double rpm = 0;

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
        gpio_put(25, 1);
        
        //set up the reading pin
        gpio_init(2);
        gpio_set_dir(2, GPIO_IN);
        //gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
        gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    	// Init UART communication
	stdio_init_all();

        while(true){
	  printf("Reading: %d \n", gpio_get(2));
          sleep_ms(1000);
        }

}
