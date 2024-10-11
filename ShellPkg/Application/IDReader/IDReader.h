#ifndef ID_READER_H_
#define ID_READER_H_

/*******************************************************************************
 *	Date		  Version		Comment
 *	24/10/11	V0.1.0		Initial Version
 ******************************************************************************/
#define VERSION_MAJOR						0
#define VERSION_MINOR						0
#define VERSION_BUILD					  1

#define MAX_TRY_CNT							5

#define UARTA							      0x3F8
#define UARTB							      0x2F8

#define SIZE_CMD_ARGS						16
#define SIZE_CMD_PACKET					8
#define SIZE_MAX_ID_SN					16
#define STX								    	0x02
#define ETX									    0x03

#define CMD_CHECK_CONNECTION		0x41
#define CMD_GET_BOARD_ID				0x72
#define CMD_SET_BOARD_ID				0x73
#define CMD_GET_BOARD_SN				0x74
#define CMD_SET_BOARD_SN				0x75

#define BOARD_SN							  1
#define BOARD_ID							  2







#endif
