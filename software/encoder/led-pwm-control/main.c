#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/binary_info.h"

// PWM calculations
uint freqHz = 5;
uint wrapP = 25000000;//(1/freqHz)/(8*1000000000);

uint slice_num;
        
int main() {

	// set up the built-in led
	gpio_init(25);
    	gpio_set_dir(25, GPIO_OUT);
	gpio_init(1);
    	gpio_set_dir(1, GPIO_OUT);

        // set up pwm on GPIO 0
        gpio_set_function(0, GPIO_FUNC_PWM);
        // get PWM channel for that pin
        slice_num = pwm_gpio_to_slice_num(0);
        // enable PWM on that channel
        pwm_set_enabled(slice_num, true);
        // set wrap point
        pwm_set_wrap(slice_num, wrapP);
	
    	// Init UART communication
	stdio_init_all();

        pwm_set_chan_level(slice_num, PWM_CHAN_A, 5000 );
        
        while(true){
          gpio_put(1, 1);
          sleep_ms(2000);
          gpio_put(1, 0);
          sleep_ms(2000);
          
        }

}
