/** @file
  This is a test application that demonstrates how to use the C-style entry point
  for a shell application.

  Copyright (c) 2009 - 2015, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SerialPortLib.h> //brnxxx 20241021 

#include <Protocol/SimpleFileSystem.h>

/*
#define	VERSION_MAJOR						        0			// Major version
#define	VERSION_MINOR						        1			// Minor version 1
#define	VERSION_BUILD						        0			// Build version

#define V1                              1
#define V2                              2
#define V3                              3
*/
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
ShellAppMain(
  IN UINTN   Argc,
  IN CHAR16** Argv
)
{
  EFI_STATUS Status;
  UINT8 UartTest[8] = {0x02, 0x42, 0x0B, 0x0B, 0x00, 0x00, 0x00, 0x03};  //post code test send
  UINTN NumBytes;
  UINTN RetBytes;
  UINTN P80Num;

  NumBytes = sizeof(UartTest);
  RetBytes = 0xFF;
  P80Num = 0;

  Status = SerialPortInitialize();

  if (EFI_ERROR(Status)) {
    Print(L" Failed Serial Port Initialize \n");
    return Status;
  }

  if (Argc > 1) {

    P80Num = StrHexToUintn(Argv[2]);
    UartTest[2] = (UINT8)P80Num & 0x0F;
    UartTest[3] = ((UINT8)P80Num >> 4) & 0x0F;
  }

  RetBytes = SerialPortWrite(UartTest, NumBytes);

  if (RetBytes != 0) {
    Print(L"Serial Port Write Test Fail %d\n", RetBytes);
  }

  Print(L"Serial Port Write %x \n", (UartTest[2]|UartTest[3] << 4));
   
  return 0;
}

/*

  CHAR8 Str[18];
  UINTN size;

  size = StrLen(Argv[1]);

  Print(L"%s\n", Argv[1]);
  Print(L"1Size = %d\n", size);

  Status = UnicodeStrToAsciiStrS(
    Argv[1],
    Str,
    sizeof(Str)
  );

  Print(L"Status %r \n", Status);

  Print(L"Ascii Str %d\n", AsciiStrLen(Str));

  for (UINTN i = 0; i < AsciiStrLen(Str); i++) {
    Print(L"%X", Str[i]);
  }
  Print(L"\n");
  for (UINTN i = 0; i < AsciiStrLen(Str); i++) {
    Print(L"%c", Str[i]);
  }
  /*
  size = StrLen(Argv[1]);

  Print(L"1Size = %d\n", size);

  for (UINTN i = 0; i < 16; i++) {
    Print(L"%x", *(pStr+i));
    Print(L"\n");
  }
  */


/* 
{
  EFI_STATUS Status = EFI_UNSUPPORTED;

  EFI_FILE_PROTOCOL* gRoot = NULL;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* gSimpleFileSystem = NULL;

  Status = gBS->LocateProtocol(
    &gEfiSimpleFileSystemProtocolGuid,
    NULL,
    (VOID**)&gSimpleFileSystem
  );

  if (EFI_ERROR(Status)) {
    Print(L"  Failed to Open File System\n");
    return Status;
  }

  Print(L"  Open File System %r\n", Status);

  Status = gSimpleFileSystem->OpenVolume(
    gSimpleFileSystem,
    &gRoot
  );

  if (EFI_ERROR(Status)) {
    Print(L"  Failed to Open Root\n");
    return Status;
  }

  Print(L"  Open File Root %r\n", Status);

  return Status;

}
*/

/*
{
  EFI_STATUS Status;
  IPv4_ADDRESS Addr;
  Status = StrToIpv4Address(
    Argv[1], NULL, &Addr, NULL
  );

  Print(L"IP Address %d.%d.%d.%d\n", Addr.Addr[0], Addr.Addr[1], Addr.Addr[2], Addr.Addr[3]);

  return Status;
}

  CHAR16 str[] = L"VC S/W VERSION=%d.%d.%d\n VC F/W VERSION=%d.%d.%d\n";
  CHAR16 SlotStr[] = L"Slot%d B\n";
  VA_LIST Marker;
  UINTN bufSize;

  VA_START(Marker, str);
  bufSize = SPrintLength(str, Marker);
  VA_END(Marker);

  Print(L"STR Buffer Size %d\n", bufSize);
  UnicodeSPrint(
    str,
    bufSize,
    L"VC S/W VERSION=%d.%d.%d\nVC F/W VERSION=%d.%d.%d\n",
    VERSION_MAJOR,
    VERSION_MINOR,
    VERSION_BUILD,
    V1,
    V2,
    V3
  );

  Print(L"%s", str);

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
    Print(L"\n");
  }
  
  Print(L"\n");
  return 0;
}
*/
  /*
  UINTN Slot;
  UINT8 Masked = 0x00;

  Slot = StrDecimalToUintn(Argv[1]);
 
  Print(L"  [ERROR] Invalid socket number : %d\n", Slot);
 

  if (Slot != 0) Masked |= ~(0x1 << (((UINT8)Slot) - 1));

  Print(L"  [ERROR] Invalid Masked number : 0x%2X\n", Masked);


  for (UINT8 i = 1; i <= 8; i++) {
    if (!(Masked & (0x01 << (i - 1)))) {
      Print(L"  unMasked number : %d\n", i);
    }
  }
  */
  /*
  UINTN  Index;
  UINTN  bufSize;
  CHAR16 str[] = L"VC S/W VERSION=%d.%d.%d\n VC F/W VERSION=%d.%d.%d\n";
  CHAR16 SlotStr[] = L"Slot%d B\n";
  VA_LIST Marker;

  if (Argc == 1) {
    Print (L"Argv[1] = NULL\n");
  }

  for (Index = 1; Index < Argc; Index++) {
    Print (L"Argv[%d]: \"%s\"\n", Index, Argv[Index]);
  }

  VA_START(Marker, str);
  bufSize = SPrintLength(str, Marker);
  VA_END(Marker);
  Print(L"STR Buffer Size %d\n", bufSize);

  VA_START(Marker, SlotStr);
  bufSize = SPrintLength(SlotStr, Marker);
  VA_END(Marker);
  Print(L"SlotStr Buffer Size %d\n", bufSize);
  */
