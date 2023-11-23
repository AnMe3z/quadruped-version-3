#include "pico/stdlib.h"
#include "hardware/pwm.h"

int main() {

	// set up the built-in led
    	gpio_init(25);
    	gpio_set_dir(25, GPIO_OUT);

	// set up pwm on GPIO 0
	gpio_set_function(0, GPIO_FUNC_PWM);
	// get PWM channel for that pin
	uint slice_num = pwm_gpio_to_slice_num(0);
	// enable PWM on that channel
	pwm_set_enabled(slice_num, true);
	// set wrap point
	pwm_set_wrap(slice_num, 12500);
	// set the duty cycle
	pwm_set_chan_level(slice_num, PWM_CHAN_A, 6250);

    	while (true) {
//        	gpio_put(25, 1);
//        	sleep_ms(2500);
//        	gpio_put(25, 0);
//        	sleep_ms(2500);

		tight_loop_contents();
    	}
}
