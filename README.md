# Vehicle Fault Detection & Logging System — Embedded C (Dual ATmega32)

A professional-grade **dual-ECU embedded system** for real-time vehicle monitoring, fault detection, and persistent fault logging. Built entirely in **Embedded C** on two **ATmega32** microcontrollers communicating over **UART**, with full layered driver architecture and simulation in **Proteus**. Developed as the **Final Project of the Edges Academy Embedded Systems Diploma**.

---

## System Architecture

The system is split across two independent ATmega32 MCUs with a strict separation of concerns:

```
┌──────────────────────────┐        UART (9600 baud)         ┌──────────────────────────────────┐
│        HMI ECU           │ ◄─────────────────────────────► │         Control ECU              │
│     ATmega32 @ 8MHz      │                                  │        ATmega32 @ 8MHz           │
│                          │                                  │                                  │
│  [Keypad 4x4]            │  Commands → START/DISPLAY        │  [Ultrasonic HC-SR04]  [LM35]    │
│  [LCD 16x4]              │            FAULTS/STOP           │  [External EEPROM I2C]           │
│  [Timer1 CTC 1Hz]        │  ──────────────────────────────► │  [DC Motor x2 via PWM]           │
│                          │                                  │  [Push Buttons x4]               │
│                          │  Live Frame ← 0xAA T D W1 W2    │                                  │
│                          │  Fault Frame← 0xAA C1 C2 0x55   │                                  │
└──────────────────────────┘                                  └──────────────────────────────────┘
```

---

## Features

### Control ECU — Sensor & Actuator Layer
- **Distance measurement** via HC-SR04 Ultrasonic Sensor using ICU (Input Capture Unit) — rising/falling edge pulse timing with formula: `distance = (pulseWidth / 58.8) + 1` cm
- **Temperature sensing** via LM35 analog sensor through configurable ADC (10-bit, internal 2.56V reference, prescaler/64), formula: `temp = (adc × Vref × 100) / 1023`
- **Fault detection** with two defined fault codes:
  - `P001` — Distance < 10 cm (collision risk)
  - `P002` — Engine temperature > 90°C (overheat)
- **Persistent fault logging** to external I2C EEPROM at addresses `0x0000`–`0x0001`, survives power cycles; faults logged once and not overwritten
- **Window motor control** — 2× DC motors individually driven via PWM (Timer0 Fast PWM, F_CPU/8 prescaler) through H-bridge, CW/ACW/STOP states
- **Push button handling** for 4 window control inputs (Window1 open/close, Window2 open/close) with internal pull-ups
- **UART RX interrupt-driven** reception (`USART_RXC_vect`) with 5000-cycle timeout

### HMI ECU — User Interface Layer
- **4×4 Keypad** for user menu navigation
- **16×4 LCD** display with full menu system and real-time data rendering
- **Timer1 CTC** at `OCR1A = 7812` → exact 1 Hz tick at 8 MHz / prescaler 1024, using `TIMER1_COMPA_vect` ISR with callback pattern
- Main menu with 4 options (keys 1–4), live display refresh every 2 seconds, fault display with decoded messages

---

## User Interface — Menu Flow

```
┌─────────────────────────────────────────────────────────────┐
│  Key 1 → START    Initialize sensors & actuators            │
│  Key 2 → DISPLAY  Live sensor data refreshed every 2s       │
│  Key 3 → FAULTS   Read & display EEPROM fault log           │
│  Key 4 → STOP     Stop all motors, halt system              │
└─────────────────────────────────────────────────────────────┘
```

**Live Display (Key 2):**
```
Temp= 37 C
Distance= 25 cm
Win1: CLOSE
Win2: OPEN
```

**Fault Display (Key 3):**
```
Logged Faults:
P001: TooClose
P002: Overheat
--End of List--
```

---

## UART Communication Protocol

**Configuration:** 9600 baud | 8-bit data | No parity | 2 stop bits | RX Interrupt enabled

### Commands — HMI → Control ECU

| Byte | Command | Description |
|------|---------|-------------|
| `1` | `START_CAR` | Initialize sensors & motors, begin monitoring |
| `2` | `DISPLAY` | Request live sensor data frame |
| `3` | `FAULTS` | Request EEPROM fault log |
| `4` | `STOP_CAR` | Stop all motors, de-initialize actuators |
| `5` | `CONT` | Continue — request next frame |
| `0` | End | Stop streaming |

