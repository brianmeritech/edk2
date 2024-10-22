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

  if (Argc == 2 && Argv[2] != NULL) {

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

