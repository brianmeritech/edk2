/** @file
  This is a test application that demonstrates how to use the C-style entry point
  for a shell application.

  Copyright (c) 1996 - 2024, Meritech Corporation. All rights reserved.<BR>

**/

#include "VC.h"

#include <Library/SerialPortLib.h>

#define RETRY                           5

#define	SIZE_CMD_PACKET						      8
#define STX									            0x02
#define ETX									            0x03

#define STX_INDX                        0
#define CMD_INDX                        1
#define DAT1_INDX                       2
#define DAT2_INDX                       3
#define DAT3_INDX                       4
#define DAT4_INDX                       5
#define DAT5_INDX                       6
#define ETX_INDX                        7

#define MAX_LED_NUMBER                  8
#define MAX_LED_TABLE                   2

#define MAX_SLOT_NUMBER                 8
#define MAX_SLOT_TABLE                  2

#define MAX_BOARD_INFO_TABLE            4

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
#define GET_CUR_VOLTAGE_DONE            0x92
#define GET_SLOT_COUNT_DONE             0x95
#define GET_FAN_RPM_DONE                0x96
#define SET_LED_STATUS_DONE             0xA4
#define SET_FAN_PWM_DOEN                0xA8

#define CMD_GET_IP_INFO						      0x71
#define CMD_SET_IP_INFO						      0x78

#define GET_IP_INFO_DONE                0xB1
#define SET_IP_INFO_DONE                0xB8

#define TYPE_IP_ADDRESS						      1
#define TYPE_SUBNET_MASK					      2
#define TYPE_GATEWAY						        3

#define CMD_GET_BOARD_ID					      0x72
#define CMD_SET_BOARD_ID					      0x73
#define CMD_GET_BOARD_SN					      0x74
#define CMD_SET_BOARD_SN					      0x75

#define BOARD_SN							          1
#define BOARD_ID							          2

UINT8 gTxPkt[SIZE_CMD_PACKET];
UINT8 gRxPkt[SIZE_CMD_PACKET];
UINTN NumBytes = 0;

