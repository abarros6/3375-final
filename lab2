#define BOARD                 "DE10-Standard"

/* Cyclone V FPGA devices */
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define TIMER_BASE            0xFF202000

// Pointers that point to hardward
volatile int* btnHardware = (int*) KEY_BASE;
volatile int* tmrHardware = (int*) TIMER_BASE;
volatile int* swtchHardware = (int*) SW_BASE;
volatile int* hex_ptr = (int*) HEX3_HEX0_BASE;
volatile int* hex_ptr2 = (int*) HEX5_HEX4_BASE;

// Track time values
int hS1, hS2, s1, s2, m1, m2 = 0; 

// To know when it's running
int running = 0;

// Stores lap time values
int lapHS1, lapHS2, lapS1, lapS2, lapM1, lapM2 = 0;

int getTimerDisplayHex(int value)
{
    // Value table for converting display value to hex 
    int lookUpTable[10];

    // Populate values from 0 to 9
    lookUpTable[0] = 0x3F;
    lookUpTable[1] = 0x6;
    lookUpTable[2] = 0x5B;
    lookUpTable[3] = 0x4F;
    lookUpTable[4] = 0x66;
    lookUpTable[5] = 0x6D;
    lookUpTable[6] = 0x7D;
    lookUpTable[7] = 0x7;
    lookUpTable[8] = 0x7F;
    lookUpTable[9] = 0x67;

    return lookUpTable[value];
}

int main(void) {
	
    //these two lines set the counter start value low and high to 1000000 in decminal
    //this amounts to counting in 10s of milliseconds per tick as the clock speed is 100-MHz
	*(tmrHardware + 2) = 0x4240; 
    *(tmrHardware + 3) = 0x000F;

    //this line inits the control register of the timer 
	*(tmrHardware + 1) = 0b1111;
    
	
    // keep program alive 
    while(1) {
        // check the btns to see what option is pressed (debounce done automatically)

        /* START BUTTON */
        // bit placement (last one- bit 0: 0001)
        if ((*btnHardware == 0b0001) || (running == 1)) {
            // goes to hardware control process (tmr+1) and sets to start (start, stop, continue, ito) bits
            running = 1;
			*(tmrHardware + 1) = 0b0100;   
        } 

        /* STOP BUTTON */
        if (*btnHardware == 0b0010) {
            running = 0;
			*(tmrHardware + 1) = 0b1000;
        }

        /* LAP BUTTON */
        if (*btnHardware == 0b0100) {
            // save lap times 
            lapHS1 = hS1;
            lapHS2 = hS2;
            lapS1 = s1;
            lapS2 = s2;
            lapM1 = m1;
            lapM2 = m2;
        }

        /* RESET BUTTON */
        if (*btnHardware == 0b1000) {
            // clear variables set for time 
            hS1 = 0;
            hS2 = 0;
            s1 = 0;
            s2 = 0;
            m1 = 0;
            m2 = 0;

            // clear variables set for lapped time
            lapHS1 = 0;
            lapHS2 = 0;
            lapS1 = 0;
            lapS2 = 0;
            lapM1 = 0;
            lapM2 = 0;
        }

		/* TIMER CONTROLLED UPDATES FOR STORED VARIABLES*/

        // runs when the tmrHardware is in running state and triggered to update
		if (*(tmrHardware) == 0b0011) {
            // progress the lowest count milliscond
            hS2++;

            // refresh the tmr start status (that will update hs2 at the time of the tmr cycle)
			*(tmrHardware + 1) = 0b0100; 

        

            // if it makes it to 10.. 0 it and increment the other variable 
            if (hS2 == 10) {
                hS1++;
                hS2 = 0;
            }
            
            if (hS1 == 10) {
                s2++;
                hS1 = 0;
            }
            if (s2 == 10) {
                s1++;
                s2 = 0;
            }
            if (s1 == 6) {
                m2++;
                s1 = 0;
            }

            if (m2 == 10) {
                m1++;
                m2 = 0;
            }

            if (m1 == 6) {
                //if the time is over 59:59:99
                // clear variables set for time 
                hS1 = 0;
                hS2 = 0;
                s1 = 0;
                s2 = 0;
                m1 = 0;
                m2 = 0;
            }

            //write to the RUN status register
			*tmrHardware = 0b0010;
        } 
		
    

        /* DISPLAY VALUES ON BOARD*/
        // if switch is 0 its the running time. 0 is showing the lapped time
        if (*swtchHardware == 0) {
            *((char*)hex_ptr) = getTimerDisplayHex(hS2);
			*((char*)hex_ptr + 1) = getTimerDisplayHex(hS1);
			*((char*)hex_ptr + 2) = getTimerDisplayHex(s2);
			*((char*)hex_ptr + 3) = getTimerDisplayHex(s1);
			*((char*)hex_ptr2) = getTimerDisplayHex(m2);
			*((char*)hex_ptr2 + 1) = getTimerDisplayHex(m1);
        } else {
            *((char*)hex_ptr) = getTimerDisplayHex(lapHS2);
			*((char*)hex_ptr + 1) = getTimerDisplayHex(lapHS1);
			*((char*)hex_ptr + 2) = getTimerDisplayHex(lapS2);
			*((char*)hex_ptr + 3) = getTimerDisplayHex(lapS1);
			*((char*)hex_ptr2) = getTimerDisplayHex(lapM2);
			*((char*)hex_ptr2 + 1) = getTimerDisplayHex(lapM1);
        }
    }
}
