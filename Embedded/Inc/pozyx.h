/*
**************************************************************************************************************
* @file     pozyx.h
* @author   Ryan Lederhose
* @date     07/02/2023
* @brief    Driver for the Pozyx master tag
**************************************************************************************************************
*/

#ifndef INC_POZYX_H_
#define INC_POZYX_H_

#include "main.h"
#include "registers.h"
#include "stdio.h"
#include "stddef.h"

#define BAD_READ_ERROR -1
#define INCORRECT_VALUE_ERROR -2
#define BAD_WRITE_ERROR -3
#define POSITIONS_NOT_READY -4
#define BAD_FUNCTION_CALL -5
#define INT_ERR -6
#define GOOD_READ 1
#define GOOD_INIT 2
#define DEVICE_ADDED 3
#define POSITIONS_RETRIEVED 4
#define INTERRUPT 5
#define POSITIONS_REQUESTED 6
#define DEVICE_CALIBRATED 7
#define TRANSMITTED_MESSAGE 8

/** Initialise the master tag for operation
 *  @param slaveAddr the address of the slave - this is typically 0x4B
 *  @param hi2c the i2c handle for master tag communication
 *  @return for an error in communication < 0, otherwise 1
 */
int master_tag_init(uint8_t slaveAddr, I2C_HandleTypeDef *hi2c);

/** Control the on board LEDs on the master tag
 *  @param slaveAddr the address of the slave - this is typically 0x4B
 *  @param hi2c the i2c handle for master tag communication
 *  @param ledVal the LED in whic to turn on
 *  @return for an error in communication < 0, otherwise 1
 */
int control_tag_led(uint8_t slaveAddr, I2C_HandleTypeDef *hi2c, uint8_t ledVal);

/** Control the on board LEDs on the master tag
 *  @param slaveAddr the address of the slave - this is typically 0x4B
 *  @param hi2c the i2c handle for master tag communication
 *  @param ledVal the LED in whic to turn on
 *  @return for an error in communication < 0, otherwise 1
 */
int add_anchors(uint8_t slaveAddr, I2C_HandleTypeDef *hi2c, deviceCoords_t device);

/** Check the status registers on the pozyx master tag for errors
 *  @param slaveAddr the address of the slave
 *  @param hi2c the i2c handle
 *  @return for an error < 0 otherwise > 0
 */
int check_status_registers(uint8_t slaveAddr, I2C_HandleTypeDef *hi2c);

/** Retrieve the positions after a positioning command from the appropriate
 *  pozyx tag registers
 *  @param slaveAddr the address of the slav - this is typically 0x4B
 *  @param hi2c the i2c handle for master tag communication
 *  @param coordinates a pointer to the coordinates struct to store the positions
 *  @return for an error < 0, otherwise > 0
 */
int get_positions(uint8_t slaveAddr, I2C_HandleTypeDef *hi2c, coordinates_t *coordinates);

/** Send the pozyx master tag a positioning request via i2c
 *  @param hi2c the i2c handle
 *  @param slaveAddr the address of the slave
 */
int send_positioning_request(I2C_HandleTypeDef *hi2c, uint16_t slaveAddr);

/** Use the inbuilt pozyx calibration function to calibrate the anchors position
 *  @param hi2c pointer to i2c handle
 *  @param slaveAddr the address of the slave
 */
int calibrate_positions(I2C_HandleTypeDef *hi2c, uint16_t slaveAddr);

/** Save a writable registers on the master tag in non-volatile flash memory
 *  @param hi2c pointer to i2c handle
 *  @param slaveAddr the address of the slave
 *  @param MemAddr the memory address of the register to save in flash
 */
int flash_register(I2C_HandleTypeDef *hi2c, uint16_t slaveAddr, uint16_t MemAddr);

/** Save the device list on the master tag in non-volatile flash memory
 *  @param hi2c pointer to i2c handle
 *  @param slaveAddr the address of the slave
 */
int flash_device_list(I2C_HandleTypeDef *hi2c, uint16_t slaveAddr);

#endif /* INC_POZYX_H_ */
