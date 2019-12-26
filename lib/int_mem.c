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

VOID _INT_FreePool(EFI_BOOT_SERVICES* BS, VOID *Buffer)
{
    uefi_call_wrapper(BS->FreePool, 1, Buffer);
}

VOID _INT_memcpy(CHAR8* Dst, CHAR8* Src, UINTN Size)
{
    for (UINTN i = 0; i < Size; Dst[i] = Src[i], i++);
}

VOID _INT_memset(CHAR8* Dst, CHAR8 Val, UINTN Size)
{
    for (UINTN i = 0; i < Size; Dst[i] = Val, i++);
}

UINTN _INT_strlen16(CHAR8* str)
{
    UINTN Size = 0;

    for (; ((CHAR16*)str)[Size]; Size++);
    
    return (Size + 1) * sizeof(CHAR16);
}
