# Overhead-Crane-Real-Time-Location-System
Functioning prototype for Bluescope Acacia Ridge to assist in auditing the travel path and lift rating of overhead cranes

# Author
Ryan Lederhose

# Functionality
* 2-axis positioning through ultra-wideband two-way ranging
* Hook weight calculation through ADC and linear regression model
* External database storage through ZigBee wireless connection

This prototype is based on the Pozyx Creator One Kit (https://www.pozyx.io/creator-one-kit). The embedded code is built on the 
STM32L433CCT6 microcontroller.

# Documentation
'RTLS Documentation.pdf' provides a full description and layout of the prototype. The 'Resources' folder provides all necessary
schematics, etc.

# Build Instructions
1. Clone the repository using
```bash
git clone https://github.com/ryanlederhose/Overhead-Crane-Real-Time-Location-System.git
```
2. Please see the seperate 'Embedded STM32' build instructions for the microntroller, and 'Host PC' build instructions for the host PC
