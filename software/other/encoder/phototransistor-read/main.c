#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/binary_info.h"

int main() {

	// set up the built-in led
	gpio_init(25);
    	gpio_set_dir(25, GPIO_OUT);
	gpio_init(1);
    	gpio_set_dir(1, GPIO_OUT);
        gpio_put(25, 1);
        
        //set up the reading pin
        gpio_init(2);
        gpio_set_dir(2, GPIO_IN);

    	// Init UART communication
	stdio_init_all();

        while(true){
	  printf("Reading: %d \n", gpio_get(2));
          sleep_ms(1000);
        }

}
