#ifndef pci_db_type_h
#define pci_db_type_h
#include <efi.h>
#include <efidef.h>

typedef struct {
    CHAR16 deviceId;
    CHAR16 deviceName[64];
} PCI_DEVICE_DB;

typedef struct {
    UINT16 vendorId;
    CHAR16 vendorName[12];
    UINT16 numOfDevices;
    PCI_DEVICE_DB devices[];
} PCI_VENDOR_DB;

extern PCI_VENDOR_DB**  pci_vendor_db;
extern UINT16           pci_vendor_db_size;

#endif
