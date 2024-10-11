/** @file
  This is a test application that demonstrates how to use the C-style entry point
  for a shell application.

  Copyright (c) 1996 - 2024, Meritech Corporation. All rights reserved.<BR>


**/

#include "IPReader.h"

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
  UINTN  Index;
  CHAR16 OpCmd[MAX_ARGUMENT_STRING] = { 0, };

  if (Argc == 1 || Argc >3) {
    PrintHelpMsg();
    return Status;
  }

  Print(L"IPReader for PCT3.0 GNRAP MRDIMM V%d.%d.%d %s\n",
    VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, __DATE__);

  ToUpperCase(Argv[1], OpCmd);

  for (Index = 1; Index < Argc; Index++) {
    Print (L"Argv[%d]: \"%s\"\n", Index, Argv[Index]);
  }

  if (Argc == 2) {
    if (!StrCmp(OpCmd, L"-R")) {      //Read IP Info
      //TBD 
    }
  }
  else {
    if (!StrCmp(OpCmd, L"-W")) {   //Write IP Info
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
