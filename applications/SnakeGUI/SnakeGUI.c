#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Protocol/GraphicsOutput.h>

#define BOARD_WIDTH  66
#define BOARD_HEIGHT 33
#define CELL_SIZE    3
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
VOID RenderBoard(GameState *Game, EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop);
VOID Box(EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop, EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Color, UINTN X, UINTN Y, UINTN Width, UINTN Height, UINTN Thickness);
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

    

    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;

    gBS->LocateProtocol(
        &gEfiGraphicsOutputProtocolGuid,
        NULL,
        (VOID **)&Gop
    );

    // test
    UINT32 Width;
    UINT32 Height;

    Width = Gop->Mode->Info->HorizontalResolution;
    Height = Gop->Mode->Info->VerticalResolution;

    Print(L"Resolution: %u x %u\n", Width, Height);

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL green_color = {0,255,0,0};

    Gop->Blt(
        Gop, &green_color,             
        EfiBltVideoFill,    
        0,0,10,10,20,100,0
    );
    
    // Wait for the start key
    Status = gBS->WaitForEvent(
        1,
        &gST->ConIn->WaitForKey,
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
    RenderBoard(&Game, Gop);
    //RenderBoard(&Game, Gop);

    
    
    //test

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
                RenderBoard(&Game, Gop);
                //Box(Gop, &color, 1,1,BOARD_WIDTH,BOARD_HEIGHT,3);
                
            }
        }
    }

    // Clean up timer
    gBS->SetTimer(TimerEvent, TimerCancel, 0);
    gBS->CloseEvent(TimerEvent);

    
    Print(L"Suck balls loser!");
    gBS->Stall(500000000);

    return EFI_SUCCESS;
}

VOID Box(EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop, 
        EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Color,
        UINTN X,
        UINTN Y,
        UINTN Width,
        UINTN Height,
        UINTN Thickness
    ){

        
    // Top
    Gop->Blt(Gop, Color, EfiBltVideoFill,
             0, 0, X, Y, Width, Thickness, 0);

    // Bottom
    Gop->Blt(Gop, Color, EfiBltVideoFill,
             0, 0, X, Y + Height - Thickness,
             Width, Thickness, 0);

    // Left
    Gop->Blt(Gop, Color, EfiBltVideoFill,
             0, 0, X, Y, Thickness, Height, 0);

    // Right
    Gop->Blt(Gop, Color, EfiBltVideoFill,
             0, 0, X + Width - Thickness, Y ,
             Thickness, Height, 0);
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

VOID
DrawCell(
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop,
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Color,
    UINTN GridX,
    UINTN GridY
)
{
    Gop->Blt(
        Gop,
        Color,
        EfiBltVideoFill,
        0, 0,
        GridX * CELL_SIZE,
        GridY * CELL_SIZE,
        CELL_SIZE,
        CELL_SIZE,
        0
    );
}

VOID RenderBoard(GameState *Game, EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop)
{
    //clears the board
    
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Black = {
        .Blue = 0,
        .Green = 0,
        .Red = 0,
        .Reserved = 0
    };

    Gop->Blt(
        Gop,
        &Black,
        EfiBltVideoFill,
        0, 0,
        0, 0,
        BOARD_WIDTH * 3,
        BOARD_HEIGHT * 3,
        0
    );

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Green = {
        .Blue = 0,
        .Green = 255,
        .Red = 0,
        .Reserved = 0
    };

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Red = {
        .Blue = 0,
        .Green = 0,
        .Red = 255,
        .Reserved = 0
    };
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Yellow = {
        .Blue = 0,
        .Green = 255,
        .Red = 255,
        .Reserved = 0
    };

    Box(Gop, &Yellow, 1,1,BOARD_WIDTH * CELL_SIZE,BOARD_HEIGHT * CELL_SIZE,3);
    
    UINTN i;

    // Draw each snake segment
    for (i = 0; i < Game->SnakeLength; i++) {
        DrawCell(
            Gop,
            &Green,
            Game->Segment[i].X,
            Game->Segment[i].Y
        );
    }

    // Draw the food
    DrawCell(
        Gop,
        &Red,
        Game->Food.X,
        Game->Food.Y
    );
    
    gST->ConOut->SetCursorPosition(
        gST->ConOut,
        0, 0
    );

    Print(L"Arrow keys: move | Q: quit\n");
    
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
    Position NextHead = Game->Segment[0];

    // Calculate next head position
    switch (Game->DIR) {
        case dir_up:
            NextHead.Y--;
            break;

        case dir_down:
            NextHead.Y++;
            break;

        case dir_left:
            NextHead.X--;
            break;

        case dir_right:
            NextHead.X++;
            break;
    }

    // Check wall collision
    if (NextHead.X == 0 ||
        NextHead.X >= (BOARD_WIDTH - 1) ||
        NextHead.Y == 0 ||
        NextHead.Y >= BOARD_HEIGHT - 1) {
        Game->Running = FALSE;
        return;
    }

    // Check if food will be eaten
    BOOLEAN AteFood =
        (NextHead.X == Game->Food.X &&
         NextHead.Y == Game->Food.Y);

    if (AteFood) {
        if (Game->SnakeLength >= MAX_LENGTH) {
            Game->Running = FALSE;
            return;
        }

        Game->SnakeLength++;
    }

    // Shift body segments
    for (UINTN i = Game->SnakeLength - 1; i > 0; i--) {
        Game->Segment[i] = Game->Segment[i - 1];
    }

    // Move head
    Game->Segment[0] = NextHead;

}

