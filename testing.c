#include <stdint.h>

#define SW_BASE               0xFF200040
#define ADC_BASE              0xFF204000//Analog-to-Digital Convertor
#define JP1_BASE              0xFF200060//GPIO port
#define KEY_BASE              0xFF200050
#define HEX3_HEX0_BASE        0xFF200020

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

int main() {
    int current_value = 0; // Initial value of the hex number
    int previous_btn_state = 0; // Keep track of the previous state of the buttons
    int switchValue;

    while (1) {

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