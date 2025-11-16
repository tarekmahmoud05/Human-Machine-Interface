/******************************************************************
file: UART Functions Code
author: Nour Eldi, Tarek Mahmoud, Kareem Magdy
brief: Includes the final code for Human Machine Interface project
*******************************************************************/

/*************Include Section Start**********/

#if !defined(__AVR_ATmega328P__) // Conditional include for device-specific I/O registers if the target is not implicitly defined as ATmega328P.
#include <avr/iom328p.h>
#endif
#include "Adc.h" // Analog-to-Digital Converter functions.
#include "Dio.h" // Digital I/O (pin control) functions.
#include "Lcd.h" // LCD display functions.
#include "LED.h" // LED control functions .
#include "Uart.h" // UART (Serial communication) functions.
#include "Button.h" // Button handling functions.
#include "myString.h" // Custom string manipulation functions
#include <stdint.h> // Standard integer types
#include <avr/io.h> // Standard AVR I/O definitions
#include <util/delay.h> // Functions for generating delays.

/*************Include Section End**********/

/*************Definition Section Start**********/

#define F_CPU 16000000UL  // Define the microcontroller's clock frequency as 16 MHz (required for delay functions).
#define DIO_DIR_INPUT  0 // Define input direction for pins.
#define DIO_DIR_OUTPUT 1 // Define output direction for pins.
#define DIO_STATE_LOW  0 // Define low logic state (0V).
#define DIO_STATE_HIGH 1 // Define high logic state (VCC).
#define LED_PIN 5 // Define the pin number for alarm LED (PB5).

// Global structures/variables for peripheral control.
Led myLed; // Structure/handle for the controlled LED.
Button btnLowLimit; // Structure/handle for the Low Limit setting button.
Button btnHighLimit; // Structure/handle for the High Limit setting button.
Button btnSetLed; // Structure/handle for the LED Control setting button.

// Global state variables for the application.
uint8_t current_brightness = 0; // The current brightness level of the LED
uint16_t low_limit = 0; // User-defined lower boundary for LDR reading
uint16_t high_limit = 100; // User-defined upper boundary for LDR reading

// Flags to control which variable can be set via UART.
uint8_t led_is_unlocked = 0; //  UART input sets LED brightness. 0: Locked.
uint8_t ll_is_unlocked = 0; //  UART input sets Low Limit. 0: Locked.
uint8_t hl_is_unlocked = 0; //  UART input sets High Limit. 0: Locked.
 


int Ldr_Reading;  // Current LDR reading (mapped to 0-100).
int last1 = 100; // Last High Limit value
int last2 = 0; // Last Low Limit value
int lastR; // Last LDR reading value.
char LDR[6]; // Buffer for LDR reading string.
char HL[5]; // Buffer for High Limit string.
char LL[5]; // Buffer for Low Limit string.


/*************Definition Section End**********/

/*************main Funciton Start**********/