### Live Data Frame — Control → HMI

```
[0xAA] [Temperature °C] [Distance cm] [Window1: 0/1] [Window2: 0/1]
```
> If car not started: `[0xAA] [200=FALSE_START] [0x55]` → HMI shows "Please Start car first"

### Fault Frame — Control → HMI

```
[0xAA] [Fault Code 1] [Fault Code 2] [0x55]
```
> `0xFF` = slot empty (no fault logged), `1` = P001, `2` = P002

---

## Driver Architecture

All drivers follow a layered **HAL (Hardware Abstraction Layer)** pattern with configuration structs for portability and reusability.

### Control ECU Drivers

| Driver | Files | Implementation Details |
|--------|-------|------------------------|
| **GPIO** | `gpio.c/.h` | Port/pin abstraction — `GPIO_setupPinDirection()`, `GPIO_writePin()`, `GPIO_readPin()`, `GPIO_setupPortDirection()` |
| **ADC** | `adc.c/.h` | Configurable via `ADC_ConfigType` struct: Vref (AREF/AVCC/2.56V internal), prescaler (2/4/8/16/32/64/128); polling mode with ADIF flag |
| **UART** | `uart.c/.h` | `UART_ConfigType` struct: baud rate, data bits (5–9), parity (off/odd/even), stop bits (1/2); ISR-driven RX via `USART_RXC_vect`; U2X=1 for double speed |
| **TWI (I2C)** | `twi.c/.h` | 400 kHz fast mode (TWBR=0x02, TWSR=0x00); start/stop/writeByte/readACK/readNACK primitives; TWI status code validated at every step |
| **ICU** | `icu.c/.h` | `ICU_ConfigType` struct: clock select + edge type; `TIMER1_CAPT_vect` ISR calls application callback; `ICU_setEdgeDetectionType()` for edge switching |
| **PWM (Timer0)** | `pwm_t0.c/.h` | Fast PWM mode (WGM00+WGM01), non-inverting (COM01), F_CPU/8 prescaler; `OCR0 = duty_cycle × 256 / 100` |
| **Ultrasonic** | `Ultrasonic.c/.h` | 10 µs trigger pulse on dedicated GPIO; ICU callback counts edge 1 (rising → clear timer → switch to falling) and edge 2 (falling → capture ICR1 → switch back) |
| **LM35** | `lm35_sensor.c/.h` | Reads ADC channel, applies: `temp = (adc_value × g_adcVref × 100) / ADC_MAX` using global Vref set during ADC init |
| **DC Motor** | `DC_motor.c/.h` | Independent H-bridge for Motor1 (IN1/IN2) and Motor2 (IN3/IN4); `DcMotor_Rotate(state, speed, motor_id)` with CW/ACW/STOP; `DcMotor_DeInit()` drives EN low |
| **External EEPROM** | `external_eeprom.c/.h` | 16-bit address I2C EEPROM; write: START → SLA+W → addr_high → addr_low → data → STOP; read: START → SLA+W → addr → RepSTART → SLA+R → data+NACK → STOP |
| **Push Buttons** | `buttons.c/.h` | 4 input pins with internal pull-ups; `button_pressed()` returns 1–4 (active-low detection) or 0 |

### HMI ECU Drivers

| Driver | Files | Implementation Details |
|--------|-------|------------------------|
| **GPIO** | `gpio.c/.h` | Same abstraction layer as Control ECU |
| **UART** | `uart.c/.h` | Same configurable driver mirrored to HMI side |
| **LCD** | `lcd.c/.h` | 16×4 LCD; 4-bit and 8-bit mode selectable via `#define`; `LCD_moveCursor(row, col)`, `LCD_displayString()`, `LCD_intgerToString()`, `LCD_clearScreen()` |
| **Keypad** | `keypad.c/.h` | 4×4 matrix scanning; `KEYPAD_getPressedKey()` returns pressed digit |
| **Timer** | `timer.c/.h` | Unified driver for Timer0/1/2; Normal and CTC modes via `Timer_ConfigType` struct; separate overflow and compare callbacks per timer; `g_sec` global incremented in `TIMER1_COMPA_vect` |

---

## Hardware Configuration Summary

