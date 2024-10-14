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

//--- Define VDD, VPP MIN/MAX voltages
#define	MIN_VDD								          4250	// Min. VDD volt = 4.25V
#define	MAX_VDD								          15000	// Max. VDD volt = 15.00V
#define MIN_P3_3V							          3000	// Min. P3.3 volt = 3.0V
#define MAX_P3_3V							          3600	// Max. P3.3 volt = 3.6V

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

void
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
  UINTN  Index;
  CHAR16 OpCmd[SIZE_ARGUMENT_MAX] = { 0, };

  Print(L"Voltage Control Program for PCT3.0 GNRAP MRDIMM V%d.%d.%d %s\n",
      VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, __DATE__);

  ///* change to BIOS ? TBD
  Status = InitSerialPort();
  if (EFI_ERROR(Status)) {
    Print(L"  Failed to configure Serial Port %r \n", Status);
  }

  if (Argc == 1) {
    PrintHelpMsg();
    return Status;
  }

  // Test for shell UI debug use
  for (Index = 1; Index < Argc; Index++) {
        Print(L"Argv[%d]: \"%s\"\n", Index, Argv[Index]);
  }

  ToUpperCase(Argv[1], OpCmd);

  if(Argc == 2) {
    if (!StrCmp(OpCmd, L"-H")) {
        PrintHelpMsg();
    }
    else if (!StrCmp(OpCmd, L"-CC")) {  //Check Connect
      Status = CheckConnect();
      if (!EFI_ERROR(Status)) {
        Print(L"  Connection Ok!\n");
      }
      else {
        Print(L"  Connection Error!\n");
      }
    }
    else if (!StrCmp(OpCmd, L"-GR")) {  //Get Firwmare Version
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

    }

    if (!StrCmp(OpCmd, L"-GV")) { // Get Output voltage

    }

    if (!StrCmp(OpCmd, L"-GB")) { // Get Boot voltage

    }

    if (!StrCmp(OpCmd, L"-GC")) { // Get Memory Slot Count

    }

    if (!StrCmp(OpCmd, L"-SC")) { // Set Memory Slot Count Action

    }

    if (!StrCmp(OpCmd, L"-GF")) { // Get Fan RPM

    }

    if (!StrCmp(OpCmd, L"-P80")) { // Set Port 80
      SetP80(StrHexToUintn(Argv[2]));
    }
   
  }
  else if (Argc == 4) {
    
    if (!StrCmp(OpCmd, L"-SV")) { // Set Output Voltage

    }

    if (!StrCmp(OpCmd, L"-BV")) { // Set Boot Voltage

    }

    if (!StrCmp(OpCmd, L"-FS")) { // Set Fan Speed

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

void
GetIDFunc(
  void
)
{
  EFI_STATUS Status;
  EFI_FILE_PROTOCOL* VoltFile;
  EFI_FILE_PROTOCOL* SpcFile;

  CHAR8 str[50] = { 0 };
  CHAR8 SlotStr[10] = { 0 };
  UINTN bufSize=sizeof(str);
  UINT8 V1, V2, V3;

  Status = InitFileHandle();
  if (EFI_ERROR(Status)) {
    Print(L"  Failed to Get File Handle %r \n", Status);
  }

  gRoot->Open(
    gRoot,
    &VoltFile,
    L"VOLTDEV.TXT",
    EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
    0
  );

  Status = GetFWVersion(&V1, &V2, &V3);
  AsciiSPrint(
    str,
    bufSize,
    "VC S/W VERSION=%d.%d.%d\n VC F/W VERSION=%d.%d.%d\n",
    VERSION_MAJOR,
    VERSION_MINOR,
    VERSION_BUILD,
    V1,
    V2,
    V3
  );

  Status = VoltFile->Write(
    VoltFile,
    &bufSize,
    str
  );

  if (EFI_ERROR(Status)) {
    Print(L"  Failed to write VOLTDEV\n");
  }


  Status = VoltFile->Close(VoltFile);
  SetMem(str, bufSize, 0);

  gRoot->Open(
    gRoot,
    &SpcFile,
    L"SPCLED.TXT",
    EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
    0
  );

  for (UINT8 i = 1; i < 9; i++) {
    bufSize = sizeof(SlotStr);
    AsciiSPrint(
      SlotStr,
      bufSize,
      "Slot%d B\n",
      i
    );

    Status = SpcFile->Write(
      SpcFile,
      &bufSize,
      SlotStr
    );
  }

  Status = SpcFile->Close(SpcFile);

  Print(L"  Set CPX_VC -SL BBBBBBBB and Create VOLTDEV.TXT & SPCLED.TXT Ok!\n\n");

}
