/*
**************************************************************************************************************
* @file     zigbee.c
* @author   Ryan Lederhose
* @date     07/02/2023
* @brief    Driver for zigbee communication
**************************************************************************************************************
*/

#ifndef INC_ZIGBEE_H_
#define INC_ZIGBEE_H_

#include "main.h"
#include "math.h"

/** Send the given positions, mass and ID of the crane through the zigbee modules
 *  @param huart pointer to uart handle
 *  @param positions struct holding (x, y) positions of crane
 *  @param mass raw adc value of crane load gauge
 *  @param craneID id of crane
 */
void zigbee_send_data(UART_HandleTypeDef *huart, coordinates_t positions, uint32_t mass, uint8_t craneID);

/** Send through a set data buffer through zigbee
 *  @param huart pointer to uart handlee
 *  @param txData pointer to data buffer
 *  @param txSize size of data buffer
 */
void zigbee_send_other_data(UART_HandleTypeDef *huart, uint8_t *txData, uint16_t txSize);

/** Send through the crane ID as an okay message when the crane is stationary
 *  @param huart pointer to uart handle
 *  @param craneID id of the crane
 */
void zigbee_send_okay(UART_HandleTypeDef *huart, uint8_t craneID);

#endif /* INC_ZIGBEE_H_ */
