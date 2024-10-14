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

void SetP80(
  UINTN
);




#endif
