#ifndef int_mem_h
#define int_mem_h
#include <efi.h>
#include <efiapi.h>
#include <efidef.h>

VOID* _INT_AllocatePool(EFI_BOOT_SERVICES* BS, UINTN Size);

VOID* _INT_ReallocatePool(EFI_BOOT_SERVICES* BS, VOID* OldPool, UINTN OldSize, UINTN NewSize);

VOID _INT_FreePool(EFI_BOOT_SERVICES* BS, VOID *Buffer);

VOID _INT_memcpy(VOID* Dst, VOID* Src, UINTN Size);

VOID _INT_memset(VOID* Dst, CHAR8 Val, UINTN Size);

UINTN _INT_wcslen(CHAR16* str);

#endif
