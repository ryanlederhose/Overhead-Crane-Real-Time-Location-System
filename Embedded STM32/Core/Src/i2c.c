/*
**************************************************************************************************************
* @file     i2c.c
* @author   Ryan Lederhose
* @date     07/02/2023
* @brief    Driver for I2C read, write and function calls
**************************************************************************************************************
*/

#include "i2c.h"

/** Send an I2C function call to the pozyx master tag
 *  Function will send an I2C containing the slave address, memory address and
 *  given function parameters (txData).
 *  It will then sequentially send a read request.
 *  @param hi2c the i2c handle
 *  @param DevAddress the address of the slave
 *  @param MemAddress the address for the register memory
 *  @param MemAddSize the byte size of the memory address
 *  @param txData a pointer to a transmitted data buffer containing the function parameters
 *  @param txSize the size of txData
 *  @param rxData a pointer to a received data buffer in which the read data will be stored
 *  @param rxSize the size of rxData
 *  @param Timeout the timeout time in ms for an i2c transaction
 *  @return HAL status of the communication process
 */
HAL_StatusTypeDef I2C_Send_Function_Call(I2C_HandleTypeDef *hi2c, uint16_t DevAddress,
		uint16_t MemAddress, uint16_t MemAddSize, uint8_t *txData, uint16_t txSize,
		uint8_t *rxData, uint16_t rxSize, uint32_t Timeout) {

	uint8_t txBuffer[MemAddSize + txSize];
	uint8_t rxBuffer[1];
	memset(rxBuffer, '\0', sizeof (rxBuffer));
	memset(txBuffer, '\0', sizeof (txBuffer));

	//Copy the memory address and function parameters to the buffer
	txBuffer[0] = (uint8_t) MemAddress;
	if (txData != NULL) {
		memcpy(txBuffer + 1, txData, txSize);
	}

	while(HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY);

	do {
		//Sequentially transmit the mem address and function parameters
		if (HAL_I2C_Master_Seq_Transmit_IT(hi2c, DevAddress << 1, txBuffer, sizeof (txBuffer), I2C_FIRST_FRAME) != HAL_OK) {
			return HAL_ERROR;
		}

		while(HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY);	//wait for i2c to be ready

	} while (HAL_I2C_GetError(hi2c) == HAL_I2C_ERROR_AF);

	do {
		//Sequentially read the data from the pozyx device
		if (HAL_I2C_Master_Seq_Receive_IT(hi2c, DevAddress << 1, rxData, rxSize, I2C_LAST_FRAME) != HAL_OK) {
			return HAL_ERROR;
		}

		while (HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY);	//wait for i2c to be ready

	} while (HAL_I2C_GetError(hi2c) == HAL_I2C_ERROR_AF);

	while(HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY);

	HAL_Delay(I2C_DELAY);		//small delay between transactions

	return HAL_OK;
}

/** Write a given data buffer to a specified register
 *  @param hi2c pointer to i2c handle
 *  @param MemAddress memory address of register
 *  @param txData pointer to data buffer
 *  @param txSize size of data buffer
 *  @retunr HAL status of i2c communication
 */
HAL_StatusTypeDef I2C_Write_Reg(I2C_HandleTypeDef *hi2c, uint16_t MemAddress,
		uint8_t *txData, uint16_t txSize) {
	while(HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY);	//wait for I2C to be ready

	return HAL_I2C_Mem_Write(hi2c, SLAVE_ADDR << 1, MemAddress,		//write the given data
			I2C_MEMADD_SIZE_8BIT, txData, txSize, 10);
}

/** Read a specified register to a given data buffer
 *  @param hi2c pointer to i2c handle
 *  @param MemAddress memory address of register
 *  @param rxData pointer to data buffer
 *  @param rxSize size of data buffer
 *  @return HAL status of i2c communication
 */
HAL_StatusTypeDef I2C_Read_Reg(I2C_HandleTypeDef *hi2c, uint16_t MemAddress,
		uint8_t *rxData, uint16_t rxSize) {
	while(HAL_I2C_GetState(hi2c) != HAL_I2C_STATE_READY);	//wait for i2c to be ready

	return HAL_I2C_Mem_Read(hi2c, SLAVE_ADDR << 1, MemAddress,	//write the given data
			I2C_MEMADD_SIZE_8BIT, rxData, rxSize, 10);
}


