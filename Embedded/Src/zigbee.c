/*
**************************************************************************************************************
* @file     zigbee.c
* @author   Ryan Lederhose
* @date     07/02/2023
* @brief    Driver for zigbee communication
**************************************************************************************************************
*/

#include "zigbee.h"

/** Send the given positions, mass and ID of the crane through the zigbee modules
 *  @param huart pointer to uart handle
 *  @param positions struct holding (x, y) positions of crane
 *  @param mass raw adc value of crane load gauge
 *  @param craneID id of crane
 */
void zigbee_send_data(UART_HandleTypeDef *huart, coordinates_t positions, uint32_t mass, uint8_t craneID) {
	int32_t posX = positions.posX;
	int32_t posY = positions.posY;

	//Get number of digits in positions, mass and id
	uint8_t posXSize = log10(posX) + 1;
	uint8_t posYSize = log10(posY) + 1;
	uint8_t massSize = log10(mass) + 1;
	uint8_t idSize = log10(craneID) + 1;

	//If mass and positions are zero, set number of digits accordingly
	if (posXSize == 0) {
		posXSize = 1;
	}
	if (posYSize == 0) {
		posYSize = 1;
	}
	if (massSize == 0) {
		massSize = 1;
	}

	//populate char arrays with positions, mass and id
	char posYArr[posYSize + 3];
	memset(posYArr, 0, sizeof (posYArr));
	sprintf(posYArr, "y%d\r\n", posY);

	char idArr[idSize + 2];
	memset(idArr, 0, sizeof (idArr));
	sprintf(idArr, "i%d ", craneID);

	char posXArr[posXSize + 2];
	memset(posXArr, 0, sizeof (posXArr));
	sprintf(posXArr, "x%d ", posX);

	char massArr[massSize + 2];
	memset(massArr, 0, sizeof (massArr));
	sprintf(massArr, "m%d ", mass);

	//integrate char arrays and other commands for zigbee communication
	uint8_t dataLength = sizeof (posXArr) + sizeof (posYArr) + sizeof (massArr) + sizeof (idArr);
	uint8_t data[4 + dataLength];
	data[0] = 0xFD;
	data[1] = dataLength;
	data[2] = 0xFF;
	data[3] = 0xFF;
	memcpy(data + 4, idArr, sizeof (idArr));
	memcpy(data + 4 + sizeof (idArr), massArr, sizeof (massArr));
	memcpy(data + 4 + sizeof (idArr) + sizeof (massArr), posXArr, sizeof (posXArr));
	memcpy(data + 4 + sizeof (idArr) + sizeof (massArr) + sizeof (posXArr), posYArr, sizeof (posYArr));

	HAL_UART_Transmit(huart, data, sizeof (data), 10);	//send data
}

/** Send through a set data buffer through zigbee
 *  @param huart pointer to uart handlee
 *  @param txData pointer to data buffer
 *  @param txSize size of data buffer
 */
void zigbee_send_other_data(UART_HandleTypeDef *huart, uint8_t *txData, uint16_t txSize) {

	uint8_t txBuffer[4 + txSize];
	memset(txBuffer, 0, sizeof (txBuffer));

	txBuffer[0] = 0xFD;
	txBuffer[1] = txSize;
	txBuffer[2] = 0xFF;
	txBuffer[3] = 0xFF;
	memcpy(txBuffer + 4, txData, txSize);

	HAL_UART_Transmit(huart, txBuffer, sizeof (txBuffer), 10);
}

/** Send through the crane ID as an okay message when the crane is stationary
 *  @param huart pointer to uart handle
 *  @param craneID id of the crane
 */
void zigbee_send_okay(UART_HandleTypeDef *huart, uint8_t craneID) {

	//Get number of digits in id
	uint8_t idSize = log10(craneID) + 1;

	//populate char arrays with id
	char idArr[idSize + 2];
	memset(idArr, 0, sizeof (idArr));
	sprintf(idArr, "i%d ", craneID);

	char kArr[3];
	memset(kArr, 0, sizeof (kArr));
	sprintf(kArr, "k\r\n");

	//integrate char arrays and other commands for zigbee communication
	uint8_t dataLength = sizeof (idArr) + sizeof (kArr);
	uint8_t data[4 + dataLength];
	data[0] = 0xFD;
	data[1] = dataLength;
	data[2] = 0xFF;
	data[3] = 0xFF;
	memcpy(data + 4, idArr, sizeof (idArr));
	memcpy(data + 4 + sizeof (idArr), kArr, sizeof (kArr));

	HAL_UART_Transmit(huart, data, sizeof (data), 10);	//send data
}
