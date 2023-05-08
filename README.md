# Overhead-Crane-Real-Time-Location-System
Functioning prototype for Bluescope Acacia Ridge to assist in auditing the travel path and lift rating of overhead cranes

# Author
Ryan Lederhose

# File Layout
    .
    |Overhead-Crane-Real-Time-Location-System/
    |____Python/
    |________serial_to_excel.py
    |________serial_to_db.py
    |________serial_to_mqtt.py
    |____Documentation/
    |________Images/
    |________RTLS Documentation.tex
    |________clean.sh
    |____Embedded/
    |________Inc/
    |____________i2c.h
    |____________pozyx.h
    |____________main.h
    |____________registers.h
    |____________stm32l4xx_hal_conf.h
    |____________stm32l4xx_it.h
    |____________wireless.h
    |____________zigbee.h
    |________Src/
    |____________i2c.c
    |____________main.c
    |____________pozyx.c
    |____________stm32l4xx_hal_msp.c
    |____________stm32l4xx_it.c
    |____________syscalls.c
    |____________sysmem.c
    |____________system_stm32l4xx.c
    |____________wireless.c
    |____________zigbee.c
    |________Startup/
    |____________startup_stm32l433cctx.s
    |____README.md

# Build Instructions