EFI_STATUS
ReadUartData(
  void
  )
{
  UINTN ReadSize = 0;;
  for (UINT8 i = 0; i < RETRY; i++) {
    if (SerialPortPoll()) {
      ReadSize = SerialPortRead(gRxPkt, SIZE_CMD_PACKET);
      if (ReadSize == SIZE_CMD_PACKET)
        return EFI_SUCCESS;
      else
        return EFI_DEVICE_ERROR;
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
  NumBytes = SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
  if (NumBytes == SIZE_CMD_PACKET) {
    if (!EFI_ERROR(ReadUartData())) {
      if (gRxPkt[CMD_INDX] == CHECK_CONNECTION_DONE)
        return EFI_SUCCESS;
    }
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
  gTxPkt[DAT1_INDX] = (UINT8)Dat & 0x0F;         //Low Byte
  gTxPkt[DAT2_INDX] = ((UINT8)Dat & 0xF0) >> 4;  //High Byte

  NumBytes = SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
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
  gTxPkt[DAT1_INDX] = 1; //Main MCU FW
  NumBytes = SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
  if (NumBytes == SIZE_CMD_PACKET) {

    if (!EFI_ERROR(ReadUartData())) {
      if (gRxPkt[CMD_INDX] == GET_FW_VERSION_DONE) {
        *pV1 = gRxPkt[DAT2_INDX];
        *pV2 = gRxPkt[DAT3_INDX];
        *pV3 = gRxPkt[DAT4_INDX];
        return EFI_SUCCESS;
      }
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
  gTxPkt[DAT1_INDX] = (UINT8)FAN;
  NumBytes = SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
  if (NumBytes == SIZE_CMD_PACKET) {
    if (!EFI_ERROR(ReadUartData())) {
      if (gRxPkt[CMD_INDX] == GET_FAN_RPM_DONE) {
        tRPM = gRxPkt[3] | ((gRxPkt[4] << 8) & 0xFF00);
        return tRPM;
      }
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
    gTxPkt[DAT1_INDX] = i;                     // 0: LED1~4 ; 1: LED5~8
    gTxPkt[DAT2_INDX] = LedTyp[i * 4];
    gTxPkt[DAT3_INDX] = LedTyp[i * 4 + 1];
    gTxPkt[DAT4_INDX] = LedTyp[i * 4 + 2];
    gTxPkt[DAT5_INDX] = LedTyp[i * 4 + 3];
    NumBytes = SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
    if (NumBytes == SIZE_CMD_PACKET) {
      if (!EFI_ERROR(ReadUartData())) {
        if (gRxPkt[CMD_INDX] == SET_LED_STATUS_DONE) {
          gBS->Stall(150);
          Status = EFI_SUCCESS;
        }
        else {
          Print(L"  [ERROR] Set LED Communication ERROR(%d)\n", i);
          Status = EFI_UNSUPPORTED;
        }
      }
    }
  }

  return Status;
}

EFI_STATUS
SetFanSpeed(
  UINTN Channel,
  UINTN Speed
  )
{
  EFI_STATUS Status = EFI_UNSUPPORTED;

  if (Channel == 0 || Channel > 3) {
    Print(L"  [ERROR] Set F-SPD invalid FAN Channel : %d\n", Channel);
    return Status;
  }

  if (Speed == 0 || Speed > 10) {
    Print(L"  [ERROR] Set F-SPD invalid FAN Speed : %d\n", Speed);
    return Status;
  }

  InitTxPkt();
  gTxPkt[CMD_INDX] = CMD_SET_PWM_PULSE_WIDTH;
  gTxPkt[DAT1_INDX] = (UINT8)Channel;
  gTxPkt[DAT2_INDX] = (UINT8)Speed;

  NumBytes = SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
  if (NumBytes == SIZE_CMD_PACKET) {
    if (!EFI_ERROR(ReadUartData())) {
      if (gRxPkt[CMD_INDX] == SET_FAN_PWM_DOEN) {
        Status = EFI_SUCCESS;
      }
    }
  }

  return Status;
}


EFI_STATUS
GetSlotCount(
  UINTN Slot,
  UINT32* pCount
  )
{
  EFI_STATUS Status = EFI_UNSUPPORTED;

  InitTxPkt();
  InitRxPkt();
  gTxPkt[CMD_INDX] = CMD_GET_MEM_COUNT;
  gTxPkt[DAT1_INDX] = (UINT8)Slot;

  NumBytes = SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
  if (NumBytes == SIZE_CMD_PACKET) {
    if (!EFI_ERROR(ReadUartData())) {
      if (gRxPkt[CMD_INDX] == GET_SLOT_COUNT_DONE) {
        *pCount = gRxPkt[DAT2_INDX] | (gRxPkt[DAT3_INDX] << 8) | (gRxPkt[DAT4_INDX] << 16) | (gRxPkt[DAT5_INDX] << 24);
        Status = EFI_SUCCESS;
      }
    }
  }

  return Status;
}

EFI_STATUS
SetSlotCountAct(
  CHAR16* SlotAct
  )
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  UINT8 ActTyp[MAX_SLOT_NUMBER];
  
  for (UINT8 i = 0; i < MAX_SLOT_NUMBER; i++) {
    switch (CharToUpper(SlotAct[i])) {
    case 0x0058:        //X NO Action
      ActTyp[i] = 0;
      break;
    case 0x002B:        //+ Increase
      ActTyp[i] = 1;
      break;
    case 0x0052:        //R Reset
      ActTyp[i] = 2;
      break;
    }
  }

  for (UINT8 i = 0; i < MAX_SLOT_TABLE; i++) {
    InitTxPkt();
    gTxPkt[CMD_INDX] = CMD_SET_MEM_COUNT_ACTION;
    gTxPkt[DAT1_INDX] = i;                     // 0: SLOT1~4 ; 1: SLOT5~8
    gTxPkt[DAT2_INDX] = ActTyp[i * 4];
    gTxPkt[DAT3_INDX] = ActTyp[i * 4 + 1];
    gTxPkt[DAT4_INDX] = ActTyp[i * 4 + 2];
    gTxPkt[DAT5_INDX] = ActTyp[i * 4 + 3];

    NumBytes = SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
    if (NumBytes != SIZE_CMD_PACKET) Status = EFI_DEVICE_ERROR;
    else Status = EFI_SUCCESS;
  
/*
    if (!EFI_ERROR(ReadUartData())) {
      if (gRxPkt[CMD_INDX] == CMD_SET_MEM_COUNT_ACTION) {
        gBS->Stall(150);
        Status = EFI_SUCCESS;
      }
      else {
        Print(L"  [ERROR] Memory Slot Action Communication ERROR : %d\n", i);
        Status = EFI_UNSUPPORTED;
      }
    }
    */
  }

  return Status;
}

EFI_STATUS
GetVRVoltage(
  UINT8 bCmd,
  UINTN Channel,
  UINT16* Vol
  )
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  
  InitTxPkt();
  InitRxPkt();
  gTxPkt[CMD_INDX] = bCmd;
  gTxPkt[DAT1_INDX] = (UINT8)Channel;
  NumBytes = SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
  if (NumBytes == SIZE_CMD_PACKET) {
    if (!EFI_ERROR(ReadUartData())) {
      if (gRxPkt[CMD_INDX] == bCmd + 0x40) {
        *Vol = gRxPkt[3] | ((gRxPkt[4] << 8) & 0xFF00);
        Status = EFI_SUCCESS;
      }
    }
  }

  return Status;
}

EFI_STATUS
SetVRVoltage(
  UINT8 bCmd,
  UINTN Channel,
  UINT16 Vol
  )
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  
  InitTxPkt();
  InitRxPkt();
  gTxPkt[CMD_INDX] = bCmd;
  gTxPkt[DAT1_INDX] = (UINT8)Channel;
  gTxPkt[DAT2_INDX] = (UINT8)(Vol & 0x00FF);
  gTxPkt[DAT3_INDX] = (UINT8)((Vol & 0xFF00) >> 8);
  NumBytes = SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
  if (NumBytes == SIZE_CMD_PACKET) {
    if (!EFI_ERROR(ReadUartData())) {
      if (gRxPkt[CMD_INDX] == bCmd + 0x40) {

        Status = EFI_SUCCESS;
      }
    }
  }

  return Status;
}

EFI_STATUS
GetIPAddr(
  IPv4_ADDRESS* Addr
)
{
  EFI_STATUS Status = EFI_NOT_READY;
  
  InitTxPkt();
  InitRxPkt();

  gTxPkt[CMD_INDX] = CMD_GET_IP_INFO;
  gTxPkt[DAT1_INDX] = TYPE_IP_ADDRESS;

  NumBytes = SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
  if (NumBytes == SIZE_CMD_PACKET) {
    if (!EFI_ERROR(ReadUartData())) {
      if (gRxPkt[CMD_INDX] == GET_IP_INFO_DONE) {
        Addr->Addr[0] = gRxPkt[DAT2_INDX];
        Addr->Addr[1] = gRxPkt[DAT3_INDX];
        Addr->Addr[2] = gRxPkt[DAT4_INDX];
        Addr->Addr[3] = gRxPkt[DAT5_INDX];
        Status = EFI_SUCCESS;
      }
    }
  }

  return Status;
}

EFI_STATUS
SaveIPAddr(
  IPv4_ADDRESS* Addr
)
{
  EFI_STATUS Status = EFI_INVALID_PARAMETER;
  InitTxPkt();
  InitRxPkt();

  gTxPkt[CMD_INDX] = CMD_SET_IP_INFO;
  gTxPkt[DAT1_INDX] = TYPE_IP_ADDRESS;
  gTxPkt[DAT2_INDX] = Addr->Addr[0];
  gTxPkt[DAT3_INDX] = Addr->Addr[1];
  gTxPkt[DAT4_INDX] = Addr->Addr[2];
  gTxPkt[DAT5_INDX] = Addr->Addr[3];
  NumBytes = SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
  if(NumBytes == SIZE_CMD_PACKET) {
    if (!EFI_ERROR(ReadUartData())) {
      if (gRxPkt[CMD_INDX] == SET_IP_INFO_DONE) {
        Status = EFI_SUCCESS;
      }
    }
  }
  return Status;
}

EFI_STATUS
SaveBoardInfo(
  UINT8 Cmd,
  CHAR8* InfoStr
)
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  
  InitTxPkt();
  InitRxPkt();
  
  gTxPkt[CMD_INDX] = Cmd;
  for (UINTN i = 0; i < MAX_BOARD_INFO_TABLE; i++) {    
    gTxPkt[DAT1_INDX] = (UINT8)i;    // Index of ID String
    gTxPkt[DAT2_INDX] = *(InfoStr+(i * MAX_BOARD_INFO_TABLE));
    gTxPkt[DAT3_INDX] = *(InfoStr+(i * MAX_BOARD_INFO_TABLE) + 1);
    gTxPkt[DAT4_INDX] = *(InfoStr+(i * MAX_BOARD_INFO_TABLE) + 2);
    gTxPkt[DAT5_INDX] = *(InfoStr+(i * MAX_BOARD_INFO_TABLE) + 3);
    NumBytes = SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
    if (NumBytes == SIZE_CMD_PACKET) {
      if (!EFI_ERROR(ReadUartData())) {
        if (gRxPkt[CMD_INDX] == Cmd + 0x40) {
          Status = EFI_SUCCESS;
        }
        else {
          Status = EFI_NOT_READY;
          if (Cmd == CMD_SET_BOARD_ID) {
            Print(L"  [ERROR] Write ID error.index = %d\n", i);
          }
          else if (Cmd == CMD_SET_BOARD_SN) {
            Print(L"  [ERROR] Write SN error.index = %d\n", i);
          }
          break;
        }
      }
      gBS->Stall(150);
    }
  }

  return Status;
}

EFI_STATUS
GetBoardInfo(
  UINT8 Cmd,
  CHAR8* InfoStr
)
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  
  InitTxPkt();
  InitRxPkt();

  gTxPkt[CMD_INDX] = Cmd;
  for (UINTN i = 0 ; i < MAX_BOARD_INFO_TABLE ; i++) {
    gTxPkt[DAT1_INDX] = (UINT8)i;
    NumBytes = SerialPortWrite(gTxPkt, SIZE_CMD_PACKET);
    if (NumBytes == SIZE_CMD_PACKET) {
      if (!EFI_ERROR(ReadUartData())) {
        if (gRxPkt[CMD_INDX] == Cmd + 0x40) {
          *(InfoStr + (i * MAX_BOARD_INFO_TABLE)) = gRxPkt[DAT2_INDX];
          *(InfoStr + (i * MAX_BOARD_INFO_TABLE) + 1) = gRxPkt[DAT3_INDX];
          *(InfoStr + (i * MAX_BOARD_INFO_TABLE) + 2) = gRxPkt[DAT4_INDX];
          *(InfoStr + (i * MAX_BOARD_INFO_TABLE) + 3) = gRxPkt[DAT5_INDX];
        }
      }
      else {
        Status = EFI_NOT_READY;
        if (Cmd == CMD_GET_BOARD_ID) {
          Print(L"  [ERROR] Read ID error.index = %d\n", i);
        }
        else if (Cmd == CMD_GET_BOARD_SN) {
          Print(L"  [ERROR] Read SN error.index = %d\n", i);
        }
        break;
      }
      gBS->Stall(150);
    }
  }

  return Status;
}
