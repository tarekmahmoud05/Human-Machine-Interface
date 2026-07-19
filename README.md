# Light Monitoring and Control (LMC) Module
 
An embedded Human-Machine Interface (HMI) system built on the Arduino Uno that monitors ambient light using an LDR sensor, controls an alarm LED and a PWM-driven brightness LED, and integrates with a Python/Computer Vision application for gesture-based parameter control.
 
## Overview
 
The LMC reads ambient light through a Light-Dependent Resistor (LDR) connected to the Arduino's ADC, compares the reading against user-defined high and low limits, and displays real-time status on an LCD. When the light level falls outside the configured range, an alarm LED is triggered. Users can update the high limit, low limit, and LED brightness using hand gestures via a companion Python/OpenCV application, with values transmitted over UART.
 
## Features
 
- **Continuous ADC-based light sensing** via an LDR on the Arduino Uno
- **Real-time LCD feedback** (4-bit mode) showing LDR value, limits, and system status
- **Accept / Danger status logic** based on configurable High Limit (HL) and Low Limit (LL)
- **Alarm LED** that activates automatically when readings fall out of range
- **PWM-controlled brightness LED** adjustable via gesture input
- **Three-button debounced input** for selecting High Limit, Low Limit, or LED Brightness for editing
- **UART communication protocol** for exchanging control data with a PC-based hand gesture application
- **Modular firmware architecture** split into independent driver modules (ADC, DIO, LED, LCD, UART, Button, String)
## System Architecture
 
```mermaid
graph TD
    %% Main Architecture Elements
    HMI["Human Machine Interface (HMI)"]
    Human["Human.ino"]
    Uart["Uart.ino / .h"]
    Dio["Dio.ino / .h"]
    Adc["Adc.ino / .h"]
    Lcd["Lcd.ino / .h"]
    Led["Led.ino / .h"]
    Button["Button.ino / .h"]
    myString["myString.ino / .h"]

    %% Direct Structural Hierarchy
    HMI --> Human
    Human --> Uart
    Human --> Dio
    Human --> Adc
    Human --> Lcd
    Human --> Led
    Human --> Button
    Human --> myString

    %% Side Alignment Documentation Boxes
    Note1["Note: Top-level integration file"]
    Note2["Note: UART communication logic"]
    Note3["Note: Digital input/output handling"]
    Note4["Note: ADC driver for LDR readings"]
    Note5["Note: LCD display driver"]
    Note6["Note: LED (alarm + PWM brightness) control"]
    Note7["Note: Debounced button input handling"]
    Note8["Note: Custom string manipulation utilities"]

    %% Link the notes to the boxes cleanly
    Human --- Note1
    Uart --- Note2
    Dio --- Note3
    Adc --- Note4
    Lcd --- Note5
    Led --- Note6
    Button --- Note7
    myString --- Note8

 
## Hardware Setup
 
| Component               | Pin      | Direction | Notes                              |
|--------------------------|----------|-----------|-------------------------------------|
| LDR Sensor               | PC0      | Input     | Read via ADC                       |
| High Limit Button        | PB2      | Input     | Pull-down resistor                 |
| Low Limit Button         | PB3      | Input     | Pull-down resistor                 |
| LED Control Button       | PB4      | Input     | Pull-down resistor                 |
| Alarm LED                | PB5      | Output    | ON when reading is out of range    |
| Brightness LED (PWM)     | PD3      | Output    | Timer 2 PWM (OC2B)                 |
| LCD Data Pins (D4–D7)    | PD4–PD7  | Output    | 4-bit mode                         |
| LCD Enable                | PB1      | Output    |                                     |
| LCD RS                    | PB0      | Output    |                                     |
 
## Configuration
 
| Parameter          | Value           | Description                                              |
|---------------------|-----------------|------------------------------------------------------------|
| `F_CPU`             | `16000000UL`    | MCU clock frequency, used for baud rate calculation        |
| Baud Rate           | `9600`          | Fixed UART baud rate                                       |
| LDR Value Range     | `0–100`         | Valid range for LDR readings                               |
| UART Input Buffer   | `5`             | Max buffer size for incoming UART commands                 |
| Main Loop Delay     | `10 ms`         | Delay applied at the end of each main loop iteration        |
 
## How It Works
 
1. The LDR continuously feeds light-level data to the ADC, and the converted value is shown on the LCD.
2. The system compares the reading against the configured High Limit and Low Limit:
   - Within range → **"Accept"** is displayed, alarm LED is OFF.
   - Out of range → **"Danger"** is displayed, alarm LED is ON.
3. Pressing any of the three buttons (High Limit, Low Limit, or LED Control) puts the system into gesture-control mode:
   - The Arduino signals the host PC over UART.
   - The Python/CV application captures a hand gesture and sends the new value back over UART.
   - The Arduino validates and applies the value, then updates the LCD.
   - The same button must be pressed again to confirm the new value.
4. If the LED Brightness parameter is updated, the change is immediately reflected on the PWM-controlled brightness LED.
## Assumptions & Constraints
 
- Clock frequency fixed at `16 MHz`
- Push buttons are pull-down and must be released after each press (debounced)
- LDR value range: `0–100`
- High limit cannot be set lower than the low limit
- UART baud rate fixed at `9600`
- LCD operates in 4-bit mode
- Hand gesture control requires a PC with a webcam and cannot be used through the Arduino IDE alone
## Repository Structure
 
@startuml
left to right direction
skinparam LookAndFeel modern

map "Human Machine Interface (HMI)" as HMI {
}

map "Human.ino" as Human {
}

map "Uart.ino / .h" as Uart {
}
map "Dio.ino / .h" as Dio {
}
map "Adc.ino / .h" as Adc {
}
map "Lcd.ino / .h" as Lcd {
}
map "Led.ino / .h" as Led {
}
map "Button.ino / .h" as Button {
}
map "myString.ino / .h" as myString {
}

HMI --> Human
Human --> Uart
Human --> Dio
Human --> Adc
Human --> Lcd
Human --> Led
Human --> Button
Human --> myString

note right of Human : Top-level integration file
note right of Uart : UART communication logic
note right of Dio : Digital input/output handling
note right of Adc : ADC driver for LDR readings
note right of Lcd : LCD display driver
note right of Led : LED (alarm + PWM brightness) control
note right of Button : Debounced button input handling
note right of myString : Custom string manipulation utilities
@enduml
 
## Getting Started
 
1. Wire the hardware according to the pinout table above.
2. Open `Human.ino` in the Arduino IDE and upload it to the Arduino Uno.
3. Connect the LCD and verify it initializes correctly (display ON, cursor OFF, screen cleared).
4. Run the companion Python/OpenCV hand gesture application on a PC connected via UART.
5. Press any of the three control buttons to enter gesture-control mode and adjust High Limit, Low Limit, or LED Brightness.
## Notes
 
- All port pins default to input (`0`) unless explicitly configured.
- Limits and LED brightness can also be updated by sending data via UART from the Arduino IDE Serial Monitor.
- Holding a button down will delay the start of gesture-control mode until it is released.
- LCD custom characters (if used) are defined via the LCD Custom Character Generator.
## Authors

 - Tarek Mahmoud Younes
- Nour Eldin
- Kareem Magdy

 
## License
 
This project is provided for academic purposes as part of an Embedded Systems module coursework.
 
