#include <efi.h>
#include "include/int_graphics.h"
#include "include/int_print.h"
#include "include/int_event.h"
#include "include/int_guid.h"
#include "include/int_dpath.h"


#define APPLE_SET_OS_VENDOR  "Apple Inc."
#define APPLE_SET_OS_VERSION "Mac OS X 10.15"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_STATUS Status;
    SIMPLE_TEXT_OUTPUT_INTERFACE* ConOut = SystemTable->ConOut;
    SIMPLE_INPUT_INTERFACE* ConIn = SystemTable->ConIn;

    _INT_SetGraphicsMode(SystemTable->BootServices, FALSE);

    _INT_Clear(ConOut);
    _INT_IPrint(ConOut, L"========== apple_set_os loader v0.2 ==========\r\n");


    EFI_HANDLE* handle_buffer;
    UINTN handle_count;

    _INT_IPrint(ConOut, L"Initializing SetOsProtocol\r\n");
    EFI_GUID apple_set_os_guid = APPLE_SET_OS_GUID;
    Status = SystemTable->BootServices->LocateHandleBuffer(
        ByProtocol, 
        &apple_set_os_guid, 
        NULL, 
        &handle_count,
        &handle_buffer
    );
    
    if (EFI_ERROR(Status)) {
        _INT_IPrint(ConOut, L"SetOsProtocol Buffer Error: %lX\r\n", (UINTN)Status);
        goto restart;
    } else if (handle_count == 0) {
        _INT_IPrint(ConOut, L"No SetOsProtocol Handles\r\n");
        goto restart;
    } else {
        _INT_IPrint(ConOut, L"SetOsProtocol Handle Count: %d\r\n", (UINTN)handle_count);
        for(UINTN i = 0; i < handle_count; i++) {
            EFI_APPLE_SET_OS_IFACE* SetOsIface = NULL;

            Status = SystemTable->BootServices->OpenProtocol(
                handle_buffer[i],
                &apple_set_os_guid,
                (VOID**)&SetOsIface,
                ImageHandle,
                NULL,
                EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
            );

            if (EFI_ERROR(Status)) {
                _INT_IPrint(ConOut, L"SetOsProtocol Error: %d\r\n", (UINTN)Status);
            } else {
                if (SetOsIface->Version != 0){
                    _INT_IPrint(ConOut, L"Setting OsVendor\r\n");
                    Status = SetOsIface->SetOsVendor((CHAR8*) APPLE_SET_OS_VENDOR);
                    if (EFI_ERROR(Status)){
                        _INT_IPrint(ConOut, L"OsVendor Error: %lX\r\n", (UINTN)Status);
                    }

                    _INT_IPrint(ConOut, L"Setting OsVersion\r\n");
                    Status = SetOsIface->SetOsVersion((CHAR8*) APPLE_SET_OS_VERSION);
                    if (EFI_ERROR(Status)){
                        _INT_IPrint(ConOut, L"OsVersion Error: %lX\r\n", (UINTN)Status);
                    }
                }
            }
        }
    }



    EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
	EFI_DEVICE_PATH* DevicePath = NULL;
	EFI_HANDLE DriverHandle;

    _INT_IPrint(ConOut, L"Initializing LoadedImageProtocol...\r\n");
    EFI_GUID efi_loaded_image_protocol_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    Status = SystemTable->BootServices->HandleProtocol(ImageHandle, &efi_loaded_image_protocol_guid, (VOID**) &LoadedImage);
    if(EFI_ERROR(Status) || LoadedImage == NULL) {
        goto halt;
    }

    _INT_IPrint(ConOut, L"Locating bootx64_original.efi...\r\n");
    DevicePath = _INT_FileDevicePath(
        SystemTable->BootServices, 
        LoadedImage->DeviceHandle, 
        L"\\EFI\\Boot\\bootx64_original.efi"
    );

    if (DevicePath == NULL) {
        _INT_IPrint(ConOut, L"Unable to find bootx64_original.efi.\r\n");
        goto halt;
	}


    _INT_IPrint(ConOut, L"Loading bootx64_original.efi to memory...\r\n");
    // Attempt to load the driver.
	Status = SystemTable->BootServices->LoadImage(FALSE, ImageHandle, DevicePath, NULL, 0, &DriverHandle);

    _INT_FreePool(SystemTable->BootServices, DevicePath);
    DevicePath = NULL;

	if (EFI_ERROR(Status)) {
        _INT_IPrint(ConOut, L"Unable to load bootx64_original.efi to memory.\r\n");
		goto halt;
	}

    _INT_IPrint(ConOut, L"Prepare to run bootx64_original.efi...\r\n");

	Status = SystemTable->BootServices->OpenProtocol(
        DriverHandle, 
        &efi_loaded_image_protocol_guid,
		(VOID**)&LoadedImage, 
        ImageHandle, 
        NULL, 
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
	if (EFI_ERROR(Status)) {
        _INT_IPrint(ConOut, L"Failed to run bootx64_original.efi.\r\n");
		goto halt;
	}

    _INT_IPrint(ConOut, L"apple_set_os loaded.\r\nPlug in your eGPU then press any key.\r\n");

    EFI_INPUT_KEY Key;

    for (UINT8 i = 5; i > 0; i--) {
        _INT_IPrint(ConOut, L"bootx64_original.efi will start in %u second(s)...\r\n", (UINT32)i);
        for (UINT16 j = 0; j < 1000; j++) {
            _INT_WaitForSingleEvent(SystemTable->BootServices, ConIn->WaitForKey, 10000);
            if (!EFI_ERROR(ConIn->ReadKeyStroke(ConIn, &Key))) {
                goto run;
            }
        }
    }

run:
    _INT_IPrint(ConOut, L"Starting bootx64_original.efi...\r\n");
    for (UINT16 j = 0; j < 1000; j++) {
        _INT_WaitForSingleEvent(SystemTable->BootServices, ConIn->WaitForKey, 10000);
    }
    ConOut->ClearScreen(ConOut);
    _INT_SetGraphicsMode(SystemTable->BootServices, TRUE);

    // Load was a success - attempt to start the driver
    Status = SystemTable->BootServices->StartImage(DriverHandle, NULL, NULL);
    if (EFI_ERROR(Status)) {
        _INT_IPrint(ConOut, L"Unable to start bootx64_original.efi.\r\n");
        goto halt;
    }

halt:
    while (1) { }
    
    return EFI_SUCCESS;

restart:
        _INT_IPrint(ConOut, L"SetOsProtocol is not loaded, restarting...\r\n");
        for (UINT16 j = 0; j < 250; j++) {
            _INT_WaitForSingleEvent(SystemTable->BootServices, ConIn->WaitForKey, 10000);
        }
        return SystemTable->RuntimeServices->ResetSystem(
            EfiResetWarm,
            EFI_SUCCESS,
            sizeof(NULL),
            NULL
        );
}
