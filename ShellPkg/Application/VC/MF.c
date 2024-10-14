/** @file
  This is a test application that demonstrates how to use the C-style entry point
  for a shell application.

  Copyright (c) 1996 - 2024, Meritech Corporation. All rights reserved.<BR>

**/


#include <Library\SerialPortLib.h>
#include "VC.h"

#define	RECV_BUFF_SIZE						      1024
#define	SIZE_CMD_PACKET						      8
#define STX									            0x02
#define ETX									            0x03

// --- Command Definition
#define CMD_CHECK_CONNECTION			      0x41
#define CMD_PORT80_DATA						      0x42
#define CMD_GET_FW_VERSION				      0x51
#define CMD_GET_CUR_VOLTAGE				      0x52
#define CMD_GET_SLEW_RATE					      0x53
#define CMD_GET_BOOT_VOLTAGE			      0x54
#define CMD_GET_MEM_COUNT					      0x55
#define CMD_GET_FAN_RPM						      0x56
#define CMD_SET_OUT_VOLTAGE				      0x61
#define CMD_SET_SLEW_RATE					      0x62
#define CMD_SET_BOOT_VOLTAGE			      0x63
#define CMD_SET_LED							        0x64
#define CMD_SET_MEM_COUNT_ACTION	      0x65
#define CMD_SET_VOLTAGE_TUNE			      0x66
#define CMD_SET_ADC_TUNE					      0x67
#define CMD_SET_PWM_PULSE_WIDTH		      0x68

UINT8 gTxPkt[SIZE_CMD_PACKET];
UINT8 gRxPkt[SIZE_CMD_PACKET];

EFI_STATUS
Init_SerialPort(
  void
)
{

  return SerialPortInitialize();
}

void Init_TxPkt()
{
  SetMem(gTxPkt, SIZE_CMD_PACKET, 0);
  gTxPkt[0] = STX;
  gTxPkt[7] = ETX;
}

void Init_RxPkt()
{
  SetMem(gRxPkt, SIZE_CMD_PACKET, 0);
}
