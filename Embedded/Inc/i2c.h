/*
**************************************************************************************************************
* @file     i2c.h
* @author   Ryan Lederhose
* @date     07/02/2023
* @brief    Driver for I2C read, write and function calls
**************************************************************************************************************
*/

#ifndef INC_I2C_H_
#define INC_I2C_H_

#include "main.h"
#include "registers.h"

#define I2C_DELAY 5

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
HAL_StatusTypeDef I2C_Send_Function_Call(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress,
		uint16_t MemAddSize, uint8_t *txData, uint16_t txSize,
		uint8_t *rxData, uint16_t rxSize, uint32_t Timeout);

/** Write a given data buffer to a specified register
 *  @param hi2c pointer to i2c handle
 *  @param MemAddress memory address of register
 *  @param txData pointer to data buffer
 *  @param txSize size of data buffer
 *  @return HAL status of i2c communication
 */
HAL_StatusTypeDef I2C_Write_Reg(I2C_HandleTypeDef *hi2c, uint16_t MemAddress,
		uint8_t *txData, uint16_t txSize);

/** Read a specified register to a given data buffer
 *  @param hi2c pointer to i2c handle
 *  @param MemAddress memory address of register
 *  @param rxData pointer to data buffer
 *  @param rxSize size of data buffer
 *  @return HAL status of i2c communication
 */
HAL_StatusTypeDef I2C_Read_Reg(I2C_HandleTypeDef *hi2c, uint16_t MemAddress,
		uint8_t *rxData, uint16_t rxSize);

#endif /* INC_I2C_H_ */
