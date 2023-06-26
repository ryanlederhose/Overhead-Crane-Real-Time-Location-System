# Overhead-Crane-Real-Time-Location-System
Functioning prototype to assist in auditing the travel path and lift rating of overhead cranes

# Author
Ryan Lederhose

# Functionality
* 2-axis positioning through ultra-wideband two-way ranging
* Hook weight calculation through ADC and linear regression model
* External database storage through ZigBee wireless connection


https://github.com/ryanlederhose/Overhead-Crane-Real-Time-Location-System/assets/112144274/d8b95483-c45d-4638-85e9-6efbddddd37b


This prototype is based on the Pozyx Creator One Kit (https://www.pozyx.io/creator-one-kit). The embedded code is built on the 
STM32L433CCT6 microcontroller.

https://github.com/ryanlederhose/Overhead-Crane-Real-Time-Location-System/assets/112144274/8b0b4897-9c23-4334-9466-ac720546a4e4

# File Description
* Documentation contains all necessary documents for the prototype
* Embedded STM32 contains code intended to run on the STM32 microcontroller
* Host PC contains code intended to run on a host PC
* Test Results contains data from the testing phase of production
  
# Build Instructions
1. Clone the repository using
```bash
git clone https://github.com/ryanlederhose/Overhead-Crane-Real-Time-Location-System.git
```
2. Please see the seperate 'Embedded STM32' build instructions for the microntroller, and 'Host PC' build instructions for the host PC
