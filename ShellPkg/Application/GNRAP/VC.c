/** @file
  This is a test application that demonstrates how to use the C-style entry point
  for a shell application.

  Copyright (c) 1996 - 2024, Meritech Corporation. All rights reserved.<BR>
  
**/
#include "VC.h"
#include "MF.h"

/*******************************************************************************
 *	Date		Version		Comment
 *	24/10/09	V0.1.0		Initial Version
 ******************************************************************************/
#define	VERSION_MAJOR						        0			// Major version
#define	VERSION_MINOR						        1			// Minor version 1
#define	VERSION_BUILD						        0			// Build version 

#define MAX_TRY_CNT							        5			// Maximum retry count for All command
#define	SIZE_ARGUMENT_MAX				        16		// Maximum command string count

#define MAX_SLOT_VALUE                  8     // MRDIMM Slot Number 

//--- Define VDD, VPP MIN/MAX voltages
#define	MIN_VDD								          4250	// Min. VDD volt = 4.25V
#define	MAX_VDD								          15000	// Max. VDD volt = 15.00V
#define MIN_P3_3V							          3000	// Min. P3.3 volt = 3.0V
#define MAX_P3_3V							          3600	// Max. P3.3 volt = 3.6V


CHAR16* StrVolCh[5] = { L"P12V-CD", L"P12V-EF", L"P12V-IJ", L"P12V-KL", L"P3.3V" };

EFI_FILE_PROTOCOL* gRoot = NULL;
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* gSimpleFileSystem = NULL;

/*Function declaration*/
void
PrintHelpMsg(
  void
);

void ToUpperCase(
  CHAR16*,
  CHAR16*
);

EFI_STATUS
InitFileHandle(
  void
);

EFI_STATUS
GetIDFunc(
  void
);

