#include <stdint.h>

#define SW_BASE               0xFF200040//switch hardware
#define ADC_BASE              0xFF204000//Analog-to-Digital Convertor
#define JP1_BASE              0xFF200060//GPIO port
#define KEY_BASE              0xFF200050//button hardware
#define HEX3_HEX0_BASE        0xFF200020//hex output hardware location

volatile int* btnHardware = (int*) KEY_BASE;
volatile int* hex_ptr = (int*) HEX3_HEX0_BASE;

// volatile so they survive compilation
volatile int *switch_ptr = (int *)SW_BASE;//read the switches, bitmask by &=0x01 to only read SW0
volatile int switch_val;//to store the value retrieved from switch_ptr
volatile int hex_code[16] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};

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

void displayHexDigit(int digit) {
    *hex_ptr = hex_code[digit];
}

//Function to retreive SW0's value
int ReadSwitch(void)// read the lowest switch and interpret the instructions
{
    switch_val = *(switch_ptr) &= 0x01;//convert retrieved value from the lowest switch
    return switch_val;//return converted value
}


// BELOW FUNCTION IS CURRENTLY UNUSED, BROUGHT OVER FROM LAB 3
// WE WILL LIKELY BE CONVERTING THIS FUNCTION TO TRANSLATE THE POTENTIOMETER 
//OUTPUT TO THE PWM INPUT
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

int main() {

    // LAB 3 code START //

    jp1Controller->directCntrl |= 0x3FF;//set pins 0 through 9 as output
    volatile int delay_count;//to track delaying for loop
    //polling much faster than internal timers, so using main board to calculate delay
    int DELAY_LENGTH = 100;//to prevent bouncing
    volatile int val;//used to read from specific ADC channels

    // LAB 3 code END  //

    int current_value = 0; // Initial value of the hex number
    int previous_btn_state = 0; // Keep track of the previous state of the buttons
    int switchValue;

    while (1) {

        /* CURRENTLY UNUSED LAB 3 CODE FOR INTERACTING WITH ADC

            adcController[1] = 0x1;
            val = adcController[1] & 0xFFF;//only preserve bits 0 through 11
            LightPins(val);
        
        */

        if(ReadSwitch()) {
            //if swtich 0 is on then switch direction to reverse

        } else {
            //if swtich 0 is off, then direction is normal
        }
        
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

    return 0;
}