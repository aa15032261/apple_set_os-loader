#ifndef int_dpath_h
#define int_dpath_h
#include <efi.h>
#include <efiapi.h>
#include <efidef.h>
#include <efilib.h>
#include "../include/int_mem.h"


UINTN _INT_DevicePathSize(EFI_DEVICE_PATH* DevPath);

EFI_DEVICE_PATH* _INT_DuplicateDevicePath(EFI_BOOT_SERVICES* BS, EFI_DEVICE_PATH* DevPath);

EFI_DEVICE_PATH* _INT_DevicePathInstance(EFI_DEVICE_PATH** DevicePath, UINTN* Size);

UINTN _INT_DevicePathInstanceCount(EFI_DEVICE_PATH* DevicePath);

EFI_DEVICE_PATH* _INT_DevicePathFromHandle(EFI_BOOT_SERVICES* BS, EFI_HANDLE Handle);

EFI_DEVICE_PATH* _INT_AppendDevicePath(EFI_BOOT_SERVICES* BS, EFI_DEVICE_PATH* Src1, EFI_DEVICE_PATH* Src2);

EFI_DEVICE_PATH* _INT_FileDevicePath(EFI_BOOT_SERVICES* BS, EFI_HANDLE Device, CHAR16* FileName);

#endif
