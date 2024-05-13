

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
