/** @file
  This is a test application that demonstrates how to use the C-style entry point
  for a shell application.

  Copyright (c) 1996 - 2024, Meritech Corporation. All rights reserved.<BR>
 

**/
#include "IDReader.h"
#include "MF.h"

/*******************************************************************************
 *	Date		  Version		Comment
 *	24/10/11	V0.1.0		Initial Version
 ******************************************************************************/
#define VERSION_MAJOR						0
#define VERSION_MINOR						1
#define VERSION_BUILD					  0
 
#define MAX_ARGUMENT_STRING     16
#define SIZE_CMD_ARGS						16

CHAR16 Date[12];
CHAR8 AsciiStr[18];
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
  EFI_STATUS  Status = EFI_INVALID_PARAMETER;
  CHAR16 OpCmd1[SIZE_CMD_ARGS] = { 0, };
  CHAR16 OpCmd2[SIZE_CMD_ARGS] = { 0, };
  
  UnicodeSPrintAsciiFormat(
    &Date[0],
    sizeof(Date),
    __DATE__
  );

  Print(L"IDReader for PCT3.0 GNRAP MRDIMM V%d.%d.%d %s\n",
      VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, Date);

  if (Argc == 1 || Argc > 4) {
    PrintHelpMsg();
    return Status;
  }

  ToUpperCase(Argv[1], OpCmd1);
  
  if (!StrCmp(OpCmd1, L"-CC")) {          // Check Connection
    Status = CheckConnect();
    if (!EFI_ERROR(Status)) {
      Print(L"  Connection Ok!\n");
    }
    else {
      Print(L"  Connection Error!\n");
    }
  }
  else if (!StrCmp(OpCmd1, L"-R")) {      // Read Board ID
    if (Argc < 3) {
      Print(L"  [ERROR] Not enough command options.\n");
    }
    else {
      ToUpperCase(Argv[1], OpCmd2);
      if (!StrCmp(OpCmd2, L"-SN")) {      // Read Board SN
        //TBD


      }
      else if (!StrCmp(OpCmd2, L"-ID")) { //Read Board ID
        //TBD


      }
    }
  }
  else if (!StrCmp(OpCmd1, L"-W")) {
    if (Argc < 4) {
      Print(L"  [ERROR] Not enough command options.\n");      
    }
    else {     
      ToUpperCase(Argv[2], OpCmd2);
      Status = UnicodeStrToAsciiStrS(
        Argv[3],
        AsciiStr,
        sizeof(AsciiStr)
      );

      if (EFI_ERROR(Status) || StrLen(Argv[3]) != MAX_ARGUMENT_STRING) {
        Print(L"  [ERROR] BOARD ID/SN %s are not valid.\n", Argv[3]);
      }

      if (!StrCmp(OpCmd2, L"-SN")) {      //Write Board SN      
        Status = SaveBoardInfo(
          0x75,  //Set SN
          AsciiStr
        );
        if (!EFI_ERROR(Status)) {
          Print(L"  Write SN [%s] OK\n", Argv[3]);
        }
      }
      else if (!StrCmp(OpCmd2, L"-ID")) { //Write Board ID
        Status = SaveBoardInfo(
          0x73,  //Set ID
          AsciiStr
        );
        if (!EFI_ERROR(Status)) {
          Print(L"  Write ID [%s] OK\n", Argv[3]);
        }
      }
    }
  }

  if (EFI_ERROR(Status)) {
    Print(L"  [ERROR] %s %s are not valid command.\n", OpCmd1, OpCmd2);    
  }

  return Status;
}

void PrintHelpMsg(void)
{
  Print(L"Copyright (c) 1996 - 2024, Meritech Corporation. All rights reserved \n");
  Print(L"  usage : IDREADER [-R/W] [-SN/-ID] <ID String>\n");
  Print(L"          IDREADER -CC     (Check connection)\n");
  Print(L"          IDREADER -R -SN  (Read Bd ID -> EP_R_ID.TXT)\n");
  Print(L"          IDREADER -R -ID  (Read Bd SN -> EP_R_SN.TXT)\n");
  Print(L"          IDREADER -W -SN  Serial Number (Write Bd SN )\n");
  Print(L"          IDREADER -W -ID  ID            (Write Bd ID )\n");
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
