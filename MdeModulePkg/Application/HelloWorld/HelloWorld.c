/** @file
  This sample application bases on HelloWorld PCD setting
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/IoLib.h>

#define COM2    0x3E8

void initUartPort(UINT8 port)
{
  IoWrite8(0x80, 0x08);
  IoWrite8(port + 3, 0x80);
  IoWrite8(port + 1, 0);
  IoWrite8(port, 0x01);    // 115200 bps
  IoWrite8(port + 3, 0x03);
  IoWrite8(port + 2, 0xC7);
  IoWrite8(port + 1, 0x00);

                    // Delay 20ms
}

void sendUartBuffer(UINT8* tbuf, UINT8 tcnt)
{
  UINT8 i;
  UINT8 regData;

  i = 0;
  while (i < tcnt) {
    do {
      regData = IoRead8(COM2 + 5);
    } while (!(regData & (1 << 5)));

    IoWrite8(COM2, tbuf[i]);
   
    i++;
  }
  // printf("\n");
}

VOID SerialOutTest()
{
  UINT8 HelloStr[] = { "Hello World GNRAP\n" };
  UINT8 StrSize = sizeof(HelloStr);
  sendUartBuffer(&HelloStr[0], StrSize);
  sendUartBuffer(&HelloStr[0], 18);
}

//
// String token ID of help message text.
// Shell supports to find help message in the resource section of an application image if
// .MAN file is not found. This global variable is added to make build tool recognizes
// that the help string is consumed by user and then build tool will add the string into
// the resource section. Thus the application can use '-?' option to show help message in
// Shell.
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID  mStringHelpTokenId = STRING_TOKEN (STR_HELLO_WORLD_HELP_INFORMATION);

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT32  Index;

  Index = 0;

  //
  // Three PCD type (FeatureFlag, UINT32 and String) are used as the sample.
  //
  if (FeaturePcdGet (PcdHelloWorldPrintEnable)) {
    for (Index = 0; Index < PcdGet32 (PcdHelloWorldPrintTimes); Index++) {
      //
      // Use UefiLib Print API to print string to UEFI console
      //
      Print ((CHAR16 *)PcdGetPtr (PcdHelloWorldPrintString));
    }
  }

  //brnxxxx 20240827 gnrap hello world test >>>>
  SerialOutTest();
  //brnxxxx 20240827 gnrap hello world test <<<<

  return EFI_SUCCESS;
}
