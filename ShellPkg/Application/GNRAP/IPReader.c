/** @file
  This is a test application that demonstrates how to use the C-style entry point
  for a shell application.

  Copyright (c) 1996 - 2024, Meritech Corporation. All rights reserved.<BR>


**/
#include "IPReader.h"
#include "MF.h"

/*******************************************************************************
 *	Date		  Version		Comment
 *	24/10/11  V0.1.0		Initial Version
 ******************************************************************************/
#define VERSION_MAJOR						0
#define VERSION_MINOR						1
#define VERSION_BUILD					  0

#define MAX_ARGUMENT_STRING			16
#define SIZE_CMD_ARGS						16

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
ReadIPInfo(
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
  EFI_STATUS  Status = EFI_UNSUPPORTED;
  CHAR16 OpCmd[MAX_ARGUMENT_STRING] = { 0, };
  CHAR16 Date[12];

  UnicodeSPrintAsciiFormat(
    &Date[0],
    sizeof(Date),
    __DATE__
  );

  if (Argc == 1 || Argc > 3) {
    PrintHelpMsg();
    return Status;
  }

  Print(L"IPReader for PCT3.0 GNRAP MRDIMM V%d.%d.%d %s\n",
    VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, Date);

  ToUpperCase(Argv[1], OpCmd);

  Status = InitSerialPort();
  if (EFI_ERROR(Status)) {
    Print(L"  Failed to configure Serial Port %r \n", Status);
  }
    
  if (Argc == 2) {
    if (!StrCmp(OpCmd, L"-R")) {      //Read IP Info
      Status = ReadIPInfo();

      if (!EFI_ERROR(Status)) {
        Print(L"  ReadIp.TXT file create Ok!\n\n");
      }
    }
  }
  else {
    if (!StrCmp(OpCmd, L"-W")) {      //Write IP Info
      //TBD
    }
  }

  if (EFI_ERROR(Status)) {
    Print(L"  [ERROR] %s is not valid command.\n", OpCmd);
  }

  return Status;
}


void PrintHelpMsg(void)
{
  Print(L"Copyright (c) 1996 - 2024, Meritech Corporation. All rights reserved \n");
  Print(L"  usage : IPReader [-R/-W] [address]\n");
  Print(L"  examples:\n");
  Print(L"    IPReader -W address     (Write BOARD IP)\n");
  Print(L"    IPReader -R             (Read BOARD IP)\n");
}

void ToUpperCase(CHAR16* src, CHAR16* dest)
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
ReadIPInfo(
  void
)
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  EFI_FILE_PROTOCOL* IpFile;
  UINT8 IPAddr[4];

  Status = InitFileHandle();
  if (EFI_ERROR(Status)) return Status;

  gRoot->Open(
    gRoot,
    &IpFile,
    L"ReadIp.TXT",
    EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
    0
  );

  Status = GetIPAddr(IPAddr);
  if (EFI_ERROR(Status)) return Status;

  Print(L"  BOARD IP:%d.%d.%d.%d\n", IPAddr[0], IPAddr[1], IPAddr[2], IPAddr[3]);
  Print(L"  TPC IP:%d.%d.%d.250\n", IPAddr[0], IPAddr[1], IPAddr[2]);
  Print(L"  TPC PORT:30001\n");
  Print(L"  TEMP IP:%d.%d.%d.%d\n", IPAddr[0], IPAddr[1], IPAddr[2], IPAddr[3] + 5);
  Print(L"  TEMP PORT:30002\n");
  Print(L"  NETMASK:255.255.0.0\n");
  Print(L"  GATEWAY:%d.%d.1.1\n", IPAddr[0], IPAddr[1]);



  return Status;
}
