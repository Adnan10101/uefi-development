



#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 10
#define MAX_LENGTH 200

typedef struct {
    UINTN X;
    UINTN Y;
} Position;

typedef enum {
    dir_up,
    dir_down,
    dir_left,
    dir_right
} Direction;

typedef struct {
    Position Segment[MAX_LENGTH];
    Position Food;
    UINTN SnakeLength;
    Direction DIR;
    BOOLEAN Running;
} GameState;

// Function declarations
VOID InitializeGame(GameState *Game);
VOID RenderBoard(GameState *Game);
VOID ReadInput(GameState *Game);
VOID UpdateSnake(GameState *Game);
VOID GenerateFood(GameState *Game);

EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;
    EFI_EVENT TimerEvent;
    EFI_EVENT Events[2];
    UINTN EventIndex;
    GameState Game;

    gST->ConOut->ClearScreen(gST->ConOut);
    // Start screen
    Print(L"============================\r\n");
    Print(L"         SNAKE GAME         \r\n");
    Print(L"============================\r\n\r\n");
    Print(L"Press any key to start...\r\n");
    gST->ConOut->OutputString(
        gST->ConOut,
        L"\x2500"
    );

    
    // Wait for the start key
    Status = gBS->WaitForEvent(
        1,
        gST->ConIn->WaitForKey,
        &EventIndex
    );

    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Consume the start key
    Status = gST->ConIn->ReadKeyStroke(
        gST->ConIn,
        &Key
    );

    if (EFI_ERROR(Status)) {
        return Status;
    }

    // clear the screen
    gST->ConOut->ClearScreen(
        gST->ConOut
    );


    // Initialize game state
    InitializeGame(&Game);

    // Create timer event
    Status = gBS->CreateEvent(
        EVT_TIMER,
        TPL_APPLICATION,
        NULL,
        NULL,
        &TimerEvent
    );

    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Set timer to fire every 500 ms
    Status = gBS->SetTimer(
        TimerEvent,
        TimerPeriodic,
        5000000
    );

    if (EFI_ERROR(Status)) {
        gBS->CloseEvent(TimerEvent);
        return Status;
    }

    // Event 0 = keyboard, event 1 = timer
    Events[0] = gST->ConIn->WaitForKey;
    Events[1] = TimerEvent;

    // Initial render
    RenderBoard(&Game);

    // Main game loop
    while (Game.Running) {
        Status = gBS->WaitForEvent(
            2,
            Events,
            &EventIndex
        );

        if (EFI_ERROR(Status)) {
            break;
        }

        if (EventIndex == 0) {
            ReadInput(&Game);
        }
        else if (EventIndex == 1) {
            UpdateSnake(&Game);

            if (Game.Running) {
                RenderBoard(&Game);
            }
        }
    }

    // Clean up timer
    gBS->SetTimer(TimerEvent, TimerCancel, 0);
    gBS->CloseEvent(TimerEvent);

    
    Print(L"Thanks for Playing!");
    gBS->Stall(500000000);

    return EFI_SUCCESS;
}

VOID InitializeGame(GameState *Game)
{
    Game->Segment[0] = (Position){10, 5}; // Head
    // Game->Segment[1] = (Position){9, 5};
    // Game->Segment[2] = (Position){8, 5};
    // Game->Segment[3] = (Position){7, 5};  // Tail

    Game->Food.X = 12;
    Game->Food.Y = 3;

    Game->SnakeLength = 1;
    Game->DIR = dir_right;
    Game->Running = TRUE;
}

VOID RenderBoard(GameState *Game)
{
    UINTN X;
    UINTN Y;

    CHAR16 Hline[] = {
        BOXDRAW_HORIZONTAL,
        L'\0'
    };

    CHAR16 Vline[] = {
        BOXDRAW_VERTICAL,
        L'\0'
    };
    

    // Return to the top-left corner and overwrite the old frame
    gST->ConOut->SetCursorPosition(
        gST->ConOut,
        0,
        0
    );

    for (Y = 0; Y < BOARD_HEIGHT; Y++) {
        for (X = 0; X < BOARD_WIDTH; X++) {

            // Draw walls
            if (Y == 0 || Y == BOARD_HEIGHT - 1) {
                //Print(L"#");
                gST->ConOut->OutputString(
                    gST->ConOut,
                    Hline
                );
            }

            else if (X == 0 || X == BOARD_WIDTH - 1) {
                //Print(L"#");
                gST->ConOut->OutputString(
                    gST->ConOut,
                    Vline
                );
            }
            
            
            // Draw snake head
            else {
                BOOLEAN IsSnake = FALSE;
                for (UINTN i = 0; i < Game->SnakeLength; i++){
                    if (X == Game->Segment[i].X &&
                        Y == Game->Segment[i].Y) {
                        IsSnake = TRUE;
                        break;
                    }
                }
                if (IsSnake){
                    Print(L"O");
                }
                else if (X == Game->Food.X && Y == Game->Food.Y){
                    Print(L"*");
                }
                else{
                    Print(L" ");
                }
                
            }
            
        }

        Print(L"\r\n");
        
    }

    Print(L"Arrow keys: move | Q: quit     \r\n");
}

VOID ReadInput(GameState *Game)
{
    EFI_STATUS Status;
    EFI_INPUT_KEY Key;

    Status = gST->ConIn->ReadKeyStroke(
        gST->ConIn,
        &Key
    );

    if (EFI_ERROR(Status)) {
        return;
    }

    switch (Key.ScanCode) {
        case SCAN_UP:
            Game->DIR = dir_up;
            break;

        case SCAN_DOWN:
            Game->DIR = dir_down;
            break;

        case SCAN_LEFT:
            Game->DIR = dir_left;
            break;

        case SCAN_RIGHT:
            Game->DIR = dir_right;
            break;

        default:
            break;
    }

    if (Key.UnicodeChar == L'q' ||
        Key.UnicodeChar == L'Q') {
        Game->Running = FALSE;
    }
}

VOID UpdateSnake(GameState *Game)
{
    for (UINTN i = Game->SnakeLength - 1; i > 0; i--){
        Game->Segment[i] = Game->Segment[i - 1];
    }


    switch (Game->DIR) {
        case dir_up:
            if (Game->Segment[0].Y > 0) {
                Game->Segment[0].Y--;
            }
            break;

        case dir_down:
            Game->Segment[0].Y++;
            break;

        case dir_left:
            if (Game->Segment[0].X > 0) {
                Game->Segment[0].X--;
            }
            break;

        case dir_right:
            Game->Segment[0].X++;
            break;
    }

    Position nextHead; 
    nextHead = Game->Segment[0];
    

    // Detect collision with walls
    if (Game->Segment[0].X == 0 ||
        Game->Segment[0].X >= BOARD_WIDTH - 1 ||
        Game->Segment[0].Y == 0 ||
        Game->Segment[0].Y >= BOARD_HEIGHT - 1) {
        Game->Running = FALSE;
    }

    BOOLEAN ateFood;
    ateFood = (nextHead.X == Game->Food.X && nextHead.Y == Game->Food.Y);
    if (ateFood && Game->SnakeLength >= MAX_LENGTH){
        Game->Running = FALSE;
        return;
    }
    if (ateFood){
        Game->SnakeLength++;
    }
}

