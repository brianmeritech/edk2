/** @file
  This is a test application that demonstrates how to use the C-style entry point
  for a shell application.

  Copyright (c) 1996 - 2024, Meritech Corporation. All rights reserved.<BR>


**/
#include "VC.h"
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

CHAR16 Date[12];

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

  Status = InitSerialPort();
  if (EFI_ERROR(Status)) {
    Print(L"  Failed to configure Serial Port %r \n", Status);
  }

  ToUpperCase(Argv[1], OpCmd);
    
  if (Argc == 2) {
    if (!StrCmp(OpCmd, L"-R")) {      //Read IP Info
      Status = ReadIPInfo();

      if (!EFI_ERROR(Status)) {
        Print(L"  ReadIp.TXT file create Ok! %r\n\n", Status);
      }
    }
  }
  else {
    if (!StrCmp(OpCmd, L"-W")) {      //Write IP Info
      IPv4_ADDRESS Addr;
      Status = StrToIpv4Address(
        Argv[2], NULL, &Addr, NULL
      );

      if (!EFI_ERROR(Status)) {
        Status = SaveIPAddr(&Addr);
        if (!EFI_ERROR(Status)) {
          Print(L"  Write BOARD IP:%d.%d.%d.%d OK\n",
            Addr.Addr[0], Addr.Addr[1], Addr.Addr[2], Addr.Addr[3]);
        }
        else {
          Print(L"  [ERROR] Failed to Save Address number...%r\n", Status);
        }
      }
      else {
        Print(L"  [ERROR] Invalid Address number...%r\n", Status);
      }
    }
  }

  if (EFI_ERROR(Status)) {
    Print(L"  [ERROR] %s is not valid command %r.\n", OpCmd, Status);
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
    Print(L"  Failed to Open File System %r\n", Status);
    return Status;
  }

  Status = gSimpleFileSystem->OpenVolume(
    gSimpleFileSystem,
    &gRoot
  );

  if (EFI_ERROR(Status)) {
    Print(L"  Failed to Open Root %r\n", Status);    
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
  IPv4_ADDRESS IPAddr;

  CHAR16 BDIpStr[] = L"BOARD IP:%d.%d.%d.%d\n";
  CHAR16 TpcIpStr[] = L"TPC IP:%d.%d.%d.250\n";
  CHAR16 TcpPortStr[] = L"TPC PORT : 30001\n";    
  CHAR16 TmpIpStr[] = L"TEMP IP:%d.%d.%d.%d\n";
  CHAR16 TmpPortStr[] = L"TEMP PORT:30002\n";
  CHAR16 MaskStr[] = L"NETMASK:255.255.0.0\n";
  CHAR16 GWayStr[] = L"GATEWAY:%d.%d.1.1\n";
  CHAR16 VerStr[] = L"Version:%d.%d.%d[%d.%02d.%02d]\n";
  VA_LIST Marker;
  UINTN bufSize;
  
  Status = InitFileHandle();
  if (EFI_ERROR(Status)) return Status;

  Status = gRoot->Open(
    gRoot,
    &IpFile,
    L"ReadIp.TXT",
    EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
    0
  );
  if (EFI_ERROR(Status)) return Status;

  Status = GetIPAddr(&IPAddr);
  if (EFI_ERROR(Status)) return Status;

  // Board IP String
  VA_START(Marker, BDIpStr);
  bufSize = SPrintLength(BDIpStr, Marker);
  VA_END(Marker);

  UnicodeSPrint(
    BDIpStr,
    bufSize,
    L"BOARD IP:%d.%d.%d.%d\n",
    IPAddr.Addr[0], IPAddr.Addr[1], IPAddr.Addr[2], IPAddr.Addr[3]
  );

  Print(L"%s", BDIpStr);
  Status = IpFile->Write(
    IpFile,
    &bufSize,
    BDIpStr
  );

  // TCP IP String
  VA_START(Marker, TpcIpStr);
  bufSize = SPrintLength(TpcIpStr, Marker);
  VA_END(Marker);

  UnicodeSPrint(
    TpcIpStr,
    bufSize,
    L"TPC IP:%d.%d.%d.250\n",
    IPAddr.Addr[0], IPAddr.Addr[1], IPAddr.Addr[2]
  );

  Print(L"%s", TpcIpStr);
  Status = IpFile->Write(
    IpFile,
    &bufSize,
    TpcIpStr
  );

  // TCP Port String
  Print(L"%s", TcpPortStr);
  bufSize = sizeof(TcpPortStr);
  Status = IpFile->Write(
    IpFile,
    &bufSize,
    TcpPortStr
  );

  // TEMP IP String
  VA_START(Marker, TmpIpStr);
  bufSize = SPrintLength(TmpIpStr, Marker);
  VA_END(Marker);

  UnicodeSPrint(
    TmpIpStr,
    bufSize,
    L"TEMP IP:%d.%d.%d.%d\n",
    IPAddr.Addr[0], IPAddr.Addr[1], IPAddr.Addr[2], IPAddr.Addr[3]+5
  );

  Print(L"%s", TmpIpStr);
  Status = IpFile->Write(
    IpFile,
    &bufSize,
    TmpIpStr
  );

  // TEMP Port String
  Print(L"%s", TmpPortStr);
  bufSize = sizeof(TmpPortStr);
  Status = IpFile->Write(
    IpFile,
    &bufSize,
    TmpPortStr
  );

  // NET Mask String
  Print(L"%s", MaskStr);
  bufSize = sizeof(MaskStr);
  Status = IpFile->Write(
    IpFile,
    &bufSize,
    MaskStr
  );

  // Get Way String
  VA_START(Marker, GWayStr);
  bufSize = SPrintLength(GWayStr, Marker);
  VA_END(Marker);

  UnicodeSPrint(
    GWayStr,
    bufSize,
    L"GATEWAY:%d.%d.1.1\n",
    IPAddr.Addr[0], IPAddr.Addr[1]
  );

  Print(L"%s", GWayStr);
  Status = IpFile->Write(
    IpFile,
    &bufSize,
    GWayStr
  );

  // Version String
  VA_START(Marker, VerStr);
  bufSize = SPrintLength(VerStr, Marker);
  VA_END(Marker);

  UnicodeSPrint(
    VerStr,
    bufSize,
    L"Version:%d.%d.%d[%s]\n",
    VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD,Date
  );

  Status = IpFile->Write(
    IpFile,
    &bufSize,
    VerStr
  );

  return Status;
}
