#include "../include/int_dpath.h"
#include "../include/int_guid.h"

UINTN _INT_DevicePathSize(EFI_DEVICE_PATH* DevPath)
{
    EFI_DEVICE_PATH* Start;

    //
    // Search for the end of the device path structure
    //

    Start = DevPath;
    while (!IsDevicePathEnd(DevPath)) {
        DevPath = NextDevicePathNode(DevPath);
    }

    return ((UINTN)DevPath - (UINTN)Start) + sizeof(EFI_DEVICE_PATH);
}

EFI_DEVICE_PATH* _INT_DuplicateDevicePath(EFI_BOOT_SERVICES* BS, EFI_DEVICE_PATH* DevPath)
{
    EFI_DEVICE_PATH* NewDevPath;
    UINTN Size;

    Size = _INT_DevicePathSize(DevPath);

    NewDevPath = _INT_AllocatePool (BS, Size);
    if (NewDevPath) {
        _INT_memcpy(NewDevPath, (CHAR8*)DevPath, Size);
    }

    return NewDevPath;
}

EFI_DEVICE_PATH* _INT_DevicePathInstance(EFI_DEVICE_PATH** DevicePath, UINTN* Size)
{
    EFI_DEVICE_PATH         *Start, *Next, *DevPath;
    UINTN                   Count;

    DevPath = *DevicePath;
    Start = DevPath;

    if (!DevPath) {
        return NULL;
    }

    //
    // Check for end of device path type
    //

    for (Count = 0; ; Count++) {

        Next = NextDevicePathNode(DevPath);

        if (IsDevicePathEndType(DevPath)) {
            break;
        }

        if (Count > 01000) {
            //
            // BugBug: Debug code to catch bogus device paths
            //
            break;
        }

        DevPath = Next;
    }

    //
    // Set next position
    //

    if (DevicePathSubType(DevPath) == END_ENTIRE_DEVICE_PATH_SUBTYPE) {
        Next = NULL;
    }

    *DevicePath = Next;

    //
    // Return size and start of device path instance
    //

    *Size = ((UINT8 *) DevPath) - ((UINT8 *) Start);
    return Start;
}

UINTN _INT_DevicePathInstanceCount(EFI_DEVICE_PATH* DevicePath)
{
    UINTN Count, Size;

    Count = 0;
    while (_INT_DevicePathInstance(&DevicePath, &Size)) {
        Count += 1;
    }

    return Count;
}

EFI_DEVICE_PATH* _INT_DevicePathFromHandle(EFI_BOOT_SERVICES* BS, EFI_HANDLE Handle)
{
    EFI_STATUS          Status;
    EFI_DEVICE_PATH     *DevicePath;

    EFI_GUID efi_device_path_protocol_guid = EFI_DEVICE_PATH_PROTOCOL_GUID;

    Status = BS->HandleProtocol(Handle, &efi_device_path_protocol_guid, (VOID*)&DevicePath);
    if (EFI_ERROR(Status)) {
        DevicePath = NULL;
    }

    return DevicePath;
}

EFI_DEVICE_PATH* _INT_AppendDevicePath(EFI_BOOT_SERVICES* BS, EFI_DEVICE_PATH* Src1, EFI_DEVICE_PATH* Src2)
{
    UINTN               Src1Size, Src1Inst, Src2Size, Size;
    EFI_DEVICE_PATH     *Dst, *Inst;
    UINT8               *DstPos;

    //
    // If there's only 1 path, just duplicate it
    //

    if (!Src1) {
        return _INT_DuplicateDevicePath(BS, Src2);
    }

    if (!Src2) {
        return _INT_DuplicateDevicePath(BS, Src1);
    }

    Src1Size = _INT_DevicePathSize(Src1);
    Src1Inst = _INT_DevicePathInstanceCount(Src1);
    Src2Size = _INT_DevicePathSize(Src2);

    Size = Src1Size * Src1Inst + Src2Size;

    Dst = _INT_AllocatePool(BS, Size);

    if (Dst) {
        DstPos = (UINT8*)Dst;

        while ((Inst = _INT_DevicePathInstance(&Src1, &Size))) {

            _INT_memcpy(DstPos, Inst, Size);
            DstPos += Size;

            _INT_memcpy(DstPos, Src2, Src2Size);
            DstPos += Src2Size;

            _INT_memcpy(DstPos, EndInstanceDevicePath, sizeof(EFI_DEVICE_PATH));
            DstPos += sizeof(EFI_DEVICE_PATH);
        }

        // Change last end marker
        DstPos -= sizeof(EFI_DEVICE_PATH);
        _INT_memcpy(DstPos, EndDevicePath, sizeof(EFI_DEVICE_PATH));
    }

    return Dst;
}

EFI_DEVICE_PATH* _INT_FileDevicePath(EFI_BOOT_SERVICES* BS, EFI_HANDLE Device, CHAR16* FileName) {
    UINTN                   Size;
    FILEPATH_DEVICE_PATH    *FilePath;
    EFI_DEVICE_PATH         *Eop, *DevicePath;

    DevicePath = NULL;

    Size = _INT_wcslen(FileName);
    FilePath = _INT_AllocatePool(BS, Size + SIZE_OF_FILEPATH_DEVICE_PATH + sizeof(EFI_DEVICE_PATH));

    if (FilePath != NULL) {
        _INT_memset(FilePath, 0, Size + SIZE_OF_FILEPATH_DEVICE_PATH + sizeof(EFI_DEVICE_PATH));

        if (FilePath) {

            FilePath->Header.Type = MEDIA_DEVICE_PATH;
            FilePath->Header.SubType = MEDIA_FILEPATH_DP;

            SetDevicePathNodeLength (&FilePath->Header, Size + SIZE_OF_FILEPATH_DEVICE_PATH);
            _INT_memcpy(FilePath->PathName, FileName, Size);

            Eop = NextDevicePathNode(&FilePath->Header);
            SetDevicePathEndNode(Eop);

            DevicePath = (EFI_DEVICE_PATH *) FilePath;

            if (Device) {
                DevicePath = _INT_AppendDevicePath(
                    BS,
                    _INT_DevicePathFromHandle(BS, Device),
                    DevicePath
                );

                _INT_FreePool(BS, FilePath);
            }
        }
    }

    return DevicePath;
}
