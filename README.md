# AVR C++ Library for nRF24L01+ (Ported from nRF24/RF24)

This repo is a small, low-level C++ port of the nRF24/RF24 library for ATmega328P microcontroller. It works standalone in pure AVR C++, without Arduino IDE or framework.

## Purpose
- This repository was created only for my personal use and learning purpose.
- It has only the main methods needed for a stable wireless connection with Auto-ACK protocol. 
- I share this code to public because it can be useful reference for someone who want port Arduino libraries to pure AVR hardware.
- It is mainly copyed from RF24.h with small refactors to AVR compatibility, using just 1 simple method and ignoring elses methods.
- The original register definitions belong to their authors (Stefan Engelke & Greg Copeland via nRF24 community).

## Project Structure

Your project folder must look like this:
```text
├── RF24/
│   └── nRF24L01.h     <-- Original register file from Stefan Engelke
├── main.cpp           <-- Transmitter logic
├── nrf24l01.cpp       <-- Ported nRF24L01+ protocol state machine
├── nrf24l01.h         <-- Refactored clean class definition for AVR
├── spi.cpp            <-- Hardware SPI for ATmega328P
├── spi.h              <-- SPI pin definitions and macros
└── Makefile           <-- GNU Make script for toolchain
```

## How to Run

### 1. Download Registry File
You must download the official register file from original repository:
- Go to GitHub: https://github.com
- Download the file named "nRF24L01.h"
- Put it inside a subfolder named "RF24" in your project folder.

### 2. Compilation and Flashing
Open terminal in the project directory and use these commands:

To clean old builds and compile the binary file:
```bash
make clean
make all
```

To flash the code into your ATmega328P with USBasp programmer:
```bash
make flash
```

## License
- The file RF24/nRF24L01.h is under its original MIT License (Copyright 2007 Stefan Engelke).
- All other files (spi.*, nrf24l01.*, main.cpp, Makefile) were created for my purpose under MIT licence.
