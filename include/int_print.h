#ifndef int_print_h
#define int_print_h
#include <efi.h>
#include <efiapi.h>
#include <efidef.h>
#include <efilib.h>
#include <efistdarg.h>

VOID _INT_Clear (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE    *Out
);

UINTN _INT_IPrint (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE    *Out,
    IN CONST CHAR16                    *fmt,
    ...
);

UINTN _INT_IPrintAt (
    IN SIMPLE_TEXT_OUTPUT_INTERFACE     *Out,
    IN UINTN                            Column,
    IN UINTN                            Row,
    IN CONST CHAR16                     *fmt,
    ...
);
#endif