/**
  UEFI application entry point which has an interface similar to a
  standard C main function.

  The ShellCEntryLib library instance wrappers the actual UEFI application
  entry point and calls this ShellAppMain function.

  @param[in] Argc     The number of items in Argv.
  @param[in] Argv     Array of pointers to strings.

  @retval  0               The application exited normally.
  @retval  Other           An error occurred.

**/
INTN
EFIAPI
ShellAppMain (
  IN UINTN   Argc,
  IN CHAR16  **Argv
  )
{
  EFI_STATUS  Status = EFI_INVALID_PARAMETER;
  CHAR16 OpCmd[SIZE_ARGUMENT_MAX];
  CHAR16 Date[12];
  
  UnicodeSPrintAsciiFormat(
    &Date[0],
    sizeof(Date),
    __DATE__
    );

  Print(L"Voltage Control Program for PCT3.0 GNRAP MRDIMM V%d.%d.%d %s\n",
      VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, Date);
    
  if (Argc == 1) {
    PrintHelpMsg();
    return Status;
  }
    
  Status = InitSerialPort();
  if (EFI_ERROR(Status)) {
    Print(L"  Failed to configure Serial Port %r \n", Status);
  }
  
  SetMem16(OpCmd, SIZE_ARGUMENT_MAX, 0);
  ToUpperCase(Argv[1], OpCmd);

  if(Argc == 2) {
    if (!StrCmp(OpCmd, L"-H")) {
        PrintHelpMsg();
    }
    else if (!StrCmp(OpCmd, L"-CC")) {  // Check Connection
      Status = CheckConnect();
      if (!EFI_ERROR(Status)) {
        Print(L"  Connection Ok!\n");
      }
      else {
        Print(L"  Connection Error!\n");
      }
    }
    else if (!StrCmp(OpCmd, L"-GR")) {  // Get Firwmare Version
      UINT8 V1, V2, V3;
      Status = GetFWVersion(&V1, &V2, &V3);
      if (!EFI_ERROR(Status)) {
        Print(L"  Firmware version : [%d.%d.%d] OK\n", V1, V2, V3);
      }
      else {
        Print(L"  [ERROR] Get Firmware version\n");
      }
    }
    else if (!StrCmp(OpCmd, L"-ID")) {  //Get ID 
      GetIDFunc();
    }
  }
  else if (Argc == 3) {
    if (!StrCmp(OpCmd, L"-SL")) { // Set LED Status
      Status = SetLEDStatus(
        Argv[2]
      );
      if (EFI_ERROR(Status)) {
        Print(L"   [ERROR] Set LED Status %s", Argv[2]);
      }
      else {
        Print(L"   Set LED Status %s Ok", Argv[2]);
      }
    }

    if (!StrCmp(OpCmd, L"-GV")) { // Get Output voltage
      UINTN Channel;
      UINT16 Vol;
      Channel = StrDecimalToUintn(Argv[2]);
      if (Channel == 0 || Channel > 5) {
        Print(L"  [ERROR] Get V-OUT Channel number\n");
        return EFI_UNSUPPORTED;
      }

      Status = GetVRVoltage(0x52, Channel, &Vol);
      if (!EFI_ERROR(Status)) {
        Print(L"  Get %s voltage: %d (mV)\n", StrVolCh[Channel - 1], Vol);
      }
      else {
        Print(L"  [ERROR] Get %s voltage Error\n", StrVolCh[Channel - 1]);
      }
    }

    if (!StrCmp(OpCmd, L"-GB")) { // Get Boot voltage
      UINTN Channel;
      UINT16 Vol;
      Channel = StrDecimalToUintn(Argv[2]);
      if (Channel == 0 || Channel > 5) {
        Print(L"  [ERROR] Get BOOT-V Channel number\n");
        return EFI_UNSUPPORTED;
      }

      Status = GetVRVoltage(0x54, Channel, &Vol);
      if (!EFI_ERROR(Status)) {
        Print(L"  Get %s BOOT voltage: %d (mV)\n", StrVolCh[Channel - 1], Vol);
      }
      else {
        Print(L"  [ERROR] Get %s BOOT voltage Error\n", StrVolCh[Channel - 1]);
      }
    }

    if (!StrCmp(OpCmd, L"-GC")) { // Get Memory Slot Count
      UINT32 Count;
      UINTN Slot;
      UINT8 Masked = 0x00;
      Slot = StrDecimalToUintn(Argv[2]);

      if (Slot > 8) {
        Print(L"  [ERROR] Invalid socket number : %d\n", Slot);
      }

      if (Slot != 0) Masked |= ~(0x1 << (((UINT8)Slot) - 1));
      
      for (UINT8 i = 1; i <= MAX_SLOT_VALUE; i++) {
        if (!(Masked & (0x01<<(i-1)))) {
          Status = GetSlotCount(
            Slot,
            &Count
          );
          if (!EFI_ERROR(Status)) {
            Print(L"  Get Memory Socket %d Count : %d OK\n", Slot, Count);
          }
          else {
            Print(L"  [ERROR] Get Memory Socket %d Count\n", Slot);
          }
        }
      }
    }

    if (!StrCmp(OpCmd, L"-SC")) { // Set Memory Slot Count Action
      Status = SetSlotCountAct(Argv[2]);
      if (!EFI_ERROR(Status)) {
        Print(L"  Set Memory Socket Count Action %s OK\n", Argv[2]);
      }else {
        Print(L"  [ERROR] Set Memory Socket Count Action\n");
      }
    }

    if (!StrCmp(OpCmd, L"-GF")) { // Get Fan RPM
      UINT16 FanRPM = 0;
      FanRPM = GetFanRPM(
        StrDecimalToUintn(Argv[2])
      );
      if (FanRPM == 0) Print(L"  [ERROR] Get FAN RPM\n");
      else Print(L"FAN RPM : RPM = % d\n", FanRPM);
    }

    if (!StrCmp(OpCmd, L"-P80")) { // Set Port 80
      SetP80(
        StrHexToUintn(Argv[2])
      );
    }
   
  }
  else if (Argc == 4) {    
    if (!StrCmp(OpCmd, L"-SV")) { // Set Output Voltage
      UINTN Channel;
      UINTN Voltage;

      Channel = StrDecimalToUintn(Argv[2]);
      Voltage = StrDecimalToUintn(Argv[3]);

      if (Channel >= 1 && Channel <= 4) {
        if (Voltage < MIN_VDD || Voltage > MAX_VDD) {
          Print(L"  [ERROR] Set V-OUT invalid VDD voltage : %d (mV)\n", Voltage);
          return Status;
        }
      }
      else if (Channel == 5) {
        if (Voltage < MIN_P3_3V || Voltage > MAX_P3_3V) {
          Print(L"  [ERROR] Set V-OUT invalid P3.3V voltage : %d (mV)\n", Voltage);
          return Status;
        }
      }
      else {
        Print(L"  [ERROR] Set V-OUT invalid voltage channel : %d\n", Channel);
        return Status;
      }

      Status = SetVRVoltage(
        0x61,
        Channel,
        (UINT16)Voltage
      );

      if (!EFI_ERROR(Status)) {
        Print(L"  Set %s voltage : %d (mV) OK\n", StrVolCh[Channel - 1],  Voltage);
      }
      else {
        Print(L"  [ERROR] Set %s voltage : % d(mV)\n", StrVolCh[Channel - 1], Voltage);
      }
    }

    if (!StrCmp(OpCmd, L"-BV")) { // Set Boot Voltage
      UINTN Channel;
      UINTN Voltage;

      Channel = StrDecimalToUintn(Argv[2]);
      Voltage = StrDecimalToUintn(Argv[3]);
      if (Channel >= 1 && Channel <= 4) {
        if (Voltage < MIN_VDD || Voltage > MAX_VDD) {
          Print(L"  [ERROR] Set BOOT-V invalid VDD voltage : %d (mV)\n", Voltage);
          return Status;
        }
      }
      else if (Channel == 5) {
        if (Voltage < MIN_P3_3V || Voltage > MAX_P3_3V) {
          Print(L"  [ERROR] Set BOOT-V invalid P3.3V voltage : %d (mV)\n", Voltage);
          return Status;
        }
      }
      else {
        Print(L"  [ERROR] Set BOOT-V invalid voltage channel : %d\n", Channel);
        return Status;
      }

      Status = SetVRVoltage(
        0x63,
        Channel,
        (UINT16)Voltage
      );

      if (!EFI_ERROR(Status)) {
        Print(L"  Set %s BOOT voltage : %d (mV) OK\n", StrVolCh[Channel - 1], Voltage);
      }
      else {
        Print(L"  [ERROR] Set %s BOOT voltage : % d(mV)\n", StrVolCh[Channel - 1], Voltage);
      }
    }

    if (!StrCmp(OpCmd, L"-FS")) { // Set Fan Speed
      Status = SetFanSpeed(
        StrHexToUintn(Argv[2]),
        StrHexToUintn(Argv[3])
      );
      if (EFI_ERROR(Status)) {
        Print(L"   Set FAN %d Speed %d OK\n",
          StrHexToUintn(Argv[2]),
          StrHexToUintn(Argv[3])
        );
      }
      else {
        Print(L"  [ERROR] Set FAN %d Spped %d\n",
          StrHexToUintn(Argv[2]),
          StrHexToUintn(Argv[3])
        );
      }
    }  
 
  }
  else {
    PrintHelpMsg();
    return EFI_INVALID_PARAMETER;
  }

  if (EFI_ERROR(Status)) {
    Print(L"  [ERROR] %s is not valid option.\n", OpCmd);
  }

  return Status;
}

