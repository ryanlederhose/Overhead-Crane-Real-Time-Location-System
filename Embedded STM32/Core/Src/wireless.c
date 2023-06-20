/*
**************************************************************************************************************
* @file     wireless.c
* @author   Ryan Lederhose
* @date     07/02/2023
* @brief    Driver for wireless communication between master tag and remote tag
**************************************************************************************************************
*/

#include "wireless.h"

/** Remotely connect to a tag specified by the given network address and write to  a register at the given
 *  memory address
 *  @param hi2c pointer to i2c handle
 *  @param networkAddr network address of the tag
 *  @param MemAddress memory address of regiter
 *  @param txData data to be written to register
 *  @param txSize size of txData
 */
int Remote_Write_Reg(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, uint16_t MemAddress, uint8_t *txData, uint16_t txSize) {

	uint8_t rxData[1];
	uint8_t txBuffer[3 + txSize];
	memset(txBuffer, '\0', sizeof (txBuffer));

	txBuffer[0] = 0x00;
	txBuffer[1] = MemAddress;
	if (txData != NULL) {
		memcpy(txBuffer + 2, txData, txSize);
	}

	//Clear interrupt status register
	I2C_Read_Reg(hi2c, POZYX_INT_STATUS, rxData, sizeof (rxData));
	memset(rxData, '\0', sizeof (rxData));

	//Populate data buffer TX_DATA in master tag
	if (I2C_Send_Function_Call(hi2c, SLAVE_ADDR, POZYX_TX_DATA,
			I2C_MEMADD_SIZE_8BIT, txBuffer, BYTE_SIZE_2, rxData, sizeof (rxData), 10) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}

	//Check for error in function
	if (rxData[0] == 0) {
		return BAD_FUNCTION_CALL;
	}

	memset(txBuffer, '\0', sizeof (txBuffer));
	txBuffer[0] = (networkAddr & 0xFF);
	txBuffer[1] = ((networkAddr & (0xFF << 8)) >> 8);
	txBuffer[2] = 0x04;

	//Send data remotely to give network address
	if (I2C_Send_Function_Call(hi2c, SLAVE_ADDR, POZYX_TX_SEND,
			I2C_MEMADD_SIZE_8BIT, txBuffer, BYTE_SIZE_3, rxData, sizeof (rxData), 10) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}

	//Check for error
	if (rxData[0] == 0) {
		return BAD_FUNCTION_CALL;
	}

	return TRANSMITTED_MESSAGE;
}

/** Remotely connect to a tag specified by the given network address and read a register at the given
 *  memory address
 *  @param hi2c pointer to i2c handle
 *  @param networkAddr network address of the tag
 *  @param MemAddress memory address of regiter
 *  @param regSize register size to read
 */
int Remote_Read_Reg(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, uint16_t MemAddress, uint16_t regSize) {

	uint8_t rxData[1];
	uint8_t txBuffer[3];

	memset(rxData, '\0', sizeof (rxData));
	memset(txBuffer, '\0', sizeof (txBuffer));

	txBuffer[0] = 0x00;
	txBuffer[1] = MemAddress;
	txBuffer[2] = regSize;

	//Populate data buffer TX_DATA in master tag
	if (I2C_Send_Function_Call(hi2c, SLAVE_ADDR, POZYX_TX_DATA,
			I2C_MEMADD_SIZE_8BIT, txBuffer, sizeof (txBuffer), rxData, sizeof (rxData), 10) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}

	//Check for error in function
	if (rxData[0] == 0) {
		return BAD_FUNCTION_CALL;
	}

	memset(txBuffer, '\0', sizeof (txBuffer));
	memset(rxData, '\0', sizeof (rxData));

	//Clear int status register
	I2C_Read_Reg(hi2c, POZYX_INT_STATUS, rxData, sizeof (rxData));
	memset(rxData, '\0', sizeof (rxData));

	txBuffer[0] = (networkAddr & 0xFF);
	txBuffer[1] = ((networkAddr & (0xFF << 8)) >> 8);
	txBuffer[2] = 0x02;

	//Send data remotely to give network address
	if (I2C_Send_Function_Call(hi2c, SLAVE_ADDR, POZYX_TX_SEND,
			I2C_MEMADD_SIZE_8BIT, txBuffer, sizeof (txBuffer), rxData, sizeof (rxData), 10) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}

	//Check for error
	if (rxData[0] == 0) {
		return BAD_FUNCTION_CALL;
	}

	return TRANSMITTED_MESSAGE;
}