| Peripheral | MCU | Mode | Key Settings |
|------------|-----|------|--------------|
| UART | Both | Async | 9600 baud, 8-bit, 2 stop, no parity, RX ISR |
| Timer0 | Control | Fast PWM | F_CPU/8, non-inverting, OCR0 = duty×256/100 |
| Timer1 | Control | Normal + ICU | F_CPU/8, callback on ICP1/PD6 edge capture |
| Timer1 | HMI | CTC | OCR1A=7812, prescaler 1024 → 1 Hz @ 8 MHz |
| ADC | Control | Single-ended | Internal 2.56V Vref, prescaler 64, polling |
| TWI | Control | I2C Master | 400 kHz fast mode, TWBR=0x02 |
| EEPROM | External | I2C Slave | Device address 0xA0, 16-bit addressing |

---

## Project Structure

```
Vehicle_Fault_Detection/
│
├── Control_ECU/
│   ├── comtrol_ecu.c          # Main — super loop, command dispatch, fault logic, window control
│   ├── Ultrasonic.c/.h        # HC-SR04 ICU-based distance measurement
│   ├── lm35_sensor.c/.h       # LM35 temperature sensor via ADC
│   ├── adc.c/.h               # Configurable ADC driver (Vref + prescaler)
│   ├── uart.c/.h              # Configurable UART driver with ISR RX
│   ├── twi.c/.h               # I2C TWI master driver
│   ├── external_eeprom.c/.h   # External EEPROM read/write over I2C
│   ├── DC_motor.c/.h          # H-bridge DC motor (CW/ACW/STOP + PWM speed)
│   ├── pwm_t0.c/.h            # Timer0 Fast PWM driver
│   ├── icu.c/.h               # Input Capture Unit driver (callback-based)
│   ├── buttons.c/.h           # Push button driver with pull-up inputs
│   ├── gpio.c/.h              # GPIO abstraction layer
│   ├── std_types.h            # uint8, uint16, uint32 type definitions
│   └── common_macros.h        # SET_BIT, CLR_BIT, BIT_IS_CLEAR, GET_BIT macros
│
├── HMI_ECU/
│   ├── hmi_ecu.c              # Main — menu system, UART framing, LCD rendering
│   ├── lcd.c/.h               # 16×4 LCD driver (4-bit / 8-bit selectable)
│   ├── keypad.c/.h            # 4×4 matrix keypad scanning driver
│   ├── uart.c/.h              # UART driver (mirrored from Control ECU)
│   ├── timer.c/.h             # Unified Timer0/1/2 driver (Normal + CTC, callbacks)
│   ├── gpio.c/.h              # GPIO abstraction layer
│   ├── std_types.h            # Standard type definitions
│   └── common_macros.h        # Bit manipulation macros
│
└── Proteus/
    └── Vehicle_Fault_Detection.pdsprj    # Full Proteus simulation
```

---

## Key Embedded Concepts Demonstrated

- **Dual-MCU inter-processor communication** over UART with custom binary framing protocol
- **ISR-driven design** — UART RX interrupt, Timer1 compare interrupt, ICU capture interrupt
- **Callback/function-pointer pattern** — ICU and Timer drivers decouple ISR from application logic via registered callbacks
- **Layered MCAL → HAL → Application architecture** — all peripherals abstracted behind clean driver APIs
- **Configuration struct pattern** — `UART_ConfigType`, `ADC_ConfigType`, `ICU_ConfigType`, `Timer_ConfigType` for portable drivers
- **I2C EEPROM fault logging** — TWI status validated at each transaction step (START, SLA+W, DATA_ACK, REP_START, SLA+R, DATA_NACK)
- **PWM speed control** — linear duty cycle mapping: `OCR0 = duty_cycle × 256 / 100`
- **Super loop with background/foreground separation** — fault detection and window control always run in background; UART commands trigger foreground tasks
- **Volatile shared variables** — `g_car_state`, `g_uart_rx_data`, `g_timeHigh` properly declared volatile for ISR safety

---

## Tools & Environment

| Tool | Purpose |
|------|---------|
| Eclipse IDE + AVR Plugin | Development & build environment |
| AVR-GCC | C compiler targeting ATmega32 |
| Proteus Design Suite | Full circuit & firmware simulation |
| Embedded C (C99) | Language standard |

---

## Author

**Mohamed Baiomy Abdelkader**
Mechatronics & Robotics Engineering Student — Ain Shams University
[LinkedIn](https://linkedin.com/in/mohamed-baiomy) · [GitHub](https://github.com/mb4871787-creator)
