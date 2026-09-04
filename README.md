# GPS-Wildlife-Tracking-LPC2138
# Real-Time Wildlife Geofencing & Tracking System (LPC2138)

An embedded C bare-metal firmware implementation for the NXP LPC2138 ARM7 microcontroller designed to track animal coordinates via GPS, enforce a 40 sq km bounding box geofence, and trigger automated GSM SMS alerts along with local visual/audible alarms upon boundary breaches.
Designed and tested for LPC2138 microcontrollers with complete schematic support for Proteus VSM simulation.

## Features

* **FPU-Free Optimization:** Custom `coord_to_long` conversion that bypasses software-emulated floating-point arithmetic to prevent CPU lag and memory faults.
* **Robust NMEA Parser:** Manual validation of `$GPRMC` header bytes to ensure reliable sentence filtration, avoiding standard `<string.h>` library quirks.
* **Dual UART Management:** Synchronized polling loops handling high-speed asynchronous data streams simultaneously for GPS input (UART1) and GSM output (UART0).
* **Geofence Boundary Logic:** Fast integer-based evaluation comparing coordinates against predefined boundary macros (`MAX_LAT`, `MIN_LAT`, `MAX_LON`, `MIN_LON`) to monitor a 40 sq km area.
* **Display Interfaces:**
* 16x2 Character LCD (8-bit Mode): Text prompts, system status (`Tracking...`, `Status: SAFE`), and real-time latitude/longitude coordinates.


* **Multi-Modal Alert System:** Triggers a visual status indicator (Green/Red LEDs), an audible alarm (Piezo Buzzer), and an automated AT-command SMS transmission when a geofence breach occurs.

## Hardware & Peripheral Mapping

### LPC2138 Pin Configuration

| Pin | Peripheral / Mode | Pull / Level | Description |
| --- | --- | --- | --- |
| P0.0 | UART0_TX (Output) | Push-Pull | GSM Module TXD0 Interface |
| P0.1 | UART0_RX (Input) | Pull-Up | GSM Module RXD0 Interface (Optional/Unused in simple tx) |
| P0.8 | UART1_TX (Output) | Push-Pull | GPS Module TXD1 Interface (Optional) |
| P0.9 | UART1_RX (Input) | Pull-Up | GPS Module RXD1 Interface |
| P0.13 | GPIO_Output | Push-Pull | Safe Status LED (Green) |
| P0.14 | GPIO_Output | Push-Pull | Breach Alert Status LED (Red) |
| P0.15 | GPIO_Output | Push-Pull | Audible Piezo Buzzer Alarm |
| P1.16 - P1.23 | GPIO_Output | Push-Pull | LCD Data Bus Lines (D0 - D7) |
| P1.24 | GPIO_Output | Push-Pull | LCD Register Select (RS) |
| P1.25 | GPIO_Output | Push-Pull | LCD Read/Write Select (RW) |
| P1.26 | GPIO_Output | Push-Pull | LCD Clock Enable (EN) |

## Circuit Schematic (Proteus VSM)

### 1. 16x2 LCD (LM016L) — 8-Bit Interface

| LPC2138 Pin | LCD Pin | Function |
| --- | --- | --- |
| GND | Pin 1 (VSS) | Ground |
| +5V / +3.3V | Pin 2 (VDD) | Supply Voltage |
| POT WIPER | Pin 3 (VEE/V0) | Contrast Adjustment (10k Potentiometer) |
| P1.24 | Pin 4 (RS) | Register Select |
| P1.25 | Pin 5 (RW) | Read / Write (GND / Write Mode) |
| P1.26 | Pin 6 (E) | Clock Enable |
| P1.16 - P1.23 | Pin 7 - 14 (D0 - D7) | Data Lines D0 to D7 |

### 2. UART Interfaces & Peripherals

| LPC2138 Pin | External Component | Connection Details |
| --- | --- | --- |
| P0.9 (RXD1) | Virtual Terminal 1 (GPS) | TX pin of terminal connected to RXD1 for NMEA stream |
| P0.0 (TXD0) | Virtual Terminal 2 (GSM) | RX pin of terminal connected to TXD0 for AT command logging |
| P0.13 | Green LED | Connected in series with a 330Ω resistor to Ground |
| P0.14 | Red LED | Connected in series with a 330Ω resistor to Ground |
| P0.15 | Piezo Buzzer | Driven via switching transistor or digital output line |
