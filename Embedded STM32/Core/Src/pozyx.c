/*
**************************************************************************************************************
* @file     pozyx.c
* @author   Ryan Lederhose
* @date     07/02/2023
* @brief    Driver for the Pozyx master tag
**************************************************************************************************************
*/

#include "pozyx.h"

/** Initialise the master tag for operation
 *  @param slaveAddr the address of the slave - this is typically 0x4B
 *  @param hi2c the i2c handle for master tag communication
 *  @return for an error in communication < 0, otherwise 1
 */
int master_tag_init(uint8_t slaveAddr, I2C_HandleTypeDef *hi2c) {
	HAL_StatusTypeDef errCode;
	uint8_t buffer[1];
	uint8_t rxBuffer[50];
	memset(rxBuffer, '\0', sizeof (rxBuffer));

	HAL_Delay(500);	//wait for tag to power up

	//Check status registers
	uint8_t statusRegErrCode = check_status_registers(slaveAddr, hi2c);
	if (statusRegErrCode != GOOD_READ) {
		return statusRegErrCode;
	}

	//Clear devices list
	if ((errCode = I2C_Send_Function_Call(hi2c, slaveAddr, POZYX_DEVICES_CLEAR,
			I2C_MEMADD_SIZE_8BIT, NULL, 0, rxBuffer, sizeof (rxBuffer), 10)) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}

	//Configure interrupts to pin 9 on pozyx, push-pull, latch on and active high
	buffer[0] = ((0x01) | (1 << 4) | (1 << 5));
	if (I2C_Write_Reg(hi2c, POZYX_INT_CONFIG, buffer, sizeof (buffer)) != HAL_OK) {
			return BAD_WRITE_ERROR;
	}

	//Enable interrupts if a new position update is available, if error occurred or if data has been received via uwb
	buffer[0] = ((1 << 0) | (1 << 1) | (1 << 3));
	if (I2C_Write_Reg(hi2c, POZYX_INT_MASK, buffer, sizeof (buffer)) != HAL_OK) {
		return BAD_WRITE_ERROR;
	}

	//Turn off continuous positioning
	buffer[0] = 0x00;
	if (I2C_Write_Reg(hi2c, POZYX_POS_INTERVAL, buffer, sizeof (buffer)) != HAL_OK) {
		return BAD_WRITE_ERROR;
	}
	buffer[0] = 0x00;
	if (I2C_Write_Reg(hi2c, POZYX_POS_INTERVAL + 1,	buffer, sizeof (buffer)) != HAL_OK) {
		return BAD_WRITE_ERROR;
	}

	//Set position algorithm to UWB only
	buffer[0] = (POZYX_POS_ALG_UWB_ONLY | (POZYX_2D << 4));
	if (I2C_Write_Reg(hi2c, POZYX_POS_ALG, buffer, sizeof (buffer)) != HAL_OK) {
		return BAD_WRITE_ERROR;
	}

	//Turn off onboard sensors
	buffer[0] = 0x00;
	if (I2C_Write_Reg(hi2c, POZYX_SENSORS_MODE, buffer, sizeof (buffer)) != HAL_OK) {
		return BAD_WRITE_ERROR;
	}

	//Set moving average filter to a strength of 3
	buffer[0] = (0x03 | (10 << 4));
	if (I2C_Write_Reg(hi2c, POZYX_POS_FILTER, buffer, sizeof (buffer)) != HAL_OK) {
		return BAD_WRITE_ERROR;
	}

	//Set UWB channel to 0x05
	buffer[0] = 0x05;
	if (I2C_Write_Reg(hi2c, POZYX_UWB_CHANNEL, buffer, sizeof (buffer)) != HAL_OK) {
		return BAD_WRITE_ERROR;
	}

	//Set UWB bitrates to 100kbit/s and set PRF to 64MHz
	buffer[0] = (0x00 | (0x02 << 6));
	if (I2C_Write_Reg(hi2c, POZYX_UWB_RATES, buffer, sizeof (buffer)) != HAL_OK) {
		return BAD_WRITE_ERROR;
	}

	return GOOD_INIT;
}

/** Retrieve the positions after a positioning command from the appropriate
 *  pozyx tag registers
 *  @param slaveAddr the address of the slav - this is typically 0x4B
 *  @param hi2c the i2c handle for master tag communication
 *  @param coordinates a pointer to the coordinates struct to store the positions
 *  @return for an error < 0, otherwise > 0
 */
