/** @file
  This is a test application that demonstrates how to use the C-style entry point
  for a shell application.

  Copyright (c) 1996 - 2024, Meritech Corporation. All rights reserved.<BR>

**/


#include <Library\SerialPortLib.h>
#include "VC.h"

#define RETRY                           5

#define	RECV_BUFF_SIZE						      1024
#define	SIZE_CMD_PACKET						      8
#define STX									            0x02
#define ETX									            0x03

#define STX_INDX                        0
#define CMD_INDX                        1
#define DT1_INDX                        2
#define DT2_INDX                        3
#define DT3_INDX                        4
#define DT5_INDX                        5
#define DT6_INDX                        6
#define ETX_INDX                        7

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

#define CHECK_CONNECTION_DONE           0x81
#define GET_FW_VERSION_DONE             0x91



UINT8 gTxPkt[SIZE_CMD_PACKET];
UINT8 gRxPkt[SIZE_CMD_PACKET];

EFI_STATUS
ReadUartData(
  void
)
{
  for (UINT8 i = 0; i < RETRY; i++) {
    if (SerialPortPoll()) {
      if (SerialPortRead(gRxPkt, SIZE_CMD_PACKET))
        return EFI_SUCCESS;
      else
        return EFI_NOT_FOUND;
    }
    gBS->Stall(150);
  }
  return EFI_NOT_FOUND;
}

void
InitTxPkt(
  void
)
{
  SetMem(gTxPkt, SIZE_CMD_PACKET, 0);
  gTxPkt[STX_INDX] = STX;
  gTxPkt[ETX_INDX] = ETX;
}

void
InitRxPkt(
  void
)
{
  SetMem(gRxPkt, SIZE_CMD_PACKET, 0);
}

EFI_STATUS
InitSerialPort(
  void
)
{
  return SerialPortInitialize();
}


EFI_STATUS
CheckConnect(
  void
)
{
  InitTxPkt();
  InitRxPkt();

  gTxPkt[CMD_INDX] = CMD_CHECK_CONNECTION;
  SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);

  if (!EFI_ERROR(ReadUartData())) {
    if (gRxPkt[CMD_INDX] == CHECK_CONNECTION_DONE)
      return EFI_SUCCESS;
  }
  return EFI_NOT_READY;  
}

void
SetP80(
  UINTN Dat
)
{
  Init_TxPkt();
  gTxPkt[CMD_INDX] = CMD_PORT80_DATA;
  gTxPkt[DT1_INDX] = (UINT8)Dat & 0x0F;         //Low Byte
  gTxPkt[DT2_INDX] = ((UINT8)Dat & 0xF0) >> 4;  //High Byte

  SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
}


