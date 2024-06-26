// Motor Driver - ECE 3375B
// Anthony Barros, Bashshar Atif, Muhammad Shahzad, Virain Bawa
// March 24, 2024
#include <stdint.h>
#include <stdio.h>

// define gpio pins for motor controller (probably will have to change accordingly)
#define PHASE_PIN 	31  	// actual pin number for Phase2 (D31)
#define ENABLE_PIN 	30 		// actual pin number for Enable2 (PWM) (D30)

// define for GPIO and private timer check addresses and ADC addresses and direction switch 
#define PRIV_TIME 		0xFFFEC600		// private timer
#define ADC_BASE 		0xFF204000		// Analog to digital convertor
#define SW_BASE			0xFF200040		// Switch base 
#define JP1_BASE 		0xFF200060		// GPIO port
#define KEY_BASE             	0xFF200050		//button hardware
#define HEX3_HEX0_BASE        	0xFF200020		//hex outpdware location

// System configurations 
#define LOAD 200000		// (change acordingly)		

//ADC struct (8 sets of 32-bit words):
volatile unsigned int *adcController = (unsigned int *)ADC_BASE;	// Holds adc value 
volatile int pinState = 0;						// for deciding when the timer is high value or low value 

//button, switch, and hex display
volatile int* btnHardware = (int*) KEY_BASE;	
volatile int* hex_ptr = (int*) HEX3_HEX0_BASE;
volatile int *switch_ptr = (int *)SW_BASE;			// reads the switches 
volatile int switch_val;							// to store switch value 

// digits fore hex display 
volatile int hex_code[16] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};	
		
volatile int current_value = 0; 	// Initial value of the hex number
volatile int previous_btn_state = 0; 	// Keep track of the previous state of the buttons

//GPIO JP1 struct (4 sets of 32-bit words):
typedef struct
{
    uint32_t data;						//Data register
    uint32_t directCntrl;				//Direction Control register
    uint32_t interruptEnCntrl;			//Interrupt Enable Control register 
    uint32_t edgeCptrStatus;			//Edge-Capture Status register
} JP1_Struct;

// instantiate the jp1 controller
JP1_Struct *jp1Controller = (JP1_Struct *)JP1_BASE; // for the leds 

// Timer struct 
typedef struct {
	uint32_t load;			//load value (one period)
	uint32_t counter;		//current value 
	uint32_t control; 		//control setting (interrupt -maybe, auto restart - maybe, Enable)
	uint32_t status; 		//interrupt status 
} PrivateTimer;

// instantiate the timer 
volatile PrivateTimer *timer = (PrivateTimer *)PRIV_TIME;

// Initialize pin used for GPIO output PWM 
void GPIO_Init(){
	jp1Controller->directCntrl |= 0xFFFFFFFF;//set pins to output 
}

// set a specific pin to higher low 
void GPIO_Set(int pin, int value){
   	if (value) {
       	jp1Controller->data |= (1 << pin);  // Set pin high
		printf("high: %d\n", pin);
   	} else {
       	jp1Controller->data &= ~(1 << pin);  // Set pin low
		printf("low: %d\n", pin);
   	}
}

// Initialize the private timer 
void InitTimer(uint32_t loadValue){
	timer->load = loadValue;	// set the load value (one period duration)
	// maybe dont auto start and explicity restart it in sigControl every time the flag is set 
	timer->control = 0b11;		// enables and auto restarts 
	timer->status = 1; 			// Just to make sure and clear the flag
}

// reading the poteniometer value
int ReadADC(void) {
	adcController[0] = 0x1; 			// start adc conversation for channel 0 
	return adcController[0] & 0xFFF;	// return the 12 bit adc result 
}

// displaying the hex digit
void displayHexDigit(int digit) {
    *hex_ptr = hex_code[digit];
}

//Function to retreive SW0's value
int ReadSwitch(void)// read the lowest switch and interpret the instructions
{
    switch_val = *(switch_ptr) &= 0x01;	//convert retrieved value from the lowest switch
    return switch_val;					//return converted value
}

//Function to write to the GPIO pins:
void LightPins(int num){
    num = num*11/4096;		//10 pins handling 12 bits (2^12==4096)

    //write binary string containing num+1 1s in a row
    int pinWrites[11] = {
        0x0, 0x1, 0x3, 0x7,
        0xF, 0x1F, 0x3F, 0x7F,
        0xFF, 0x1FF, 0x3FF
    };
    
    //Map the value accordingly
    if(num<11) jp1Controller->data = pinWrites[num];
    else jp1Controller->data &= 0;					//clear out if number is too big to avoid damaging the LEDs
}

void ButtonControl()
{

	int btn_value = *btnHardware; // Read the value of buttons
	// Check if button for incrementing is pressed
	if ((btn_value & 0b0001) && !(previous_btn_state & 0b0001)) {
		current_value++; // Increment hex number
		if (current_value > 15) { // Ensure hex number stays within range [0, 15]
			current_value = 15;
		}
		displayHexDigit(current_value); // Display updated hex number
	}

	// Check if button for decrementing is pressed
	if ((btn_value & 0b0010) && !(previous_btn_state & 0b0010)) {
		current_value--; // Decrement hex number
		if (current_value < 0) { // Ensure hex number stays within range [0, 15]
			current_value = 0;
		}
		displayHexDigit(current_value); // Display updated hex number
	}

	previous_btn_state = btn_value; // Update previous button state
}

// polling the timer 
void SigControl(){
	// read potentiometer and light the leds 
	int adcValue = ReadADC();
	LightPins(adcValue);

	// for printing out values to trouble shoot 
	int flag = timer->status; //
	int loadval = timer->counter;
	printf("ADC Value: %d, Flag: %d, Load Value: %d\n", adcValue, flag, loadval);

	// Read switch for direction control and set phase pin
    int direction = ReadSwitch();
    GPIO_Set(PHASE_PIN, direction);

	if (timer->status & 0x01){	//check the status bit to see if the counter reached 0 
		
		// Toggle GPIO pin state  and 
		pinState = !pinState; 				
		// to change current HIGH and LOW 
        GPIO_Set(ENABLE_PIN, pinState);

		// scale adc for duty cycle for pwm 
		uint32_t loadOn = LOAD  *  adcValue / 4095;
		uint32_t loadOff = LOAD - loadOn;
		printf("Load On: %d, Load Off: %d\n", loadOn, loadOff); // toruble shooting 
		

		// Load the next value to the timer duration
		if (pinState) {
			timer->load = loadOn;
		}else {
			timer->load = loadOff;
		}
	}
}

// main method 
int main(void) {		
	// initialize the GPIO and timer
	GPIO_Init();
	InitTimer(LOAD); 

    volatile int delay_count;//to track delaying for loop
    //polling much faster than internal timers, so using main board to calculate delay
    int DELAY_LENGTH = 100;//to prevent bouncing
    volatile int val;//used to read from specific ADC channels

	// running loop 
	while(1){
		// for incrementing hex display 
		ButtonControl();
		// main function for producing pwm from reading the poteniometer 
		SigControl();
	}
	
}