int get_positions(uint8_t slaveAddr, I2C_HandleTypeDef *hi2c, coordinates_t *coordinates) {

	uint8_t rxBufferX[4];
	uint8_t rxBufferY[4];
	uint8_t rxBufferZ[4];
	uint8_t rxBuffer[1];

	memset(rxBufferX, '\0', sizeof (rxBufferX));
	memset(rxBufferY, '\0', sizeof (rxBufferY));
	memset(rxBufferZ, '\0', sizeof (rxBufferZ));
	memset(rxBuffer, '\0', sizeof (rxBuffer));

	for (int i = 1; i <= 4; i++) {

		//Get x positions
		if (I2C_Read_Reg(hi2c, (POZYX_POS_X + (i - 1)),	rxBuffer, sizeof (rxBuffer)) != HAL_OK) {
			return BAD_READ_ERROR;
		}
		rxBufferX[i - 1] = rxBuffer[0];
		memset(rxBuffer, '\0', sizeof (rxBuffer));

		//Get y positions
		if (I2C_Read_Reg(hi2c, (POZYX_POS_Y + (i - 1)),	rxBuffer, sizeof (rxBuffer)) != HAL_OK) {
			return BAD_READ_ERROR;
		}
		rxBufferY[i - 1] = rxBuffer[0];
		memset(rxBuffer, '\0', sizeof (rxBuffer));

		//Get z positions
		if (I2C_Read_Reg(hi2c, (POZYX_POS_Z + (i - 1)), rxBuffer, sizeof (rxBuffer)) != HAL_OK) {
			return BAD_READ_ERROR;
		}
		rxBufferZ[i - 1] = rxBuffer[0];
		memset(rxBuffer, '\0', sizeof (rxBuffer));

	}

	//Clear previous positions
	coordinates->posX &= 0;
	coordinates->posY &= 0;
	coordinates->posZ &= 0;

	//write positions to struct
	for (int j = 0; j < 4; j++) {
		coordinates->posX |= (rxBufferX[j] << (j * 8));
		coordinates->posY |= (rxBufferY[j] << (j * 8));
		coordinates->posZ |= (rxBufferZ[j] << (j * 8));
	}

	return POSITIONS_RETRIEVED;
}

/** Send an I2C message to the master tag containing the relevant data to add a pozyx device to its list
 *  @param slaveAddr the address of the slave
 *  @param hi2c the i2c handle
 *  @param device the relevant pozyx device network ID, flag and positions
 *  @return for an error < 0 otherwise > 0
 */
int add_anchors(uint8_t slaveAddr, I2C_HandleTypeDef *hi2c, deviceCoords_t device) {

	uint8_t txBuffer[15];
	uint8_t rxBuffer[1];
	memset(txBuffer, '\0', sizeof (txBuffer));
	memset(rxBuffer, '\0', sizeof (rxBuffer));

	//Send function call to add device
	if (I2C_Send_Function_Call(hi2c, slaveAddr, POZYX_DEVICE_ADD, I2C_MEMADD_SIZE_8BIT, (uint8_t *) &device,
			sizeof (deviceCoords_t), rxBuffer, sizeof (rxBuffer), 10) != HAL_OK) {
			return BAD_FUNCTION_CALL;
		}

	HAL_Delay(150);

	return DEVICE_ADDED;
}


/** Control the on board LEDs on the master tag
 *  @param slaveAddr the address of the slave - this is typically 0x4B
 *  @param hi2c the i2c handle for master tag communication
 *  @param ledVal the LED in whic to turn on
 *  @return for an error in communication < 0, otherwise 1
 */
int control_tag_led(uint8_t slaveAddr, I2C_HandleTypeDef *hi2c, uint8_t ledVal) {
	uint8_t txBuffer[1];
	uint8_t rxBuffer[1];
	uint8_t bitShiftedLedVal = 1 << (ledVal - 1);

	//Enable override of LED status
	txBuffer[0] = ((bitShiftedLedVal << 4) | bitShiftedLedVal);

	//Send I2C function call - check for an error
	if (I2C_Send_Function_Call(hi2c, slaveAddr, POZYX_LED_CTRL, I2C_MEMADD_SIZE_8BIT,
			txBuffer, sizeof (txBuffer), rxBuffer, sizeof (rxBuffer), 10) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}

	return GOOD_READ;
}

/** Check the status registers on the pozyx master tag for errors
 *  @param slaveAddr the address of the slave
 *  @param hi2c the i2c handle
 *  @return for an error < 0 otherwise > 0
 */
