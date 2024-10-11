#ifndef IP_READER_H_
#define IP_READER_H_

/*******************************************************************************
 *	Date		  Version		Comment
 *	24/10/11  V0.1.0		Initial Version 
 ******************************************************************************/
#define VERSION_MAJOR						0
#define VERSION_MINOR						1
#define VERSION_BUILD					  0

#define MAX_TRY_CNT							5


#define MAX_ARGUMENT_STRING			16

#define SIZE_CMD_ARGS						16
#define SIZE_CMD_PACKET					8
#define SIZE_MAX_ID_SN					16
#define STX									    0x02
#define ETX									    0x03

#define CMD_CHECK_CONNECTION		0x41
#define CMD_GET_IP_INFO					0x71
#define CMD_SET_IP_INFO					0x78

#define TYPE_IP_ADDRESS					1
#define TYPE_SUBNET_MASK				2
#define TYPE_GATEWAY						3






#endif
