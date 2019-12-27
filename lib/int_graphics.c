#include "../include/int_graphics.h"
#include "../include/int_guid.h"
#include "../include/int_mem.h"
#include "../include/int_print.h"

VOID _INT_SetGraphicsMode(EFI_BOOT_SERVICES* BS, BOOLEAN Enable)
{
    EFI_CONSOLE_CONTROL_PROTOCOL* ConsoleControl = NULL;

    EFI_GUID efi_console_control_protocol_guid = EFI_CONSOLE_CONTROL_PROTOCOL_GUID;

    // get protocols
    EFI_STATUS Status = BS->LocateProtocol(&efi_console_control_protocol_guid, NULL, (VOID**) &ConsoleControl);
    if (!EFI_ERROR(Status)) {
        EFI_CONSOLE_CONTROL_SCREEN_MODE CurrentMode;
        EFI_CONSOLE_CONTROL_SCREEN_MODE NewMode;

        ConsoleControl->GetMode(ConsoleControl, &CurrentMode, NULL, NULL);

        NewMode = Enable ? EfiConsoleControlScreenGraphics : EfiConsoleControlScreenText;
        if (CurrentMode != NewMode) {
            ConsoleControl->SetMode(ConsoleControl, NewMode);
        }
    }
}

VOID _INT_SimpleTextGraphicsInit(_INT_SimpleTextGraphicsStruct* gs)
{
    if (gs->buf == NULL) {
        EFI_STATUS Status;

        UINTN MaxCol = 80;
        UINTN MaxRow = 25;
        INT32 Mode   = gs->ConOut->Mode->Mode;

        for (INT32 i = 0; i < gs->ConOut->Mode->MaxMode; i++) {
            UINTN TmpMaxCol = 80;
            UINTN TmpMaxRow = 25;

            Status = gs->ConOut->QueryMode(
                gs->ConOut,
                i,
                &TmpMaxCol,
                &TmpMaxRow
            );

            if (!EFI_ERROR(Status)) {
                if (TmpMaxCol * TmpMaxRow > MaxCol * MaxRow ) {
                    Mode = i;
                    MaxCol = TmpMaxCol;
                    MaxRow = TmpMaxCol;
                }
            }
        }

        Status = gs->ConOut->SetMode(
            gs->ConOut,
            Mode
        );
        if (EFI_ERROR(Status)) {
            MaxCol = 80;
            MaxRow = 25;
        }

        gs->col = MaxCol;
        gs->row = MaxRow - 1;

        gs->buf = (CHAR16**)_INT_AllocatePool(gs->BS, gs->row * sizeof(CHAR16*));

        for (UINTN i = 0; i < gs->row; i++) {
            gs->buf[i] = (CHAR16*)_INT_AllocatePool(gs->BS, (gs->col + 1) * sizeof(CHAR16));
            
            for (UINTN j = 0; j < gs->col; j++) {
                gs->buf[i][j] = 0x20;
            }
            gs->buf[i][gs->col] = 0;
        }

        gs->ConOut->Reset(gs->ConOut, FALSE);
        gs->ConOut->SetCursorPosition(gs->ConOut, 0, 0);
        gs->ConOut->ClearScreen(gs->ConOut);
        gs->ConOut->EnableCursor(gs->ConOut, FALSE);
    }
}

VOID _INT_SimpleTextGraphicsDeinit(_INT_SimpleTextGraphicsStruct* gs)
{
    if (gs->buf != NULL) {
        for (UINTN i = 0; i < gs->row; i++) {
            _INT_FreePool(gs->BS, gs->buf[i]);
        }

        _INT_FreePool(gs->BS, gs->buf);

        gs->buf = NULL;
    }
}

VOID _INT_SimpleTextGraphicsPrint(
    _INT_SimpleTextGraphicsStruct* gs,
    UINTN col,
    UINTN row,
    BOOLEAN clear,
    BOOLEAN refresh,
    CHAR16* fmt,
    ...
)
{
    if (gs->buf != NULL) {

        CHAR16* _tmp = (CHAR16*)_INT_AllocatePool(gs->BS, (gs->col - col) * sizeof(CHAR16));
        _INT_memset(_tmp, 0, (gs->col - col) * sizeof(CHAR16));

        va_list args;
        va_start(args, fmt);
        _INT_VPoolPrint(_tmp, (gs->col - col) * sizeof(CHAR16), fmt, args);
        va_end(args);

        INT32 len = _INT_wcslen(_tmp);

        if (col < gs->col && row < gs->row) {
            if (col + len >= gs->col) {
                len = gs->col - col;
            }

            if (clear) {
                for (UINTN j = 0; j < gs->col; j++) {
                    gs->buf[row][j] = 0x20;
                }
            }
            
            _INT_memcpy(gs->buf[row] + col, _tmp, len * sizeof(CHAR16));
            gs->buf[row][gs->col] = 0;
        }

        _INT_FreePool(gs->BS, _tmp);

        if (refresh) {
            _INT_SimpleTextGraphicsRefresh(gs);
        }
    }
}

VOID _INT_SimpleTextGraphicsRefresh(_INT_SimpleTextGraphicsStruct* gs)
{
    if (gs->buf != NULL) {
        for (UINTN i = 0; i < gs->row; i++) {
            gs->ConOut->SetCursorPosition(gs->ConOut, 0, i);
            gs->ConOut->OutputString(gs->ConOut, gs->buf[i]);
        }

        gs->ConOut->SetCursorPosition(gs->ConOut, 0, 0);
    }
}
