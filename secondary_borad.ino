

#define F_CPU 16000000UL

#if !defined(__AVR_ATmega328P__)
#include <avr/iom328p.h>
#endif

#include <avr/io.h>
#include <util/delay.h>

// Define the output pins that connect to the DUT's button inputs
// These are on the TESTER Arduino
#define PIN_LL_PORT PORTD
#define PIN_LL_NUM  PD2  // Tester Pin D2

#define PIN_HL_PORT PORTD
#define PIN_HL_NUM  PD3  // Tester Pin D3

#define PIN_LED_PORT PORTD
#define PIN_LED_NUM  PD4  // Tester Pin D4

// How long to "hold" the button down (in milliseconds)
#define PRESS_DURATION 100
#define BAUD_RATE 9600

// --- Low-Level UART Functions ---

void Uart_Init(void) {
    // Calculate UBRR value
    uint16_t ubrr = (F_CPU / (16UL * BAUD_RATE)) - 1;
    
    // Set baud rate
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    
    // Enable receiver and transmitter
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    
    // Set frame format: 8data, 1stop bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void Uart_SendByte(unsigned char data) {
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)));
    // Put data into buffer, sends the data
    UDR0 = data;
}

void Uart_SendString(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        Uart_SendByte(str[i]);
    }
}

unsigned char Uart_ReceiveByte(void) {
    // Wait for data to be received
    while (!(UCSR0A & (1 << RXC0)));
    // Get and return received data from buffer
    return UDR0;
}

uint8_t Uart_IsDataAvailable(void) {
    return (UCSR0A & (1 << RXC0));
}

// --- GPIO & Test Logic ---

void GPIO_Init(void) {
    // Set D2, D3, D4 as outputs
    DDRD |= (1 << DDD2) | (1 << DDD3) | (1 << DDD4);
    
    // *** IMPORTANT LOGIC FIX ***
    // Default to LOW. Your DUT's button.c detects a HIGH signal as a press.
    PORTD &= ~((1 << PORTD2) | (1 << PORTD3) | (1 << PORTD4));
}

void pressButton(volatile uint8_t* port, uint8_t pinNum) {
    // *** IMPORTANT LOGIC FIX ***
    // Pulse HIGH (press)
    *port |= (1 << pinNum);
    _delay_ms(PRESS_DURATION);
    // Pulse LOW (release)
    *port &= ~(1 << pinNum);
}

int main(void) {
    GPIO_Init();
    Uart_Init();
    
    Uart_SendString("Tester Arduino Ready (Low-Level C)\r\n");
    Uart_SendString("Send L, H, or B.\r\n");

    while (1) {
        if (Uart_IsDataAvailable()) {
            char cmd = Uart_ReceiveByte();

            if (cmd == 'L') {
                pressButton(&PIN_LL_PORT, PIN_LL_NUM);
                Uart_SendString("Pressed: Low Limit\r\n");
            } 
            else if (cmd == 'H') {
                pressButton(&PIN_HL_PORT, PIN_HL_NUM);
                Uart_SendString("Pressed: High Limit\r\n");
            } 
            else if (cmd == 'B') {
                pressButton(&PIN_LED_PORT, PIN_LED_NUM);
                Uart_SendString("Pressed: Set LED\r\n");
            }
        }
    }
    return 0; // Never reached
}