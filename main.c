/* 
 * File:   main.c
 * Author: Stanislav Antonov
 *
 * Created on November 28, 2020, 6:19 PM
 * 
 * Based on https://www.gooligum.com.au/projects/PIC_Proj_1.pdf
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <xc.h>
#include <pic12f509.h>

#define _XTAL_FREQ 4000000

#define BUTTON GPIObits.GP3
#define LIGHT_RED GPIObits.GP1
#define LIGHT_YELLOW GPIObits.GP5
#define LIGHT_GREEN GPIObits.GP4

#define LIGHTS_OFF LIGHT_RED = 0; LIGHT_YELLOW = 0; LIGHT_GREEN = 0

#define LIGHT_RED_DURATION 10
#define LIGHT_YELLOW_DURATION 5
#define LIGHT_GREEN_DURATION 5

#define LIGHT_RED_START 0
#define LIGHT_YELLOW_1_START LIGHT_RED_START + (LIGHT_RED_DURATION - LIGHT_YELLOW_DURATION)
#define LIGHT_GREEN_START LIGHT_YELLOW_1_START + LIGHT_YELLOW_DURATION
#define LIGHT_YELLOW_2_START LIGHT_GREEN_START + LIGHT_GREEN_START
#define LIGHT_YELLOW_2_END LIGHT_YELLOW_2_START + LIGHT_YELLOW_DURATION

#define DEBOUNCE 10 * 1000 / 256 // Button debounce time constant assuming 
                                 // the timer Timer0 is running at 256 us/tick

#define BUTTON_POLL_DELAY 50     // In milliseconds
#define GENERAL_TIMEOUT 5        // In minutes

#pragma config OSC = IntRC // Oscillator selection bits (internal RC oscillator)
#pragma config WDT = OFF   // Watchdog timer enable bit (WDT disabled)
#pragma config CP = OFF    // Code protection bit (Code protection off)
#pragma config MCLRE = OFF // Int reset

void init(void);
void debounce(uint8_t pin);
void standby(void);

/*
 * 
 */
int main(int argc, char** argv) {
    init();
    debounce(BUTTON); // Wait for a stable button release
    
    uint16_t seconds_total = 0;
    
    while (1) {
        for (uint8_t seconds = 0; seconds < LIGHT_YELLOW_2_END; seconds++) {
            if (seconds == LIGHT_RED_START) {
                LIGHTS_OFF;
                LIGHT_RED = 1;
            }
            
            if (seconds == LIGHT_YELLOW_1_START) {
                LIGHT_YELLOW = 1;
            }
            
            if (seconds == LIGHT_GREEN_START) {
                LIGHT_RED = 0;
                LIGHT_YELLOW = 0;
                LIGHT_GREEN = 1;
            }
            
            if (seconds == LIGHT_YELLOW_2_START) {
                LIGHT_GREEN = 0;
                LIGHT_YELLOW = 1;
            }
            
            // Delay for 1 second (1000 ms) while polling the button 
            // in order to provide to the outer loop one-second granularity
            for (uint8_t p_cnt = 0; p_cnt < 1000 / BUTTON_POLL_DELAY; p_cnt++) {
                __delay_ms(BUTTON_POLL_DELAY);
                
                if (!BUTTON) {
                    standby();
                }
            }
            
            // Check for a general timeout
            if (++seconds_total == GENERAL_TIMEOUT * 60) {
                standby();
            }
        }
    }
    
    return (EXIT_SUCCESS);
}

/*
 *
 */
void init(void) {
    // I/O ports configuration
    // bit 5: GP5 is set as output
    // bit 4: GP4 is set as output
    // bit 3: GP3 is set as input
    // bit 2: GP2 is set as output
    // bit 1: GP1 is set as output
    // bit 0: GP0 is set as output
    TRIS = 0b001000;
    
    // bit 7 RBWU: Wake-up on Pin Change bit 
    // bit 6 RBPU: Weak Pull-ups 
    // bit 5 T0CS: Timer0 clock source select bit 
    // bit 4 T0SE: Timer0 Source Edge Select bit
    // bit 3 PSA:  Prescaler Assignment bit
    // bit 2-0 PS<2:0>: Prescaler Rate Select bits
    OPTION = 0b00000000;
    // 0------- wake-up on a pin change is enabled 
    // -0------ weak pull-ups are enabled in order to omit extra hardware 
    //          pull-up resistors. It also makes all the "floating" 
    //          (unused/untied) I/O pins of the CMOS be tied internally to Vdd 
    // --0----- transition on internal instruction cycle clock (Fosc/4)
    // ---0---- increment on low-to-high transition on the T0CKI pin
    // ----0--- prescaler assigned to Timer0
    // -----111 prescale = 256, increment every 256us
}

/*
 *
 */
void debounce(uint8_t pin) {
    TMR0 = 0;
    while (TMR0 < DEBOUNCE) {
        if (pin == 0) {
            TMR0 = 0;
        }
    }
}

/*
 * 
 */
void standby(void) {
    LIGHTS_OFF;
    debounce(BUTTON); // Wait for a stable button release
    SLEEP(); // Switch to sleep mode
}
