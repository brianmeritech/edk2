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
#include <Protocol/SimpleFileSystem.h>


#define	VERSION_MAJOR						        0			// Major version
#define	VERSION_MINOR						        1			// Minor version 1
#define	VERSION_BUILD						        0			// Build version

#define V1                              1
#define V2                              2
#define V3                              3

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
  EFI_STATUS Status = EFI_UNSUPPORTED;

  EFI_FILE_PROTOCOL* gRoot = NULL;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* gSimpleFileSystem = NULL;
  EFI_FILE_PROTOCOL* NewFile = NULL;
  CHAR16 str[] = L"VC S/W VERSION=%d.%d.%d\n VC F/W VERSION=%d.%d.%d\n";
  CHAR16 SlotStr[] = L"Slot%d B\n";
  VA_LIST Marker;
  UINTN bufSize;

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

  if (Argc == 2 && Argv[2] != NULL) {
    Status = gRoot->Open(
      gRoot,
      &NewFile,
      Argv[2],
      EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
      0
    );
  }
  else {
    Status = gRoot->Open(
      gRoot,
      &NewFile,
      L"NewFile.TXT",
      EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
      0
    );   
  }

  if (!EFI_ERROR(Status)) {
    VA_START(Marker, str);
    bufSize = SPrintLength(str, Marker);
    VA_END(Marker);

    UnicodeSPrint(
      str,
      bufSize,
      L"VC S/W VERSION=%d.%d.%d\nVC F/W VERSION=%d.%d.%d\n",
      VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD,
      V1, V2, V3
    );

    Status = NewFile->Write(
      NewFile,
      &bufSize,
      str
    );

    VA_START(Marker, SlotStr);
    bufSize = SPrintLength(SlotStr, Marker);
    VA_END(Marker);

    for (UINT8 i = 1; i < 9; i++) {
      UnicodeSPrint(
        SlotStr,
        bufSize,
        L"Slot%d B\n",
        i
      );
      Status = NewFile->Write(
        NewFile,
        &bufSize,
        SlotStr
      );
    }
    
    if (!EFI_ERROR(Status)) {
      NewFile->Close(NewFile);
    }
  }

  Status = gRoot->Close(gRoot);
  Print(L" gRoot Close %r\n", Status); 

  return Status;
}