/* Function Implement */
void
PrintHelpMsg(
  void
)
{
  Print(L"Copyright (c) 1996 - 2024, Meritech Corporation. All rights reserved \n");
  Print(L"  Usage : VC [command] [ch] [mVolt]\n");
  Print(L"  Examples:\n");
  Print(L"    VC -CC                     (Check Connection)\n");
  Print(L"    VC -GR                     (Get Firmware Version)\n");
  Print(L"    VC -GF [FAN_NUM]           (Get CPU FAN RPM)\n");
  Print(L"    VC -ID                     (Execute LED Off, Create VOLTDEV.TXT & SPCLED.TXT)\n");
  Print(L"    VC -GV [V-CH]              (Get output Voltage)\n");
  Print(L"    VC -GB [V-CH]              (Get Boot Voltage)\n");
  Print(L"    VC -SL [LED-STAT]          (Set LED Control)\n");
  Print(L"    VC -GC [M-SLOT]            (Get Memory Slot Count)\n");
  Print(L"    VC -SC [M-ACTION]          (Set Memory Slot Count Action)\n");
  Print(L"    VC -SV [V-CH] [VOLTAGE]    (Set output Voltage)\n");
  Print(L"    VC -BV [V-CH] [VOLTAGE]    (Set Boot Voltage)\n");
  Print(L"    VC -FS [FAN] [FAN_SPEED]   (Set FAN Speed Control)\n");
  Print(L"  V-CH : 1 = P12V-CD, 2 = P12V-EF, 3 = P12V-IJ, 4 = P12V-KL, 5 = P3.3V\n");
  Print(L"  Voltage range : %d mv <= P12V <= %d mV\n", MIN_VDD, MAX_VDD);
  Print(L"                  %d mV <= 3.3V <= %d mV\n", MIN_P3_3V, MAX_P3_3V);
  Print(L"  LED Control : P = Pass, F = Fail, B = Blank, X = No change\n");
  Print(L"  Memory Slot Count Action : + = Increase count, X = No Action, R = Reset Count\n");
  Print(L"  FAN_NUM : 1=CPU FAN 1/2, 2=CPU FAN 3/4, 3=SYS_FAN 1, 4=SYS_FAN 2\n");
  Print(L"  FAN : 1 = CPU FAN1, FAN2, 2 = CPU FAN 3, FAN4, 3 = CPU FAN1 ~ FAN4\n");
  Print(L"  FAN_SPEED : 3 ~ 10\n");
}

