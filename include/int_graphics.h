#ifndef int_graphics_h
#define int_graphics_h
#include <efi.h>
#include <efiapi.h>
#include <efidef.h>

VOID _INT_SetGraphicsMode(EFI_BOOT_SERVICES* BS, BOOLEAN Enable);

#endif
