# Overhead-Crane-Real-Time-Location-System
Functioning prototype for Bluescope Acacia Ridge to assist in auditing the travel path and lift rating of overhead cranes

![alt text](bluescope-logo.png)

# Author
Ryan Lederhose

![alt text](https://media.licdn.com/dms/image/D4E03AQHmpBb2oYg8dQ/profile-displayphoto-shrink_100_100/0/1684370476814?e=1692835200&v=beta&t=NmVOVQkqLnGjUX6LbNndaAuaY1erydQMihyx6VheiuE)

# Functionality
* 2-axis positioning through ultra-wideband two-way ranging
* Hook weight calculation through ADC and linear regression model
* External database storage through ZigBee wireless connection

This prototype is based on the Pozyx Creator One Kit (https://www.pozyx.io/creator-one-kit). The embedded code is built on the 
STM32L433CCT6 microcontroller.

![alt text](https://assets-global.website-files.com/612f4c781c90a5752d371287/63760fd861d895221d8a1243_Creator%20One%20kit-p-1080.webp)

# Documentation
'RTLS Documentation.pdf' provides a full description and layout of the prototype. The 'Resources' folder provides all necessary
schematics, etc.

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
