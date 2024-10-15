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
#define DT4_INDX                        5
#define DT5_INDX                        6
#define ETX_INDX                        7

#define MAX_LED_NUMBER                  8
#define MAX_LED_TABLE                   2

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
#define CMD_SET_LED_STATUS			        0x64
#define CMD_SET_MEM_COUNT_ACTION	      0x65
#define CMD_SET_VOLTAGE_TUNE			      0x66
#define CMD_SET_ADC_TUNE					      0x67
#define CMD_SET_PWM_PULSE_WIDTH		      0x68

#define CHECK_CONNECTION_DONE           0x81
#define GET_FW_VERSION_DONE             0x91
#define GET_FAN_RPM_DONE                0x96
#define SET_LED_STATUS_DONE             0xA4


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
  InitTxPkt();
  gTxPkt[CMD_INDX] = CMD_PORT80_DATA;
  gTxPkt[DT1_INDX] = (UINT8)Dat & 0x0F;         //Low Byte
  gTxPkt[DT2_INDX] = ((UINT8)Dat & 0xF0) >> 4;  //High Byte

  SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
}

EFI_STATUS
GetFWVersion(
  UINT8* pV1,
  UINT8* pV2,
  UINT8* pV3
)
{
  InitTxPkt();
  InitRxPkt();

  gTxPkt[CMD_INDX] = CMD_GET_FW_VERSION;
  gTxPkt[DT1_INDX] = 1; //Main MCU FW
  SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);

  if (!EFI_ERROR(ReadUartData())) {
    if (gRxPkt[CMD_INDX] == GET_FW_VERSION_DONE) {
      *pV1 = gRxPkt[DT2_INDX];
      *pV2 = gRxPkt[DT3_INDX];
      *pV3 = gRxPkt[DT4_INDX];
      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_READY;
}

UINT16
GetFanRPM(
  UINTN FAN
)
{
  UINT16 tRPM = 0;

  InitTxPkt();
  InitRxPkt();

  gTxPkt[CMD_INDX] = CMD_GET_FAN_RPM;
  gTxPkt[DT1_INDX] = (UINT8)FAN;
  SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);

  if (!EFI_ERROR(ReadUartData())) {
    if (gRxPkt[CMD_INDX] == GET_FAN_RPM_DONE) {
      tRPM = gRxPkt[3] | ((gRxPkt[4] << 8) & 0xFF00);
      return tRPM;
    }    
  }

  return 0;
}

EFI_STATUS
SetLEDStatus(
  CHAR16* LedStatus
)
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  UINT8 LedTyp[MAX_LED_NUMBER];

  for (UINT8 i = 0; i < MAX_LED_NUMBER; i++) {
    switch (CharToUpper(LedStatus[i])) {
    case 0x0042:  //B
      LedTyp[i] = 0;
      break;
    case 0x0050:  //P
      LedTyp[i] = 1;
      break;
    case 0x0046:    //F
      LedTyp[i] = 2;
      break;
    }
  }

  for (UINT8 i = 0; i < MAX_LED_TABLE; i++) {
    InitTxPkt();
    gTxPkt[CMD_INDX] = CMD_SET_LED_STATUS;
    gTxPkt[DT1_INDX] = i;                     // 0: LED1~4 ; 1: LED5~8
    gTxPkt[DT2_INDX] = LedTyp[i * 4];
    gTxPkt[DT3_INDX] = LedTyp[i * 4 + 1];
    gTxPkt[DT4_INDX] = LedTyp[i * 4 + 2];
    gTxPkt[DT5_INDX] = LedTyp[i * 4 + 3];
    SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);

    if (!EFI_ERROR(ReadUartData())) {
      if (gRxPkt[CMD_INDX] == SET_LED_STATUS_DONE) {
        gBS->Stall(150);        
      }
      else {
        Print(L"  [ERROR] Set LED Communication ERROR(%d)\n", i);
      }
    }
  }

  return Status;
}
