/** @file
  This is a test application that demonstrates how to use the C-style entry point
  for a shell application.

  Copyright (c) 1996 - 2024, Meritech Corporation. All rights reserved.<BR>


**/
#include "IPReader.h"

/*******************************************************************************
 *	Date		  Version		Comment
 *	24/10/11  V0.1.0		Initial Version
 ******************************************************************************/
#define VERSION_MAJOR						0
#define VERSION_MINOR						1
#define VERSION_BUILD					  0

#define MAX_TRY_CNT							5

#define MAX_ARGUMENT_STRING			16

#define SIZE_CMD_ARGS						16
#define SIZE_CMD_PACKET					8
#define SIZE_MAX_ID_SN					16
#define STX									    0x02
#define ETX									    0x03

#define CMD_CHECK_CONNECTION		0x41
#define CMD_GET_IP_INFO					0x71
#define CMD_SET_IP_INFO					0x78

#define TYPE_IP_ADDRESS					1
#define TYPE_SUBNET_MASK				2
#define TYPE_GATEWAY						3

 /*Function declaration*/
void
PrintHelpMsg(
  void
);

void ToUpperCase(
  CHAR16*,
  CHAR16*
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
    
  if (Argc == 2) {
    if (!StrCmp(OpCmd, L"-R")) {      //Read IP Info
      //TBD 
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
