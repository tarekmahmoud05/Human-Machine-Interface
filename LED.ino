/******************************************************************
file: UART Functions Code
author: Nour Eldin, Tarek Mahmoud Younes, Kareem Magdy
brief: includes definitions of led control 
*******************************************************************/

/*************Include Section Start**********/

#include "LED.h" // Include the custom header file for the LED structure and function prototypes.
#include "Dio.h" // Include the custom header file for Digital I/O 
#include <avr/io.h>// Include standard AVR I/O definitions

/*************Include Section End**********/


/*************Defined Functions Section Start**********/

// Static function (local to this file) to set the LED brightness via PWM.
static void setLedBrightness(uint8_t brightness) 
{
    OCR2B = brightness; // Set the compare value for Timer/Counter 2, Channel B, this value directly determines the PWM duty cycle/brightness (0-255).
}

// Function to initialize the LED control system (Timer/Counter 2 in PWM mode).
void Led_init(Led* led) 
{   
    // Configure the dedicated PWM pin (OC2B, typically PD3 on ATmega328P) as an output.
    Dio_SetPinDirection(&DDRD, DDD3, DIO_DIR_OUTPUT);   // Use custom Dio function to set PD3 as output.

    // Reset Timer/Counter 2 Control Registers A and B.
    TCCR2A = 0; // Clear all bits in TCCR2A.
    TCCR2B = 0; // Clear all bits in TCCR2B.
    
    // Configure OC2B non-inverting mode.
    Dio_SetPinState(&TCCR2A, COM2B1, DIO_STATE_HIGH); // Set COM2B1 bit (Bit 5 in TCCR2A) to 1. This sets the non-inverting mode
    TCCR2A |= (1 << WGM20) | (1 << WGM21); // Set Waveform Generation Mode (WGM) bits for Fast PWM mode (Mode 3).

    
    
    Dio_SetPinState(&TCCR2B, CS22, DIO_STATE_HIGH);// This typically selects a prescaler like clk/64
    
    OCR2B = 0; // Set the initial brightness/duty cycle to 0 (LED off).
    
    led->setBrightness = setLedBrightness; // Assign the local PWM function to the function pointer within the Led structure.
}

/*************Defined Functions Section End**********/