int main(void) {
    LCD_Init(); // Initialize the LCD display.
    Adc_Init(); // Initialize the ADC.
    Uart_Init(); // Initialize the UART for serial communication.
    Led_init(&myLed);  // Initialize the controlled LED.
    
    Dio_SetPinDirection(&DDRB, LED_PIN, DIO_DIR_OUTPUT); // Set the LED pin (PB5) direction as output.
    Button_init(&btnLowLimit, &DDRB, &PORTB, &PINB, PB2); // Initialize the Low Limit button on PB2.
    Button_init(&btnHighLimit, &DDRB, &PORTB, &PINB, PB3); // Initialize the High Limit button on PB3.
    Button_init(&btnSetLed, &DDRB, &PORTB, &PINB, PB4); // Initialize the LED Control button on PB4.

    char buffer[5]; // Buffer to temporarily hold received number digits
    char num_buffer[7]; // Buffer to hold the converted number string for transmission.
    uint8_t index = 0;  // Index for the 'buffer' array.
    uint16_t value_100; // Holds the numerical value received from UART (0-100).

    // Display fixed text on the LCD.
    LCD_String_xy(0, 0, "Val:"); // Display "Val:" at (Row 0, Col 0).
    LCD_String_xy(1, 0, "LL:"); // Display "LL:" at (Row 1, Col 0).
    LCD_String_xy(1, 9, "HL:"); // Display "HL:" at (Row 1, Col 9).
    
    // Main program loop (infinite).
    while (1) {
        // --- Button Handling ---

        
        if (btnLowLimit.getState(&btnLowLimit) == BTN_PRESSED)  // Check if the Low Limit button is pressed.
        {
            ll_is_unlocked = 1; // set the 'll_is_unlocked'.
            // Lock other settings
            hl_is_unlocked = 0; 
            led_is_unlocked = 0;
            Uart_SendString("Low Limit: Pressed\r\n", 20); // Send status message over UART.
        }
        while(btnLowLimit.getState(&btnLowLimit) == BTN_PRESSED); // Debounce/wait for the button to be released.

        if (btnHighLimit.getState(&btnHighLimit) == BTN_PRESSED)  // Check if the High Limit button is pressed.
        {
            hl_is_unlocked = 1;// set the 'hl_is_unlocked'.
            // Lock other settings.
            led_is_unlocked = 0;
            ll_is_unlocked = 0;
            Uart_SendString("High Limit: Pressed\r\n", 21); // Send status message over UART.
        }
        while(btnHighLimit.getState(&btnHighLimit) == BTN_PRESSED);// Debounce/wait for the button to be released.

        if (btnSetLed.getState(&btnSetLed) == BTN_PRESSED)  // Check if the LED Control button is pressed.
        { 
            led_is_unlocked = 1; // set the 'led_is_unlocked'.
            // Lock other settings.
            ll_is_unlocked = 0;
            hl_is_unlocked = 0;
            Uart_SendString("LED Control: Pressed\r\n", 22);// Send status message over UART.
        }
        while(btnSetLed.getState(&btnSetLed) == BTN_PRESSED);// Debounce/wait for the button to be released.

        // --- UART Input Handling ---

        if (Uart_IsDataAvailable()) // Check if a byte is available in the UART receive buffer.
        {
            unsigned char c = Uart_ReadData(); // Read the received character.
            
            if (c == '?') // Check for the status query command '?'
            {
                 char str_val[10]; // Temporary buffer for integer-to-string conversion.
                 Uart_SendString("STATUS:", 7); // Send status header.
                 
                // Transmit LDR reading.
                 Uart_SendString(" LDR=", 5); 
                 int_to_string(Ldr_Reading, str_val);
                 Uart_SendString(str_val, simple_strlen(str_val));
                
                // Transmit Low Limit.
                 Uart_SendString(" LL=", 4);
                 int_to_string(low_limit, str_val);
                 Uart_SendString(str_val, simple_strlen(str_val));

                // Transmit High Limit.
                 Uart_SendString(" HL=", 4);
                 int_to_string(high_limit, str_val);
                 Uart_SendString(str_val, simple_strlen(str_val));
                
                // Transmit current LED Brightness. 
                 Uart_SendString(" BRT=", 5);
                 int_to_string(current_brightness, str_val);
                 Uart_SendString(str_val, simple_strlen(str_val));

                 Uart_SendString("\r\n", 2); // Send carriage return and newline.
                 index = 0;  // Reset buffer index.
                 continue;  // Skip further processing for this loop iteration
            }
            
            if (c == '\r') // Ignore carriage return characters.
            {
                continue;
            }

            if (c >= '0' && c <= '9') // Process digit characters.
            {
                if (index < 4) // Store the digit in the buffer if there's space (max 4 digits).
                {
                    buffer[index++] = c;
                }
            } 
            else if (c == '\n') // Process the newline character (marks the end of input).
            {
                if (index > 0) // If at least one digit was received.
                {
                    buffer[index] = '\0'; // Null-terminate the received digit string.
                    value_100 = simple_atoi(buffer); // Convert the string to an integer.
                    int_to_string(value_100, num_buffer); // Convert value back to string for echoing.
                    // Echo the received value.
                    Uart_SendString("Received: ", 10);
                    Uart_SendString(num_buffer, simple_strlen(num_buffer));
                    Uart_SendString("\r\n", 2);
                    
                    
                    if (value_100 > 100) // Clamp the value to a maximum of 100.
                    {
                        value_100 = 100;
                    }

                    // Apply the received value based on which flag is unlocked.
                    if (led_is_unlocked) 
                    {
                        uint8_t mapped_led = (uint8_t)map(value_100, 0, 100, 0, 255); // Map the 0-100 value to the 0-255 PWM/brightness range.
                        myLed.setBrightness(mapped_led); // Set the LED brightness.
                        current_brightness = mapped_led; // Update the tracking variable.
                        Uart_SendString("-> Updated Brightness\r\n", 22);
                    } 
                    if (ll_is_unlocked) 
                    {
                        low_limit = value_100; // Update the low limit.
                        Uart_SendString("-> Updated Low Limit\r\n", 21);
                    }  
                    if (hl_is_unlocked) {
                        high_limit = value_100; // Update the high limit.
                        Uart_SendString("-> Updated High Limit\r\n", 22);
                    }
                }
                index = 0; // Reset buffer index for the next input.
            }
        }

        // --- LDR Reading and Limit Checking ---

        Ldr_Reading = Adc_ReadChannel(0); // Read the analog value from ADC Channel 0 (LDR).
        Ldr_Reading = (uint16_t)map(Ldr_Reading, 0, 1023, 0, 100); // Map the 0-1023 ADC raw reading to a simplified 0-100 scale.
        
        // Ensure limits are logically consistent (Low Limit <= High Limit).
        if(low_limit > high_limit) low_limit = high_limit; 
        if(high_limit < low_limit) high_limit = low_limit;

        // Convert the current limits and LDR reading to strings for display.
        int_to_string(low_limit, LL);
        int_to_string(high_limit, HL);
        int_to_string(Ldr_Reading, LDR); 

        // Logic to clear the LCD line if the number of digits of a displayed value changes.
        // This prevents old digits from remaining on the screen (e.g., 100 becoming 10).
        // The check uses `number_of_digits` which was previously noted
        if(number_of_digits(high_limit)<number_of_digits(last1)||number_of_digits(low_limit)<number_of_digits(last2)|| number_of_digits(Ldr_Reading) < number_of_digits(lastR)){
            LCD_Clear(); // Clear the entire LCD screen.
            // Re-display the fixed labels.
            LCD_String_xy(0, 0, "Val:");
            LCD_String_xy(1, 0, "LL:");
            LCD_String_xy(1, 9, "HL:");
        }
        // Update the 'last' variables for the next comparison.
        last1 = high_limit;
        last2 = low_limit;
        lastR = Ldr_Reading;
        
        // --- Limit Compliance Check ---

        if((Ldr_Reading>=low_limit)&&(Ldr_Reading<=high_limit)) // Check if the LDR reading is within the acceptable range (inclusive).
        {
            LCD_String_xy(0, 10, "Accept"); // Display "Accept" status.
            Dio_SetPinState(&PORTB, LED_PIN, DIO_STATE_LOW); // Turn off the external LED
        }
        else{
            LCD_String_xy(0, 10, "Danger"); // Display "Danger" status.
            Dio_SetPinState(&PORTB, LED_PIN, DIO_STATE_HIGH); // Turn on the external LED
        }

        // Update the dynamic values on the LCD.
        LCD_String_xy(0,4,LDR); // Display current LDR value.
        LCD_String_xy(1,12,HL); // Display High Limit value.
        LCD_String_xy(1,3,LL); // Display Low Limit value.
        _delay_ms(10); // Wait for 10 milliseconds before the next loop iteration.
    }

    return 0;
}
/*************main Funciton End**********/