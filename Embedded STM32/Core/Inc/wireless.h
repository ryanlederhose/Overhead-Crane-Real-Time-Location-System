/*
**************************************************************************************************************
* @file     wireless.h
* @author   Ryan Lederhose
* @date     07/02/2023
* @brief    Driver for wireless communication between master tag and remote tag
**************************************************************************************************************
*/

#ifndef INC_WIRELESS_H_
#define INC_WIRELESS_H_

#include "main.h"
#include "registers.h"
#include "pozyx.h"

/** Remotely connect to a tag specified by the given network address and write to  a register at the given
 *  memory address
 *  @param hi2c pointer to i2c handle
 *  @param networkAddr network address of the tag
 *  @param MemAddress memory address of regiter
 *  @param txData data to be written to register
 *  @param txSize size of txData
 */
int Remote_Write_Reg(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, uint16_t MemAddress, uint8_t *txData, uint16_t txSize);

/** Remotely connect to a tag specified by the given network address and read a register at the given
 *  memory address
 *  @param hi2c pointer to i2c handle
 *  @param networkAddr network address of the tag
 *  @param MemAddress memory address of regiter
 *  @param regSize register size to read
 */
int Remote_Read_Reg(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, uint16_t MemAddress, uint16_t regSize);

/** Remotely connect to a tag specified by the given network address and perform a function call at the
 * register at the given memory address
 *  @param hi2c pointer to i2c handle
 *  @param networkAddr network address of the tag
 *  @param MemAddress memory address of regiter
 *  @param txData parameters of function
 *  @param txSize size of txData
 */
int Remote_Function_Call(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, uint16_t MemAddress, uint8_t *txData, uint16_t txSize);

/** Read the Rx buffer on the pozyx master tag
 *  @param hi2c pointer to a i2c handle
 *  @param rxData data buffer to store received data
 *  @param rxSize size of rxData
 */
HAL_StatusTypeDef Read_Rx_Buffer(I2C_HandleTypeDef *hi2c, uint8_t *rxData, uint16_t rxSize);

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
							  uint8_t *txData, uint16_t txSize, uint8_t *rxData, uint16_t rxSize);

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
						 uint8_t *rxData, uint16_t rxSize, uint16_t regSize);

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
						  uint8_t *txData, uint16_t txSize, uint8_t *rxData, uint16_t rxSize);

/** Initialise a remote tag for positioning
 *  @param hi2c pointer to a i2c handle
 *  @parm networkAddr the network address of the remote tag
 *  @return < 0 for and error, other > 0
 */
int remote_tag_init(I2C_HandleTypeDef *hi2c, uint16_t networkAddr);

/** Add an anchor to the internal list of a remote tag
 *  @param hi2c i2c handle
 *  @param device the relevant pozyx device network ID, flag and position
 *  @param networkAddr the network address of the remote tag
 *  @return < 0 for an error, otherwise > 0
 */
int remote_add_anchors(I2C_HandleTypeDef *hi2c, deviceCoords_t device, uint16_t networkAddr);

/** Position a remote tag given by the network address & save the positions to a given struct
 *  @param hi2c i2c handle
 *  @param networkAddr the network address of the tag
 *  @param coordinates position struct
 *  @return < 0 for an error, otherwise > 0
 */
int remote_positioning(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, coordinates_t *coordinates);

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
int remote_calibration(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, calibration_t anchorIDs);

/** Save a writable registers on a remote tag in non-volatile flash memory
 *  @param hi2c pointer to i2c handle
 *  @param networkAddr the network address of the remote tag
 *  @param MemAddr the memory address of the register to save in flash
 */
int remote_flash_register(I2C_HandleTypeDef *hi2c, uint16_t networkAddr, uint16_t MemAddr);

/** Save the device list on a remote tag in non-volatile flash memory
 *  @param hi2c pointer to i2c handle
 *  @param networkAddr the network address of the remote tag
 */
int remote_save_device_list(I2C_HandleTypeDef *hi2c, uint16_t networkAddr);

#endif /* INC_WIRELESS_H_ */
