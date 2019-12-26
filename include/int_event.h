#ifndef int_event_h
#define int_event_h
#include <efi.h>
#include <efiapi.h>
#include <efidef.h>


EFI_STATUS _INT_WaitForSingleEvent(EFI_BOOT_SERVICES* BS, EFI_EVENT Event, UINT64 Timeout);

#endif
