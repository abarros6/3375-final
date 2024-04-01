#include <stdint.h>
#define SW_BASE               0xFF200040
#define ADC_BASE              0xFF204000//Analog-to-Digital Convertor
#define JP1_BASE              0xFF200060//GPIO port

// volatile so they survive compilation
volatile int *switch_ptr = (int *)SW_BASE;//read the switches, bitmask by &=0x01 to only read SW0
volatile int switch_val;//to store the value retrieved from switch_ptr

//ADC struct (8 sets of 32-bit words):
volatile unsigned int *adcController = (unsigned int *)ADC_BASE;

//GPIO JP1 struct (4 sets of 32-bit words):
typedef struct
{
    uint32_t data;//Data register
    uint32_t directCntrl;//Direction Control register
    uint32_t interruptEnCntrl;//Interrupt Enable Control register
    uint32_t edgeCptrStatus;//Edge-Capture Status register
} JP1_Struct;
JP1_Struct *jp1Controller = (JP1_Struct *)JP1_BASE;

//Function to retreive SW0's value
int ReadSwitch(void)// read the lowest switch and interpret the instructions
{
    switch_val = *(switch_ptr) &= 0x01;//convert retrieved value from the lowest switch
    return switch_val;//return converted value
}

//Function to write to the GPIO pins:
void LightPins(int num){
    num = num*11/4096;//10 pins handling 12 bits (2^12==4096)
    //write binary string containing num+1 1s in a row
    int pinWrites[11] = {
        0x0, 0x1, 0x3, 0x7,
        0xF, 0x1F, 0x3F, 0x7F,
        0xFF, 0x1FF, 0x3FF
    };
    
    //Map the value accordingly
    if(num<11) jp1Controller->data = pinWrites[num];
    else jp1Controller->data &= 0;//clear out if number is too big to avoid damaging the LEDs
}

int main(void) {
    jp1Controller->directCntrl |= 0x3FF;//set pins 0 through 9 as output
    volatile int delay_count;//to track delaying for loop
    //polling much faster than internal timers, so using main board to calculate delay
    int DELAY_LENGTH = 100;//to prevent bouncing
    volatile int val;//used to read from specific ADC channels

    //manip ADC and GPIO as needed
    while (1) 
	{
        if(ReadSwitch()){//if switch is on, read from 1st potentiometer
            adcController[1] = 0x1;
            val = adcController[1] & 0xFFF;//only preserve bits 0 through 11
            LightPins(val);
        }
        else{//otherwise, read from the 0th potentiometer
            adcController[0] = 0x1;
            val = adcController[0] & 0xFFF;//only preserve bits 0 through 11
            LightPins(val);
        }
        for (delay_count = DELAY_LENGTH; delay_count != 0; --delay_count)
        ; // delay loop
    }
}