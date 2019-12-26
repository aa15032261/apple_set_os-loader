#include "../include/int_dpath.h"

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
        _INT_memcpy((CHAR8*)NewDevPath, (CHAR8*)DevPath, Size);
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

            _INT_memcpy(DstPos, (CHAR8*)Inst, Size);
            DstPos += Size;

            _INT_memcpy(DstPos, (CHAR8*)Src2, Src2Size);
            DstPos += Src2Size;

            _INT_memcpy(DstPos, (CHAR8*)EndInstanceDevicePath, sizeof(EFI_DEVICE_PATH));
            DstPos += sizeof(EFI_DEVICE_PATH);
        }

        // Change last end marker
        DstPos -= sizeof(EFI_DEVICE_PATH);
        _INT_memcpy(DstPos, (CHAR8*)EndDevicePath, sizeof(EFI_DEVICE_PATH));
    }

    return Dst;
}
