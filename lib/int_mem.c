#include "../include/int_mem.h"

VOID* _INT_AllocatePool(EFI_BOOT_SERVICES* BS, UINTN Size)
{
    EFI_STATUS              Status;
    VOID                    *p;

    Status = BS->AllocatePool(EfiBootServicesData, Size, &p);
    if (EFI_ERROR(Status)) {
        p = NULL;
    }
    return p;
}

VOID* _INT_ReallocatePool(EFI_BOOT_SERVICES* BS, VOID* OldPool, UINTN OldSize, UINTN NewSize)
{
    VOID                    *NewPool;

    NewPool = NULL;
    if (NewSize) {
        NewPool = _INT_AllocatePool(BS, NewSize);
    }

    if (OldPool) {
        if (NewPool) {
            _INT_memcpy(NewPool, OldPool, OldSize < NewSize ? OldSize : NewSize);
        }
    
        _INT_FreePool(BS, OldPool);
    }
    
    return NewPool;
}

VOID _INT_FreePool(EFI_BOOT_SERVICES* BS, VOID *Buffer)
{
    BS->FreePool(Buffer);
}

VOID _INT_memcpy(VOID* Dst, VOID* Src, UINTN Size)
{
    for (UINTN i = 0; i < Size; ((CHAR8*)Dst)[i] = ((CHAR8*)Src)[i], i++);
}

VOID _INT_memset(VOID* Dst, CHAR8 Val, UINTN Size)
{
    for (UINTN i = 0; i < Size; ((CHAR8*)Dst)[i] = Val, i++);
}

UINTN _INT_wcslen(CHAR16* str)
{
    UINTN Size = 0;

    for (; str[Size]; Size++);
    
    return Size;
}
