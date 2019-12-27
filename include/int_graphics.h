#ifndef int_graphics_h
#define int_graphics_h
#include <efi.h>
#include <efiapi.h>
#include <efidef.h>

typedef struct {
    EFI_BOOT_SERVICES*              BS;
    SIMPLE_TEXT_OUTPUT_INTERFACE*   ConOut;
    CHAR16**                        buf;
    UINTN                           col;
    UINTN                           row;
} _INT_SimpleTextGraphicsStruct;


VOID _INT_SetGraphicsMode(EFI_BOOT_SERVICES* BS, BOOLEAN Enable);

VOID _INT_SimpleTextGraphicsInit(_INT_SimpleTextGraphicsStruct* gs);
VOID _INT_SimpleTextGraphicsDeinit(_INT_SimpleTextGraphicsStruct* gs);
VOID _INT_SimpleTextGraphicsPrint(
    _INT_SimpleTextGraphicsStruct* gs,
    UINTN col,
    UINTN row,
    BOOLEAN clear,
    BOOLEAN refresh,
    CHAR16* fmt,
    ...
);
VOID _INT_SimpleTextGraphicsRefresh(_INT_SimpleTextGraphicsStruct* gs);


#endif