/** Remotely connect to a tag specified by the given network address and perform a function call at the
 * register at the given memory address
 *  @param hi2c pointer to i2c handle
 *  @param networkAddr network address of the tag
 *  @param MemAddress memory address of regiter
 *  @param txData parameters of function
 *  @param txSize size of txData
 */
int Remote_Function_Call(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, uint16_t MemAddress, uint8_t *txData, uint16_t txSize) {

	uint8_t rxData[1];
	uint16_t rxSize = sizeof (rxData);

	uint8_t txBuffer[txSize + I2C_MEMADD_SIZE_8BIT + 1];
	txBuffer[0] = 0x00;
	txBuffer[1] = MemAddress;
	if (txSize > 0) {
		memcpy(txBuffer + 2, txData, txSize);
	}

	//Clear int status register
	I2C_Read_Reg(hi2c, POZYX_INT_STATUS, rxData, sizeof (rxData));
	memset(rxData, '\0', sizeof (rxData));

	//Populate data buffer TX_DATA in master tag
	if (I2C_Send_Function_Call(hi2c, SLAVE_ADDR, POZYX_TX_DATA,
			I2C_MEMADD_SIZE_8BIT, txBuffer, sizeof (txBuffer), rxData, rxSize, 10) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}

	//Check for error in function
	if (rxData[0] == 0) {
		return BAD_FUNCTION_CALL;
	}

	memset(txBuffer, '\0', sizeof (txBuffer));
	txBuffer[0] = (networkAddr & 0xFF);
	txBuffer[1] = ((networkAddr & (0xFF << 8)) >> 8);
	txBuffer[2] = 0x08;

	//Clear int status register
	I2C_Read_Reg(hi2c, POZYX_INT_STATUS, rxData, sizeof (rxData));
	memset(rxData, '\0', sizeof (rxData));

	//Send data remotely to give network address
	if (I2C_Send_Function_Call(hi2c, SLAVE_ADDR, POZYX_TX_SEND,
			I2C_MEMADD_SIZE_8BIT, txBuffer, BYTE_SIZE_3, rxData, rxSize, 10) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}

	//Check for error
	if (rxData[0] == 0) {
		return BAD_FUNCTION_CALL;
	}

	return TRANSMITTED_MESSAGE;
}

/** Read the Rx buffer on the pozyx master tag
 *  @param hi2c pointer to a i2c handle
 *  @param rxData data buffer to store received data
 *  @param rxSize size of rxData
 */
