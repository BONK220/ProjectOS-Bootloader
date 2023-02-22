#include "efi.h"
#include "ErrorCodes.h"
#include "efilibs.h"

CHAR16* EFI_MEMORY_TYPE_STRINGS[16] = 
{
    L"EfiReservedMemoryType",
    L"EfiLoaderCode",
    L"EfiLoaderData",
    L"EfiBootServiceCode",
    L"EfiBootServiceData",
    L"EfiRuntimeServiceCode",
    L"EfiRuntimeServiceData",
    L"EfiConventionalMemory",
    L"EfiUnusableMemory",
    L"EfiACPIReclaimMemory",
    L"EfiACPIMemoryNVS",
    L"EfiMemoryMappedIO",
    L"EfiMemoryMappedIOPortSpace",
    L"EfiPalCode",
    L"EfiPersistentMemory",
    L"EfiMaxMemoryType",
};

// This is like int main() in a typical C program.
// In this case, we create an ImageHandle for the overall EFI interface,
// as well as a System Table pointer to the EFI_SYSTEM_TABLE struct.
// UEFI 2.9 Specs PDF Page 91
EFI_STATUS efi_main(EFI_HANDLE IM, EFI_SYSTEM_TABLE *ST)
{
    // We setup this global variable in the libs.h file.
    ImageHandle = IM;
    SystemTable = ST;
    
    ResetScreen();
    
    SetColor(EFI_WHITE);
    SetTextPosition(10, 2);
    Print(L"ProjectOS Boot Manager V1.0a (PBM)\r\n\r\n");

    Timeout(500); // 500ms timeout

    SetColor(EFI_YELLOW);
    Print(L"INFO: Loading Graphics Output Protocol... ");
    EFI_STATUS status = EnableGraphics();
    if(status == EFI_SUCCESS)
    {
        SetColor(EFI_CYAN);
    } else {
        SetColor(EFI_RED);
        Print(CheckStandardEFIError(status));
        TimeoutSecs(6);
        COLD_REBOOT();
    }

    Print(CheckStandardEFIError(status));
    SetColor(EFI_YELLOW);

    SetColor(EFI_YELLOW);
    Print(L"Enabling keyboard... ");
    ResetKeyboard();
    SetColor(EFI_CYAN);
    Print(L"Successful!\r\n");
    SetColor(EFI_YELLOW);
    Print(L"INFO: Keyboard set up succesfully\r\n");

    Print(L"Initializing filesystem...");
    InitializeFilesystem();

    // memory map
    SetColor(EFI_BLUE);
    TimeoutSecs(5);
    ClearScreen();
    
    for(int i = 0; i <= 20; i++) {
        Print(L"*");
    }
    Print(L"Memory map");
    for(int i = 0; i <= 20; i++) {
        Print(L"*");
    }
    Print(L"\r\n\r\n");

    // last seq
    SetColor(EFI_BLUE);
    TimeoutSecs(5);
    ClearScreen();

    Print(L"Hit \'q\' to exit or \'r\' to reboot");

    while(1)
    {
        EFI_STATUS status = CheckKey();
        if(status == EFI_SUCCESS) {
            if(GetKey('q') == 1) {
                SHUTDOWN();
                break;
            }
            if(GetKey('r') == 1) {
                COLD_REBOOT();
            }
        }
    }

    // The EFI needs to have a 0 ( or EFI_SUCCESS ) in order to know everything is ok.
    return EFI_SUCCESS;
}