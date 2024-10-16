#ifndef MAIN_FUNC_H_
#define MAIN_FUNC_H_

EFI_STATUS
InitSerialPort(
  void
  );

EFI_STATUS
CheckConnect(
  void
  );

EFI_STATUS
GetFWVersion(
  UINT8*,
  UINT8*,
  UINT8*
  );

void
SetP80(
  UINTN
  );

UINT16
GetFanRPM(
  UINTN
  );

EFI_STATUS
SetLEDStatus(
  CHAR16*
  );

EFI_STATUS
SetFanSpeed(
  UINTN,
  UINTN
  );

EFI_STATUS
GetSlotCount(
  UINTN,
  UINT32*
  );

EFI_STATUS
SetSlotCountAct(
  CHAR16*
  );


EFI_STATUS
GetOutVoltage(
  UINTN,
  UINT16*
  );

#endif
