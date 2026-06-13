# I2C Multi-Function Timer Board

A custom-designed embedded timer built on the Arduino Nano ESP32, featuring a physical 
potentiometer, tactile buttons, a 0.96" OLED display, LED signaling, and an audible 
buzzer alert. Developed through a full hardware design cycle — schematic capture, custom 
PCB layout, and manufacturing through JLCPCB — using KiCad. The design and firmware logic 
were fully tested and confirmed in Tinkercad simulation prior to fabrication.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware & Components](#hardware--components)
- [How It Works](#how-it-works)
- [Schematic Design](#schematic-design)
- [PCB Design](#pcb-design)
- [Power Management & Audio](#power-management--audio)
- [Usage](#usage)
- [Design Philosophy](#design-philosophy)
- [Images](#images)

---

## Overview

The I2C Multi-Function Timer Board operates as both a real-time digital clock and a 
customizable countdown timer. In its idle state, the OLED displays the current date and 
time. Once a countdown is set, the display transitions to the active timer interface and 
provides live feedback throughout the session.

The project was built through a complete development cycle: schematic design in KiCad, 
custom PCB layout, fabrication by JLCPCB, and firmware written in C++ using the Arduino 
IDE. Every component is intentionally selected and placed to serve a specific purpose.

The system behavior and core firmware logic were first validated in Tinkercad simulation 
to confirm correct functionality before moving into PCB manufacturing.

The physical PCB was not fully soldered, meaning there was no complete electrical continuity 
during hardware testing. This was intentional, as the primary goal of the fabrication stage 
was to focus on KiCad workflow development, PCB design quality, and layout optimization 
rather than full hardware assembly. Despite this, the circuit design and firmware behavior 
were confirmed to be correct through Tinkercad simulation prior to fabrication.

---

## Features

- Real-time clock and date display on a 0.96" OLED
- Potentiometer for fast, analog minute selection
- Dedicated pushbutton for fine second increments
- Start and Stop buttons for full timer control
- Green and Red LED visual confirmation on Start and Stop
- Dual white power indicator LEDs
- PWM-driven buzzer alert on countdown completion
- Fully custom two-layer PCB with a ground pour on the back

---

## Hardware & Components

The board is built around the **Arduino Nano ESP32**, which handles all timing logic, 
I2C communication with the OLED, and GPIO control for every input and output on the board.

### Component Reference

| Reference | Component           | Value | Function                              |
|-----------|--------------------|-------|---------------------------------------|
| A1        | Arduino Nano ESP32 | —     | Microcontroller                       |
| J1        | OLED Connector     | 4-pin | I2C display (VCC / GND / SCL / SDA)   |
| D1        | LED                | Green | Flash when Start is pressed           |
| D2        | LED                | Red   | Flash when Stop is pressed            |
| D3        | LED                | White | Power indicator                       |
| D4        | LED                | White | Power indicator                       |
| BZ1       | Buzzer             | —     | Audible alert                         |
| SW1       | Tactile Button     | —     | Increment seconds                     |
| SW2       | Tactile Button     | —     | Start timer                           |
| SW3       | Tactile Button     | —     | Stop timer                            |
| RV1       | Potentiometer      | —     | Set minutes (analog input)            |
| C1        | Capacitor          | 0.1µF | Decoupling                            |
| C2        | Capacitor          | 10µF  | Buzzer current buffer                 |
| Q1–Q5     | NPN Transistor     | —     | LED + buzzer switching                |
| R1–R4     | Resistor           | 220Ω  | LED current limiting                  |
| R5–R9     | Resistor           | 1kΩ   | Base resistors                        |

---

## How It Works

The firmware runs two modes: **idle (clock mode)** and **active (timer mode)**.

On startup, the OLED displays real-time date and time. When the user sets a duration 
using the potentiometer and buttons, pressing start switches the system into countdown mode.

During countdown:
- OLED updates live
- LEDs indicate state changes
- Buzzer triggers at zero

Pressing stop exits back to idle mode immediately.

Communication with the OLED uses I2C (SDA/SCL). All outputs are transistor-driven to 
protect the microcontroller.

---

## Schematic Design

Designed in KiCad 9.0.7 with clear subsystem separation:

- LEDs driven through NPN transistors
- Buttons wired to digital inputs with ground reference
- Potentiometer connected to analog input
- OLED connected via I2C
- Buzzer driven using PWM through transistor Q5

Each LED includes a 220Ω resistor, and each transistor base uses a 1kΩ resistor for safe switching.

A 0.1µF decoupling capacitor stabilizes the microcontroller power rail.

---

## PCB Design

Manufactured by JLCPCB as a two-layer PCB.

- Front layer: component routing and signal traces
- Back layer: full ground plane for noise reduction and stability

Design prioritizes:
- Short I2C trace length
- Separated power and signal regions
- Clean grounding via copper pour

At prototype stage, the board was not fully soldered, resulting in incomplete electrical 
continuity during physical testing. However, full circuit behavior and firmware logic 
were validated in Tinkercad simulation prior to fabrication.

---

## Power Management & Audio

Powered via USB through the Arduino Nano ESP32 (3.3V system).

- C1 filters high-frequency noise
- C2 stabilizes buzzer current spikes
- PWM signal controls buzzer tone and timing

The buzzer activates at countdown completion, providing audible feedback even without display visibility.

---

## Usage

1. Power via USB
2. OLED shows clock
3. Set time using potentiometer
4. Adjust seconds if needed
5. Press start
6. Press stop anytime to reset
7. Alarm triggers at zero

---

## Design Philosophy

This project removes reliance on phone-based timers and replaces it with a dedicated physical interface.

The potentiometer allows fast time setting, while buttons provide fine control. Every component exists for a functional reason, emphasizing practical embedded system design rather than abstraction.

Moving from simulation (Tinkercad) to PCB fabrication through KiCad and JLCPCB demonstrates a full embedded development workflow from concept to hardware implementation.

---

## Images

**Schematic**
<img src="https://github.com/user-attachments/assets/751d345d-9680-4d3d-8154-dcac6ccd7f75" />

**PCB Layout**
<img src="https://github.com/user-attachments/assets/6a6f89bf-1c50-467d-80c2-c18c8f27645a" />

**3D Views**
<img src="https://github.com/user-attachments/assets/290cabc9-8447-4586-b611-76094ca5f599" />
<img src="https://github.com/user-attachments/assets/fc7e3ecb-3d04-4fca-9fb0-20c1a068181b" />

---

*Designed in KiCad 9.0.7 — Fabricated by JLCPCB — Built on Arduino Nano ESP32*
