/** @file
  This is a test application that demonstrates how to use the C-style entry point
  for a shell application.

  Copyright (c) 1996 - 2024, Meritech Corporation. All rights reserved.<BR>

**/

#include "VC.h"
#include "SF.h"

EFI_SERIAL_IO_PROTOCOL* gSerial = NULL;


/* Function Implement */
EFI_STATUS
InitSerialFunc(
  void
)
{
  EFI_STATUS  Status;  
  EFI_HANDLE* HandleBuffer;
  EFI_HANDLE  Uart2Handle;
  UINTN HandleCount;
  UINTN Index;
  EFI_DEVICE_PATH_PROTOCOL* DevicePath;

  Status = gBS->LocateHandleBuffer(
    ByProtocol,
    &gEfiSerialIoProtocolGuid,
    NULL,
    &HandleCount,
    &HandleBuffer
  );
 
  if (EFI_ERROR(Status)) {
    Print(L"Failed to locate Serial IO handles %r\n", Status);
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol(
      HandleBuffer[Index],
      &gEfiSerialIoProtocolGuid,
      (void**)&gSerial
    );

    if (EFI_ERROR(Status)) {
      Print(L"Failed to Handle Serial Protocol on Handle %u: %r\n", Index, Status);
      continue;
    }

    Status = gBS->HandleProtocol(
      HandleBuffer[Index],
      &gEfiDevicePathProtocolGuid,
      (void**)&DevicePath
    );

    if (EFI_ERROR(Status)) {
      Print(L"Failed to Handle Device Path Protocol on Handle %u: %r\n", Index, Status);
      continue;
    }

    EFI_DEVICE_PATH_PROTOCOL* CurrentNode = DevicePath;

    while (!IsDevicePathEnd(CurrentNode)) {
      if (DevicePathType(CurrentNode) == ACPI_DEVICE_PATH && DevicePathSubType(CurrentNode) == ACPI_DP) {
        ACPI_HID_DEVICE_PATH* AcpiNode = (ACPI_HID_DEVICE_PATH*)CurrentNode;

        Print(L"Handle 0x%8X, Check Device UID %d\n", HandleBuffer[Index], AcpiNode->UID);

        if (AcpiNode->UID == 0x1) {
          Print(L"Found COMB 0x2F8 at Index = %d\n", Index);
          Uart2Handle = HandleBuffer[Index];
          return Status;
        }
        // Move to the next node in the device path
        CurrentNode = NextDevicePathNode(CurrentNode);
      }
    }    
  }

  if (HandleBuffer != NULL) {
    FreePool(HandleBuffer);
  }

  return Status = EFI_NOT_FOUND;  
 
}
