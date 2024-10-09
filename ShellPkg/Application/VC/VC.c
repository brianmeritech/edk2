/** @file
  This is a test application that demonstrates how to use the C-style entry point
  for a shell application.

  Copyright (c) 1996 - 2024, Meritech Corporation. All rights reserved.<BR>
  
**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>

#include "VC.h"

/*Definition*/

/*Function declaration*/
void
PrintHelpMsg(
  void
);


/**
 * @brief  convert string to upper case
 * @note
 * @param  *src: source string
 * @param  *dest: converted string
 * @retval None
 */
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
  UINTN  Index;
  CHAR16 OpCmd[SIZE_ARGUMENT_MAX] = { 0, };

  //Print(L"Voltage Control Program for PCT3.0-GNRAP MRDIMM V%d.%d.%d %s\n",
  //  VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, __DATE__);

  if (Argc == 1) {
    PrintHelpMsg();
    return 0;
  }

  for (Index = 1; Index < Argc; Index++) {
        Print(L"Argv[%d]: \"%s\"\n", Index, Argv[Index]);
  }

  ToUpperCase(Argv[1], OpCmd);

  if (Argc == 2) {
    if (!StrCmp(OpCmd, L"-H")) {
        PrintHelpMsg();
    }
    else if (!StrCmp(OpCmd, L"-CC")) {  //Check Connect

    }
    else if (!StrCmp(OpCmd, L"-GR")) {  //Get Firwmare Version

    }
    else if (!StrCmp(OpCmd, L"-ID")) {  //Get ID 

    }
    else {
      Print(L"  [ERROR] %s is not valid option.\n", OpCmd);
    }
  }
  
  return 0;
}


void PrintHelpMsg()
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
