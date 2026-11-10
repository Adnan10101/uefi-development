#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#define BUFFER_SIZE 128

// gST - global system table: provides access to things such as console, firmare config, table etc
// gBS - global boot service: provides functinos that firmware makes available, use it for waiting for keyboard events, allocating memory etc

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
){
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    
    UINTN EventIndex;
    CHAR16 Buffer[BUFFER_SIZE];
    UINTN Length = 0;

    Print(L"Start of text type.....\n");

    while(1){
        Status = gBS->WaitForEvent(
            1,
            &gST->ConIn->WaitForKey,
            &EventIndex
        );

        if (EFI_ERROR(Status)) {
            return Status;
        }

        Status = gST->ConIn->ReadKeyStroke(
            gST->ConIn,
            &Key
        );

        if (EFI_ERROR(Status)) {
            continue;
        }

        if (Key.UnicodeChar == L'\r') {
            break;
        }

        if (Key.UnicodeChar == CHAR_BACKSPACE){
            if (Length > 0) {
                Length--;
                Buffer[Length] = L'\0';
                Print(L"\b");
            }
            continue;
        }

        Buffer[Length] = Key.UnicodeChar;
        Length++;

        Print(L"%c",Key.UnicodeChar);
    }
    Buffer[Length] = L'\0';
    Print(L"\r\nYou typed...... %s\r",Buffer);

    Print(L"\nPress any key to quit...\r");

    Status = gBS->WaitForEvent(
        1,
        &gST->ConIn->WaitForKey,
        &EventIndex
    );

    if (!EFI_ERROR(Status)) {
        gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
    }


    //gBS->Stall(500000000);
    return EFI_SUCCESS;

}