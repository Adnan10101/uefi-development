
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>

EFI_STATUS
EFIAPI
UefiMain (
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
    )
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    UINTN EventIndex;

    Print(L"============================\n");
    Print(L"       MY FIRST GAME        \n");
    Print(L"============================\n\n");

    Print(L"Press any key to continue...\n");

    Status = gBS->WaitForEvent(
        1,
        &gST->ConIn->WaitForKey,
        &EventIndex
    );

    if (!EFI_ERROR(Status)) {
        Status = gST->ConIn->ReadKeyStroke(
            gST->ConIn,
            &Key
        );

        if (!EFI_ERROR(Status)) {
            Print(L"\nYou pressed a key!\n");
            Print(L"Welcome to MyGame.\n");
        }
    }

    int a[] = {1,2,3,4,5};
    int *addr = a;
    int i = 0;

    for(i=0;i<5;i++){
        Print(L"index %d = %d\n",i,a[i]);
        Print(L"addr %d = %p\n",i,(addr + i));
    }
    


    gBS->Stall(500000000);
    return EFI_SUCCESS;
}