HAL_StatusTypeDef Read_Rx_Buffer(I2C_HandleTypeDef *hi2c, uint8_t *rxData, uint16_t rxSize) {

	uint8_t rxBuffer[1], txBuffer[1];

	//wait until rx data buffer full
	while ((rxBuffer[0] & POZYX_INT_STATUS_RX_DATA) == POZYX_INT_STATUS_RX_DATA) {
		I2C_Read_Reg(hi2c, POZYX_INT_STATUS, rxBuffer, sizeof (rxBuffer));
	}

	txBuffer[0] = 0x00;

	if (I2C_Send_Function_Call(hi2c, SLAVE_ADDR, POZYX_RX_DATA,
			I2C_MEMADD_SIZE_8BIT, txBuffer, BYTE_SIZE_1, rxData, rxSize, 10) != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;
}

/** Remotely connect to a tag at the specified network address and perform a function call
 *  at the register given by the memory address. Sequentially read the Rx buffer populated with data
 *  through UWB transmission
 *  @param hi2c pointer to i2c handle
 *  @param networkAddr network address of remote tag
 *  @param MemAddress memory address of register
 *  @param txData parameters of function
 *  @param txSize size of txData
 *  @param rxData data buffer to read Rx buffer in to
 *  @param rxSize size of data to read in to
 */
int Remote_Function_Call_Read(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, uint16_t MemAddress,
		uint8_t *txData, uint16_t txSize, uint8_t *rxData, uint16_t rxSize) {
	if (Remote_Function_Call(hi2c, networkAddr, MemAddress, txData, txSize) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}
	if (Read_Rx_Buffer(hi2c, rxData, rxSize) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}
	return TRANSMITTED_MESSAGE;
}

/** Send a read request to a remote tag specified by the network address. Subsequently read the received
 *  buffer populated with the data of the requested read register on the remote tag
 *  @param hi2c pointer to i2c handle
 *  @param networkAddr network address of remote tag
 *  @param MemAddress memory address of remote tag register
 *  @param rxData data buffer to populate Rx buffer with
 *  @param rxSize size of data to read in to
 *  @param regSize register size to read
 */
int Remote_Read_Reg_Read(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, uint16_t MemAddress,
		uint8_t *rxData, uint16_t rxSize, uint16_t regSize) {
	if (Remote_Read_Reg(hi2c, networkAddr, MemAddress, regSize) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}
	if (Read_Rx_Buffer(hi2c, rxData, rxSize) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}
	return TRANSMITTED_MESSAGE;
}

/** Remotely connect to a tag at the specified network address and perform a write to the given
 *  register specified by the memory address. Subsequently read the Rx buffer populated from UWB
 *  @param hi2c pointer to i2c handle
 *  @param networkAddr network address of remote tag
 *  @param MemAddress memory address of register
 *  @param txData data to write to register
 *  @param txSize size of txData
 *  @param rxData data buffer to read Rx buffer in to
 *  @param rxSize size of data to read in to
 */
int Remote_Write_Reg_Read(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, uint16_t MemAddress,
		uint8_t *txData, uint16_t txSize, uint8_t *rxData, uint16_t rxSize) {
	if (Remote_Write_Reg(hi2c, networkAddr, MemAddress, txData, txSize) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}
	if (Read_Rx_Buffer(hi2c, rxData, rxSize) != HAL_OK) {
		return BAD_FUNCTION_CALL;
	}
	return TRANSMITTED_MESSAGE;
}

/** Initialise a remote tag for positioning
 *  @param hi2c pointer to a i2c handle
 *  @parm networkAddr the network address of the remote tag
 *  @return < 0 for an error, otherwise > 0
 */
int remote_tag_init(I2C_HandleTypeDef *hi2c, uint16_t networkAddr) {

	HAL_Delay(2500);	//wait for tag to power up

	uint8_t txBuffer[10], rxBuffer[10];
	memset(txBuffer, '\0', sizeof (txBuffer));
	memset(rxBuffer, '\0', sizeof (rxBuffer));

	//Clear devices list
	if (Remote_Function_Call_Read(hi2c, networkAddr, POZYX_DEVICES_CLEAR, NULL,
			0, rxBuffer, BYTE_SIZE_2) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}
	if (rxBuffer[1] != 0x01) {
		return BAD_FUNCTION_CALL;
	}

	memset(rxBuffer, '\0', sizeof (rxBuffer));

	//Set position algorithm to UWB only
	txBuffer[0] = (POZYX_POS_ALG_UWB_ONLY | (DIMENSION << 4));
	if (Remote_Write_Reg_Read(hi2c, networkAddr, POZYX_POS_ALG, txBuffer,
			BYTE_SIZE_1, rxBuffer, BYTE_SIZE_2) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}
	if (rxBuffer[0] == 0x00) {
		return BAD_FUNCTION_CALL;
	}

	remote_flash_register(hi2c, networkAddr, POZYX_POS_ALG_UWB_ONLY);

	memset(txBuffer, '\0', sizeof (txBuffer));
	memset(rxBuffer, '\0', sizeof (rxBuffer));

	//Turn off on board sensors
	txBuffer[0] = 0x00;
	if (Remote_Write_Reg_Read(hi2c, networkAddr, POZYX_SENSORS_MODE, txBuffer,
			BYTE_SIZE_1, rxBuffer, BYTE_SIZE_2) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}
	if (rxBuffer[0] == 0x00) {
		return BAD_FUNCTION_CALL;
	}

	remote_flash_register(hi2c, networkAddr, POZYX_SENSORS_MODE);

	memset(txBuffer, '\0', sizeof (txBuffer));
	memset(rxBuffer ,'\0', sizeof (rxBuffer));

	//Set moving average filter to a strength of 10
	txBuffer[0] = (0x04 | (10 << 4));
	if (Remote_Write_Reg_Read(hi2c, networkAddr, POZYX_POS_FILTER, txBuffer,
			BYTE_SIZE_1, rxBuffer, BYTE_SIZE_2) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}
	if (rxBuffer[0] == 0x00) {
		return BAD_FUNCTION_CALL;
	}

	remote_flash_register(hi2c, networkAddr, POZYX_POS_FILTER);

	return GOOD_INIT;
}

/** Add an anchor to the internal list of a remote tag
 *  @param hi2c i2c handle
 *  @param device the relevant pozyx device network ID, flag and position
 *  @param networkAddr the network address of the remote tag
 *  @return < 0 for an error, otherwise > 0
 */
int remote_add_anchors(I2C_HandleTypeDef *hi2c, deviceCoords_t device, uint16_t networkAddr) {
	uint8_t txBuffer[15];
	uint8_t rxBuffer[15];
	memset(txBuffer, '\0', sizeof (txBuffer));
	memset(rxBuffer, '\0', sizeof (rxBuffer));

	//send function call to add anchor to remote tag memory
	if (Remote_Function_Call_Read(hi2c, networkAddr, POZYX_DEVICE_ADD, (uint8_t *) &device,
			sizeof (deviceCoords_t), rxBuffer, BYTE_SIZE_2) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}
	if (rxBuffer[0] != 0x01 && rxBuffer[1] != 0x01) {
		return BAD_FUNCTION_CALL;
	}

	return DEVICE_ADDED;

}

/** Position a remote tag given by the network address & save the positions to a given struct
 *  @param hi2c i2c handle
 *  @param networkAddr the network address of the tag
 *  @param coordinates position struct
 *  @return < 0 for an error, otherwise > 0
 */
int remote_positioning(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, coordinates_t *coordinates) {
	uint8_t rxBuffer[50];
	memset(rxBuffer, '\0', sizeof (rxBuffer));

	//Clear interrupt status register by reading from it
	if (I2C_Read_Reg(hi2c, POZYX_INT_STATUS, rxBuffer, sizeof (rxBuffer)) != HAL_OK) {
		return BAD_READ_ERROR;
	}

	memset(rxBuffer, '\0', sizeof (rxBuffer));

	//Send positioning command
	if (Remote_Function_Call_Read(hi2c, networkAddr, POZYX_DO_POSITIONING, NULL,
			0, rxBuffer, sizeof (rxBuffer)) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}

	uint8_t rxBufferX[4];
	uint8_t rxBufferY[4];
	uint8_t rxBufferZ[4];

	memset(rxBufferX, '\0', sizeof (rxBufferX));
	memset(rxBufferY, '\0', sizeof (rxBufferY));
	memset(rxBufferZ, '\0', sizeof (rxBufferZ));
	memset(rxBuffer, '\0', sizeof (rxBuffer));

	HAL_Delay(70);

	//Get x positions
	if (Remote_Read_Reg_Read(hi2c, networkAddr, POZYX_POS_X, rxBuffer,
			BYTE_SIZE_1 * 5, BYTE_SIZE_2 * 2) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}

	for (int i = 0; i < 4; i++) {
		rxBufferX[3 - i] = rxBuffer[4 - i];
	}

	memset(rxBuffer, '\0', sizeof (rxBuffer));

	//Get y positions
	if (Remote_Read_Reg_Read(hi2c, networkAddr, POZYX_POS_Y, rxBuffer,
			BYTE_SIZE_1 * 5, BYTE_SIZE_2 * 2) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}

	for (int i = 0; i < 4; i++) {
		rxBufferY[3 - i] = rxBuffer[4 - i];
	}

	coordinates->posX &= 0;
	coordinates->posY &= 0;
	coordinates->posZ &= 0;

	for (int j = 0; j < 4; j++) {
		coordinates->posX |= (rxBufferX[j] << (j * 8));
		coordinates->posY |= (rxBufferY[j] << (j * 8));
		coordinates->posZ |= (rxBufferZ[j] << (j * 8));
	}

	return POSITIONS_RETRIEVED;
}

/** Perform a remote calibration on the remote tag specified by the network address
 *  anchorIDs provide the relevant anchor IDs to calibrate.
 *  	anchorID1 -> this anchor will be used as the origin
 *  	anchorID2 -> this anchor will be used to determine the x-axis (y-axis is zero)
 *  	anchorID3 -> this anchor will be used to determine the y-axis (x-axis is zero)
 *  	anchorID4, etc. -> optional anchors for further calibration
 *  @param hi2c pointer to i2c handle
 *  @param networkAddr the network address of the remote tag
 *  @param anchorIDs the anchor IDs in order needed for calibration as specified above
 */
int remote_calibration(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, calibration_t anchorIDs) {

	uint8_t txBuffer[10];
	uint8_t rxBuffer[2];

	txBuffer[0] = DIMENSION;
	txBuffer[1] = 10;	//number of measurements
	for (int j = 0; j < 2; j++) {
		txBuffer[2 + j] = ((anchorIDs.anchorID1 & 0xFF << (j * 8))  >> (j * 8));
		txBuffer[4 + j] = ((anchorIDs.anchorID2 & 0xFF << (j * 8))  >> (j * 8));
		txBuffer[6 + j] = ((anchorIDs.anchorID3 & 0xFF << (j * 8))  >> (j * 8));
		txBuffer[8 + j] = ((anchorIDs.anchorID4 & 0xFF << (j * 8))  >> (j * 8));
	}

	if (Remote_Function_Call_Read(hi2c, networkAddr, POZYX_DEVICES_CALIBRATE, txBuffer,
			sizeof (txBuffer), rxBuffer, sizeof (rxBuffer)) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}

	HAL_Delay(txBuffer[1] * 500);	//wait for calibration to finish, allow 250ms for each calibration measurement

	return DEVICE_CALIBRATED;
}

/** Save a writable registers on a remote tag in non-volatile flash memory
 *  @param hi2c pointer to i2c handle
 *  @param networkAddr the network address of the remote tag
 *  @param MemAddr the memory address of the register to save in flash
 */
int remote_flash_register(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, uint16_t MemAddr) {
	uint8_t txBuffer[2], rxBuffer[2];
	txBuffer[0] = 0x01;
	txBuffer[1] = (uint8_t) MemAddr;

	if (Remote_Function_Call_Read(hi2c, networkAddr, POZYX_FLASH_SAVE, txBuffer,
			sizeof (txBuffer), rxBuffer, BYTE_SIZE_2) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}

	HAL_Delay(300);

	return GOOD_READ;
}

/** Save the device list on a remote tag in non-volatile flash memory
 *  @param hi2c pointer to i2c handle
 *  @param networkAddr the network address of the remote tag
 */
int remote_save_device_list(I2C_HandleTypeDef *hi2c, uint16_t networkAddr) {
	uint8_t txBuffer[1], rxBuffer[2];
	txBuffer[0] = 0x03;

	if (Remote_Function_Call_Read(hi2c, networkAddr, POZYX_FLASH_SAVE, txBuffer,
			sizeof (txBuffer), rxBuffer, sizeof (rxBuffer)) != TRANSMITTED_MESSAGE) {
		return BAD_FUNCTION_CALL;
	}

	HAL_Delay(300);

	return GOOD_READ;
}


