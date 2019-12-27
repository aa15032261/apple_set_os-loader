#ifndef int_print_h
#define int_print_h
#include <efi.h>
#include <efiapi.h>
#include <efidef.h>
#include <efilib.h>
#include <efistdarg.h>

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

UINTN _INT_PoolPrint (
    IN CHAR16                           *buf,
    IN UINTN                            size,
    IN CONST CHAR16                     *fmt,
    ...
);

UINTN _INT_VPoolPrint (
    IN CHAR16                           *buf,
    IN UINTN                            size,
    IN CONST CHAR16                     *fmt,
    IN va_list                          args
);

VOID ValueToString (
    IN CHAR16   *Buffer,
    IN BOOLEAN  Comma,
    IN INT64    v
);

VOID FloatToString (
    IN CHAR16   *Buffer,
    IN BOOLEAN  Comma,
    IN double   v
);

VOID TimeToString (
    OUT CHAR16      *Buffer,
    IN EFI_TIME     *Time
);
#endif