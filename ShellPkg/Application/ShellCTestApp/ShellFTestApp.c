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
  EFI_STATUS Status = EFI_UNSUPPORTED;

  EFI_FILE_PROTOCOL* gRoot = NULL;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* gSimpleFileSystem = NULL;
  EFI_FILE_PROTOCOL* NewFile = NULL;

  UINTN strsize=0;

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
    CHAR8* pStr = "UEFI FILE SYSTEM TEST \n";
    strsize = AsciiStrSize(pStr);
    Status = NewFile->Write(
      NewFile,
      &strsize,
      pStr
    );
    if (!EFI_ERROR(Status)) {
      NewFile->Close(NewFile);
    }
  }

  Status = gRoot->Close(gRoot);

  Print(L" gRoot Close %r\n", Status); 

  return Status;
}

