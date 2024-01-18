#ifndef guid_h
#define guid_h

#include <efidef.h>
#include <efiprot.h>

#define EFI_CONSOLE_CONTROL_PROTOCOL_GUID   { 0xf42f7782, 0x012e, 0x4c12, { 0x99, 0x56, 0x49, 0xf9, 0x43, 0x04, 0xf7, 0x21 } };
#define EFI_PCI_EMUMERATION_COMPLETE_GUID   { 0x30cfe3e7, 0x3de1, 0x4586, { 0xbe, 0x20, 0xde, 0xab, 0xa1, 0xb3, 0xb7, 0x93 } };

#define APPLE_SET_OS_GUID                   { 0xc5c5da95, 0x7d5c, 0x45e6, { 0xb2, 0xf1, 0x3f, 0xd5, 0x2b, 0xb1, 0x00, 0x77 } };

#define EFI_DEVICE_PATH_PROTOCOL_GUID \
    { 0x9576e91, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }

typedef enum {
    EfiConsoleControlScreenText,
    EfiConsoleControlScreenGraphics,
    EfiConsoleControlScreenMaxValue
} EFI_CONSOLE_CONTROL_SCREEN_MODE;

typedef struct _EFI_CONSOLE_CONTROL_PROTOCOL EFI_CONSOLE_CONTROL_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_CONSOLE_CONTROL_PROTOCOL_GET_MODE) (
    IN  EFI_CONSOLE_CONTROL_PROTOCOL      *This,
    OUT EFI_CONSOLE_CONTROL_SCREEN_MODE   *Mode,
    OUT BOOLEAN                           *GopUgaExists,  OPTIONAL
    OUT BOOLEAN                           *StdInLocked    OPTIONAL
);

typedef EFI_STATUS (EFIAPI *EFI_CONSOLE_CONTROL_PROTOCOL_SET_MODE) (
    IN  EFI_CONSOLE_CONTROL_PROTOCOL      *This,
    IN  EFI_CONSOLE_CONTROL_SCREEN_MODE   Mode
);

typedef EFI_STATUS (EFIAPI *EFI_CONSOLE_CONTROL_PROTOCOL_LOCK_STD_IN) (
    IN  EFI_CONSOLE_CONTROL_PROTOCOL      *This,
    IN CHAR16                             *Password
);

struct _EFI_CONSOLE_CONTROL_PROTOCOL {
    EFI_CONSOLE_CONTROL_PROTOCOL_GET_MODE           GetMode;
    EFI_CONSOLE_CONTROL_PROTOCOL_SET_MODE           SetMode;
    EFI_CONSOLE_CONTROL_PROTOCOL_LOCK_STD_IN        LockStdIn;
};

typedef struct EFI_APPLE_SET_OS_IFACE {
    UINT64 Version;
    EFI_STATUS (EFIAPI *SetOsVersion) (IN CHAR8 *Version);
    EFI_STATUS (EFIAPI *SetOsVendor) (IN CHAR8 *vendor);
} EFI_APPLE_SET_OS_IFACE;

#endif
