#define LED_BASE 0xFF200000
#define HEX3_HEX0_BASE 0xFF200020
#define SW_BASE 0xFF200040

volatile int hex_code[16] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};

int readSwitches() {
    return ((int)SW_BASE);
}

void displayHexDigit(int digit) {
    ((int)HEX3_HEX0_BASE) = hex_code[digit];
}

int main() {
    int switchValue;

    while (1) {
        switchValue = readSwitches();
        // Mask switch value to ensure only lower 4 bits are used to select hex digit
        switchValue &= 0xF;
        displayHexDigit(switchValue);
    }
    return 0;
}