int check_status_registers(uint8_t slaveAddr, I2C_HandleTypeDef *hi2c) {
	HAL_StatusTypeDef errCode;
	uint8_t buffer[1];

	//Check if the WHO_AM_I register reads the write value
	memset(buffer, '\0', sizeof (buffer));
	errCode = I2C_Read_Reg(hi2c, POZYX_WHO_AM_I, buffer, sizeof (buffer));
	if (errCode != HAL_OK) {
		return BAD_READ_ERROR;
	} else {
		if (buffer[0] != 0x43) {
			return INCORRECT_VALUE_ERROR;	//throw an error
		}
	}

	//Check if the firmware version is correct
	memset(buffer, '\0', sizeof (buffer));
	errCode = I2C_Read_Reg(hi2c, POZYX_FIRMWARE_VER, buffer, sizeof (buffer));
	if (errCode != HAL_OK) {
		return BAD_READ_ERROR;
	}

	//Check if the hardware version is correct
	memset(buffer, '\0', sizeof (buffer));
	errCode = I2C_Read_Reg(hi2c, POZYX_HARDWARE_VER, buffer, sizeof (buffer));
	if (errCode != HAL_OK) {
		return BAD_READ_ERROR;
	}

	return GOOD_READ;
}

/** Send the pozyx master tag a positioning request via i2c
 *  @param hi2c the i2c handle
 *  @param slaveAddr the address of the slave
 */
int send_positioning_request(I2C_HandleTypeDef *hi2c, uint16_t slaveAddr) {

	uint8_t rxBuffer[1];
	memset(rxBuffer, '\0', sizeof (rxBuffer));

	//Clear interrupt status register by reading from it
	if (I2C_Read_Reg(hi2c, POZYX_INT_STATUS, rxBuffer, sizeof (rxBuffer)) != HAL_OK) {
		return BAD_READ_ERROR;
	}

	memset(rxBuffer, '\0', sizeof (rxBuffer));

	//Send positioning command
	if (I2C_Send_Function_Call(hi2c, slaveAddr, POZYX_DO_POSITIONING, I2C_MEMADD_SIZE_8BIT,
			NULL, 0, rxBuffer, sizeof (rxBuffer), 10) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}

	return POSITIONS_REQUESTED;
}

/** Use the inbuilt pozyx calibration function to calibrate the anchors position
 *  @param hi2c pointer to i2c handle
 *  @param slaveAddr the address of the slave
 */
int calibrate_positions(I2C_HandleTypeDef *hi2c, uint16_t slaveAddr) {

	uint8_t txBuffer[10];
	uint8_t rxBuffer[1];

	txBuffer[0] = POZYX_2_5D;
	txBuffer[1] = 10;	//number of measurements
	for (int j = 0; j < 2; j++) {
		txBuffer[2 + j] = ((ANCHOR_ID_1 & 0xFF << (j * 8))  >> (j * 8));
		txBuffer[4 + j] = ((ANCHOR_ID_4 & 0xFF << (j * 8))  >> (j * 8));
		txBuffer[6 + j] = ((ANCHOR_ID_3 & 0xFF << (j * 8))  >> (j * 8));
		txBuffer[8 + j] = ((ANCHOR_ID_2 & 0xFF << (j * 8))  >> (j * 8));
	}

	//Send calibration request
	if (I2C_Send_Function_Call(hi2c, slaveAddr, POZYX_DEVICES_CALIBRATE, I2C_MEMADD_SIZE_8BIT,
			txBuffer, sizeof (txBuffer), rxBuffer, sizeof (rxBuffer), 10) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}

	return DEVICE_CALIBRATED;
}

/** Save a writable registers on the master tag in non-volatile flash memory
 *  @param hi2c pointer to i2c handle
 *  @param slaveAddr the address of the slave
 *  @param MemAddr the memory address of the register to save in flash
 */
int flash_register(I2C_HandleTypeDef *hi2c, uint16_t slaveAddr, uint16_t MemAddr) {
	uint8_t txBuffer[2], rxBuffer[1];

	txBuffer[0] = 0x01;
	txBuffer[1] = (uint8_t) MemAddr;

	//send function call to flash register
	if (I2C_Send_Function_Call(hi2c, slaveAddr, POZYX_FLASH_SAVE,
			I2C_MEMADD_SIZE_8BIT, txBuffer, sizeof (txBuffer), rxBuffer, BYTE_SIZE_1, 10) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}

	HAL_Delay(300);

	return GOOD_READ;
}

/** Save the device list on the master tag in non-volatile flash memory
 *  @param hi2c pointer to i2c handle
 *  @param slaveAddr the address of the slave
 */
int flash_device_list(I2C_HandleTypeDef *hi2c, uint16_t slaveAddr) {
	uint8_t txBuffer[1], rxBuffer[1];
	txBuffer[0] = 0x03;

	//send function call to flash device list
	if (I2C_Send_Function_Call(hi2c, slaveAddr, POZYX_FLASH_SAVE,
			I2C_MEMADD_SIZE_8BIT, txBuffer, sizeof (txBuffer), rxBuffer, BYTE_SIZE_1, 10) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}

	HAL_Delay(300);

	return GOOD_READ;
}



