#ifndef int_mem_h
#define int_mem_h
#include <efi.h>
#include <efiapi.h>
#include <efidef.h>

VOID* _INT_AllocatePool(EFI_BOOT_SERVICES* BS, UINTN Size);

VOID _INT_FreePool(EFI_BOOT_SERVICES* BS, VOID *Buffer);

VOID _INT_memcpy(CHAR8* Dst, CHAR8* Src, UINTN Size);

VOID _INT_memset(CHAR8* Dst, CHAR8 Val, UINTN Size);

UINTN _INT_strlen16(CHAR8* str);

#endif
