# Embedded STM32
The code in this folder is intended to run on the STM32L433CT6 microcontroller.

# Description
* Sends positioning requests to the master tag and receives the subsequents positions
* Uses on board ADC to discretise load gauge signal
* Sends positions, ADC strain and crane ID to ZigBee module over UART

# Build Instructions
1. Ensure the STM32CubeIDE is installed on your computer
2. Make a new STM32 project within the CubeIDE
3. Ensure that the STM32L433CCT6 microcontroller is chosen for the project
4. Replace the 'Core' folder within the new project with this 'Core' folder in the GitHub
5. Change the debug option to STLink
6. Attach the STLink to your computer via USB-C
7. Run the project
