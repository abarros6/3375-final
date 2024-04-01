{\rtf1\ansi\ansicpg1252\cocoartf2757
\cocoatextscaling0\cocoaplatform0{\fonttbl\f0\fnil\fcharset0 HelveticaNeue;}
{\colortbl;\red255\green255\blue255;}
{\*\expandedcolortbl;;}
\margl1440\margr1440\vieww11520\viewh8400\viewkind0
\deftab560
\pard\pardeftab560\slleading20\partightenfactor0

\f0\fs26 \cf0 // Motor Driver \
#include <stdint.h>\
\
// define gpio pins for motor controller (probably will have to change accordingly)\
#define PHASE_PIN 35  		// define actual pin number for Phase2 (D35)\
#define ENABLE_PIN 34  		// define actual pin number for Enable2 (PWM) (D34)\
\
// define for GPIO and private timer check addresses and ADC addresses and direction switch \
#define PRIV_TIME 0xFFFEC600		// private timer\
#define ADC_BASE 0xFF204000	// Analog to digital convertor \uc0\u8232 #define SW_BASE	0xFF200040		// Switch base \
#define JP1_BASE 0xFF200060		// GPIO port\
\
// System configurations \
#define CLOCK_FREQ 100000000	// 100 MHz (change acordingly)\
#define PWM_FREQ  1000			// 1 kHz PWM (change accoridngly)\
\
// Calculate timer load values for clock frequency per desired pwm frequency  \
#define LOAD CLOCK_FREQ / PWM_FREQ\
\
volatile unsigned int *adcController = (unsigned int *)ADC_BASE		// Holds adc value \
volatile int pinState = 0										// for deciding when the timer is high value or low value \
\
volatile int *switch_ptr = (int *)SW_BASE;							// reads the switches \
volatile int switch_val;											// to store switch value \
		\
//GPIO JP1 struct (4 sets of 32-bit words):\
typedef struct\
\{\
    uint32_t data;//Data register\
    uint32_t directCntrl;//Direction Control register\
    uint32_t interruptEnCntrl;//Interrupt Enable Control register\
    uint32_t edgeCptrStatus;//Edge-Capture Status register\
\} JP1_Struct;\
\
JP1_Struct *jp1Controller = (JP1_Struct *)JP1_BASE;\
\
// Timer struct \
typedef struct \{\
	uint32_t load;		//load value (one period)\
	uint32_t counter;		//current value \
	uint32_t control; 		//control setting (interrupt -maybe, auto restart - maybe, Enable)\
	uint32_t status; 		// interrupt status \
\} PrivateTimer;\
\
// instantiate the timer \
volatile PrivateTimer *timer = (PrivateTimer *)PRIV_TIME;\
\
// Initialize pin used for GPIO output PWM \
void GPIO_Init(int pin)\{\
	// write the code here depending on if The motor controller is with jp1 or if its a seperate GPIO  if its seprate make a struct and adjust the code \
	// EX: gpioController->directCntrl |= 1 << ENABLE_PIN  // Set pins as output\
\}\
\
// set the pin to higher low \
void GPIO_Set(int pin, int value)\{\
	// write the code here depending on if The motor controller is with jp1 or if its a seperate GPIO\
//    	if (value) \{\
//        	gpioController->data |= (1 << pin);  // Set pin high\
//    	\} else \{\
//        	gpioController->data &= ~(1 << pin);  // Set pin low\
//    	\}\
\}\
\
// Initialize the private timer \
void InitTimer(uint32_t loadValue)\{\
	timer->load = loadValue;	// set the load value (one period duration)\
	timer->control = 0b11;		// enables auto restart and Enable \
	timer->status = 1; 			// Just to make sure and clear the flag\
\}\
\
int ReadADC(void) \{\
	adcController[0] = 0x1; 			// start adc conversation for channel 0 \
	return adcController[0] & 0xFFF;	// return the 12 bit adc result \
\}\
\
int ReadSwitch(void)\{\
	switch_val = *(switch_ptr) &= 0x01;		//convert retrieved value from the lowest switch\
	return switch_val;						//return converted value\
\}\
\
//Function to write to the GPIO pins:\
void LightPins(int num)\{\
    num = num*11/4096;				//10 pins handling 12 bits (2^12==4096)\
\
    //write binary string containing num+1 1s in a row\
    int pinWrites[11] = \{\
        0x0, 0x1, 0x3, 0x7,\
        0xF, 0x1F, 0x3F, 0x7F,\
        0xFF, 0x1FF, 0x3FF\
    \};\
    \
    //Map the value accordingly\
    if(num<11) jp1Controller->data = pinWrites[num];\
    else jp1Controller->data &= 0;					//clear out if number is too big to avoid damaging the LEDs\
\}\
\
// polling the timer \
void SigControl()\{\
	if (timer->status & 0x01)\{	//check the status bit to see if the counter reached 0 \
		timer->status = 1;   	// clear the interrupt status \
		\
		// Toggle GPIO pin state  and \
		pinState = !pinState; 				// to change current HIGH and LOW \
        	GPIO_Set(ENABLE_PIN, pinState);\
		\
		// read potentiometer and light lens\
		int adcValue = ReadADC();\
		LightPins(adcValue);\
\
		// scale adc for duty cycle \
		uint32_t loadOn = LOAD  *  adcValue / 4095;	 // double check this when implementing \
		uint32_t loadOff = LOAD - loadOn;\
		\
\
		// Load the next value to the timer duration\
		if (pinState) \{\
			InitTimer(loadOn);\
		\}else \{\
			InitTimer(loadOff);\
		\}\
\
		// Read switch for direction control and set phase pin\
		// see if our bird supports this method otherwise use two phase pins to control it or use a H-bridge \
       		int direction = ReadSwitch();\
        	GPIO_Set(PHASE_PIN, direction);\
	\}\
\}\
\
// main method \
int main(void) \{		\
	\
	// initialize the GPIO and timer\
	GPIO_Init();\
	InitTimer(0); 		// start at the off state \
\
	// running loop \
	while(1)\{\
		SigControl()\
	\}\
	\
\}\
}