void
ToUpperCase(
  CHAR16* src,
  CHAR16* dest
)
{
  if (src == NULL || dest == NULL)
    return;

  while (*src) {
    *dest = CharToUpper(*src);
    src++;
    dest++;
  }
}

EFI_STATUS
InitFileHandle(
  void
)
{
  EFI_STATUS Status = EFI_UNSUPPORTED;

  Status = gBS->LocateProtocol(
    &gEfiSimpleFileSystemProtocolGuid,
    NULL,
    (VOID**)&gSimpleFileSystem
  );

  if (EFI_ERROR(Status)) {
    Print(L"  Failed to Open File System\n");
    return Status;
  }

  Status = gSimpleFileSystem->OpenVolume(
    gSimpleFileSystem,
    &gRoot
  );

  if (EFI_ERROR(Status)) {
    Print(L"  Failed to Open Root\n");
    return Status;
  }

  return Status;
}

EFI_STATUS
GetIDFunc(
  void
)
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  EFI_FILE_PROTOCOL* VoltFile;
  EFI_FILE_PROTOCOL* SpcFile;

  CHAR16 VerStr[] = L"VC S/W VERSION=%d.%d.%d\nVC F/W VERSION=%d.%d.%d\n";
  CHAR16 SlotStr[] = L"Slot%d B\n";
  VA_LIST Marker;
  UINTN bufSize;
  UINT8 V1, V2, V3;

  Status = InitFileHandle();
  if (EFI_ERROR(Status)) {
    Print(L"  Failed to Get File Handle %r\n", Status);
    return Status;
  }

  gRoot->Open(
    gRoot,
    &VoltFile,
    L"VOLTDEV.TXT",
    EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
    0
  );

  Status = GetFWVersion(&V1, &V2, &V3);

  VA_START(Marker, VerStr);
  bufSize = SPrintLength(VerStr, Marker);
  VA_END(Marker);

  Print(L"STR Buffer Size %d\n", bufSize);  
  UnicodeSPrint(
    VerStr,
    bufSize,
    L"VC S/W VERSION=%d.%d.%d\n VC F/W VERSION=%d.%d.%d\n",
    VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD,
    V1, V2, V3
  );

  Status = VoltFile->Write(
    VoltFile,
    &bufSize,
    VerStr
  );

  if (EFI_ERROR(Status)) {
    Print(L"  Failed to write VOLTDEV.txt\n");
    return Status;
  }
  Status = VoltFile->Close(VoltFile);
  
  gRoot->Open(
    gRoot,
    &SpcFile,
    L"SPCLED.TXT",
    EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
    0
  );

  SetLEDStatus(L"BBBBBBBB");

  VA_START(Marker, SlotStr);
  bufSize = SPrintLength(SlotStr, Marker);
  VA_END(Marker);
  Print(L"SlotStr Buffer Size %d\n", bufSize);
  
  for (UINT8 i = 1; i < 9; i++) {    
    UnicodeSPrint(
      SlotStr,
      bufSize,
      L"Slot%d B\n",
      i
    );    

    Status = SpcFile->Write(
      SpcFile,
      &bufSize,
      SlotStr
    );

    if (EFI_ERROR(Status)) {
      Print(L"  Failed to write Slot%d SPCLED.txt\n");
      return Status;
    }
  }

  Status = SpcFile->Close(SpcFile);
  Print(L"  Set CPX_VC -SL BBBBBBBB and Create VOLTDEV.TXT & SPCLED.TXT Ok!\n\n");

  return Status;
}
