#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/binary_info.h"

int main() {

	// set up the built-in led
	gpio_init(25);
    	gpio_set_dir(25, GPIO_OUT);

	// PWM calculations
	uint freqHz = 10000;
	uint wrapP = 12500;

	// set up pwm on GPIO 0
	gpio_set_function(0, GPIO_FUNC_PWM);
	// get PWM channel for that pin
	uint slice_num = pwm_gpio_to_slice_num(0);
	// enable PWM on that channel
	pwm_set_enabled(slice_num, true);
	// set wrap point
	pwm_set_wrap(slice_num, wrapP);
    	// Init UART communication
	stdio_init_all();

	double input = 0;	

    	while (true) {
//        	gpio_put(25, 1);
        	sleep_ms(2500);

		printf("Enter duty cycle Xx: \n");
		input = getchar() - 48;
		input /= 10;
		// set the duty cycle
		pwm_set_chan_level(slice_num, PWM_CHAN_A, wrapP * input);
		printf("Duty cycle: %d \n", wrapP * input);
	}
}
