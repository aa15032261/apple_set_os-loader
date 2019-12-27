#include "../include/int_graphics.h"
#include "../include/int_guid.h"

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

