# Human-Machine-Interface
# Light Monitoring and Control Module (LMC) 💡🤖

[![Status](https://img.shields.io/badge/Status-Draft-yellow.svg)](https://github.com/)
[![Version](https://img.shields.io/badge/Version-1.0.0-blue.svg)](https://github.com/)
[![Platform](https://img.shields.io/badge/Platform-Arduino%20Uno%20%28ATmega328P%29-orange.svg)](https://www.arduino.cc/)

The **Light Monitoring and Control Module (LMC)** is a comprehensive embedded firmware system built for an Arduino Uno (ATmega328P) that acts as a low-level Human-Machine Interface (HMI). It dynamically reads ambient light conditions using a Light-Dependent Resistor (LDR) sensor, executes threshold comparison operations against user-specified boundaries, manages localized notifications via an external LCD display and status alarm LED, and exposes a real-time UART-based protocol to interface seamlessly with an external Python-driven Computer Vision application for gesture-based hardware parameter configuration.

---

## 📑 Table of Contents
- [Key Features](#-key-features)
- [System Architecture](#%EF%B8%8F-system-architecture)
- [Hardware & Pin Configuration](#-hardware--pin-configuration)
- [Functional Specifications](#-functional-specifications)
- [State Machine & Operational Logic](#-state-machine--operational-logic)
- [Project Directory & Include Structure](#-project-directory--include-structure)
- [Firmware Configuration Parameters](#%EF%B8%8F-firmware-configuration-parameters)
- [Installation & Getting Started](#-installation--getting-started)
- [Authors & Development Team](#-authors--development-team)

---

## ✨ Key Features

*   **Continuous Ambient Light Sensing:** Interfaces directly with an LDR sensor through the ATmega328P's internal Analog-to-Digital Converter (ADC).
*   **Dual-Threshold Dynamic Monitoring:** Compares real-time values against software-controlled High and Low limits, classifying the environment instantly into **Accept** or **Danger** states.
*   **Local HMI Feedback System:** Provides local visual telemetry on a character LCD configured in an optimized 4-bit interface layout.
*   **Intelligent Local Overrides:** Includes hardware-level debounced push-buttons allowing operators to step through parameters and override thresholds locally.
*   **Computer Vision Integration:** Features an active bidirectional UART protocol framing layer designed to accept on-the-fly threshold adjustments sent from an external camera-driven Python hand-gesture application.
*   **Hardware-Driven PWM Brightness:** Adjusts a dedicated high-output LED brightness using hardware Timer 2 Pulse-Width Modulation (PWM) based on configuration data.

---

## 🏗️ System Architecture

The software is strictly modularized into isolated low-level peripheral drivers, mid-level abstraction modules, and an upper high-level integration orchestrator block:

```
                      +-----------------------------+
                      |       Human.ino (Main)      |
                      +--------------+--------------+
                                     |
    +-------------+-------------+----+----+-------------+-------------+
    |             |             |         |             |             |
+---v---+     +---v---+     +---v---+ +---v---+     +---v---+     +---v---+
| Uart  |     |  Dio  |     |  Adc  | |  Lcd  |     |  Led  |     | Button|
+-------+     +-------+     +-------+ +-------+     +-------+     +-------+
```

### Architectural Assertions & Structural Constraints
*   **Clock Frequency:** Hardcoded parameter at `16000000UL` (16 MHz external crystal oscillator).
*   **Push-Buttons:** Electrically tied via external physical **Pull-Down Resistors** (Logic `Low` when inactive; `High` when depressed).
*   **Debounce Window:** Software delay logic implemented to enforce button stability and ensure positive edge registration (the system blocks execution until the active key is physically released).
*   **LDR Scalar Range:** Normalizes the 10-bit raw ADC readings down to a clean linear scale between `0` and `100`.
*   **UART Serial Buffer Constraints:** Designed around a strict 5-byte ring/input framing layout running at a static baud rate of `9600 bps`.
*   **Logical Boundary Invariance:** High limit parameter boundary checking strictly blocks any scenario where `High Limit < Low Limit`.

---

## 📌 Hardware & Pin Configuration

| Component / Subsystem | MCU Pin Symbol | Physical Pin Function | Electrical Configuration |
|:---|:---|:---|:---|
| **LDR Sensor** | `PC0` | Analog Channel 0 Input | Voltage Divider network |
| **High Limit Button** | `PB3` | Digital Input | External Pull-Down Resistor |
| **Low Limit Button** | `PB2` | Digital Input | External Pull-Down Resistor |
| **LED Control Button** | `PB4` | Digital Input | External Pull-Down Resistor |
| **Alarm / Status LED** | `PB5` | Digital Output | Active-High Direct Drive |
| **Brightness PWM LED** | `PD3` | OC2B Timer 2 Output | Hardware PWM Control |
| **LCD RS Pin** | `PB0` | Digital Output | Control Interface |
| **LCD Enable Pin** | `PB1` | Digital Output | Control Interface |
| **LCD Data Lines (D4:D7)**| `PD4` to `PD7` | 4-Bit Data Bus Outputs | Nibble-wide Data Drive |

---

## ⚙️ Functional Specifications

### 1. Ambient Monitoring & Alarm Engine
*   The ADC driver performs rapid data conversions from `PC0` using `AVcc` as the reference voltage with a customized safety prescaler factor of `128`.
*   The system continually tests boundaries using the condition: 
    $$	ext{Low Limit} \le 	ext{Current LDR Value} \le 	ext{High Limit}$$
*   **Accept State:** The current sensor input satisfies boundary criteria. The string `"Accept"` is drawn onto the character LCD, and the Alarm LED (`PB5`) is driven `LOW` (Off).
*   **Danger State:** The environment strays outside active boundary thresholds. The string `"Danger"` flashes onto the display matrix, and the Alarm LED (`PB5`) is immediately driven `HIGH` (On).

### 2. Gesture Control Interfacing (UART Hand-Gesture App)
*   Upon pressing any of the physical configuration buttons, the controller stops local monitoring and sends a specific hardware flag command upstream via UART to request external updates.
*   The system waits for data packets parsed from the external laptop camera hand-gesture computer vision script.
*   Once a new value is successfully loaded into the UART input frame buffer, the parameters (High Limit, Low Limit, or LED PWM Duty Cycle) are temporarily applied.
*   **Confirmation Sequence:** The operator clicks the exact same button a second time to safely commit the parameter to system variables and resume continuous ambient scanning.

---

## 📂 Project Directory & Include Structure

The source files inside this module are organized as follows:

```
├── Human.ino         # High-level system orchestrator loop and configuration routines
├── Adc.ino           # Native 10-bit ADC peripheral initialization and scaling driver
├── Adc.h             # Function declarations and constant definitions for ADC registers
├── Button.ino        # Hardware button polling and edge debouncing routines
├── Button.h          # Public API mappings for button status evaluation
├── Dio.ino           # Underlying bare-metal data direction and pin output control
├── Dio.h             # Bitwise macros and raw pin mapping definitions
├── Lcd.ino           # 4-bit mode HD44780 character LCD controller subsystem
├── Lcd.h             # LCD structural configuration, commands, and function headers
├── Led.ino           # State logic for Alarm LED and Timer 2 PWM configuration routines
├── Led.h             # Function definitions for LED parameter scaling
├── Uart.ino          # Native hardware UART circular framing routines
├── Uart.h            # UART baud calculations and data frame layout structures
├── myString.ino      # Optimized non-blocking custom string formatting tools
└── myString.h        # Public headers for standard array-based string handling
```

### Include Dependency Schema

```
    [ Human.ino ] 
         │
         ├──► [ Uart.h ]   ──────► ( Serial Comms / Framing Layer )
         ├──► [ Dio.h ]    ──────► ( Fundamental Hardware Control Macros )
         ├──► [ Adc.h ]    ──────► ( Analog Sensor Conversions )
         ├──► [ Lcd.h ]    ──────► ( Multi-Line Matrix Character Control )
         ├──► [ Led.h ]    ──────► ( Alarm Logic & Timer 2 PWM Duty Maps )
         ├──► [ Button.h ] ──────► ( Physical Button Debouncing Controls )
         └──► [ myString.h ] ────► ( Custom String Processing )
```

---

## 🛠️ Firmware Configuration Parameters

These global compilation parameters are fully customizable within `Human.ino` and associated driver headers:

| Parameter Macro | Default Compilation Value | Functional Target Scope |
|:---|:---|:---|
| `F_CPU` | `16000000UL` | Sets peripheral clock ticks for timing dependencies. |
| `BaudRate` | `9600` | Fixes internal hardware USART generation speed. |
| `PWM_LED_PIN` | `PD3 (OC2B)` | Sets the target output pin for LED intensity adjustments. |
| `ALARM_LED_PIN` | `PB5` | Controls the indicator output pin for target violations. |
| `LOW_LIMIT_BTN` | `PB2` | Map location for lower threshold config request tracking. |
| `HIGH_LIMIT_BTN`| `PB3` | Map location for upper threshold config request tracking. |
| `LED_CTRL_BTN` | `PB4` | Map location for target PWM override request tracking. |
| `LOOP_DELAY_MS` | `10` | Standard basic iterative cooling step for main logic cycles. |

---

## 🚀 Installation & Getting Started

### Hardware Prerequisites
1. Connect an **Arduino Uno** development board to your workspace PC.
2. Wire a character LCD matrix in standard **4-bit mode** alongside your hardware control push-buttons, LDR configuration divider circuit, and your indicator LEDs as defined in the [Hardware Configuration Table](#-hardware--pin-configuration).

### Toolchain Setup
1. Group all project files (`*.ino` and `*.h`) inside a unified local project directory explicitly named `Human/`.
2. Open the main orchestrator script `Human.ino` through the **Arduino IDE** or your preferred **PlatformIO** configuration space.
3. Verify that your core profile configuration matches the target **Arduino Uno** architecture.
4. Compile the codebase and flash the binaries onto your micro-controller.

### Interfacing with the Computer Vision Script
1. Launch your external application utilizing the OpenCV tracking library via your laptop's built-in webcam system.
2. Connect your Python application to the same serial device path assigned to your Arduino board (e.g., `COM3` on Windows systems or `/dev/ttyUSB0` under UNIX environments) using a standard speed setting of **9600 baud**.
3. Press a target configuration button on your hardware platform to trigger an upstream interrupt request. Hold your hand up to the camera to vary the parameter values dynamically, and press the button a second time to store the new values permanently.

---

## 👥 Authors & Development Team
*   **Tarek Mahmoud Younes** 
*   **Nour Eldin** 
*   **Kareem Magdy** 
