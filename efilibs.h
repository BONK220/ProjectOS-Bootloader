#ifndef EFILIBS_H
#define EFILIBS_H

#include "efi.h"
#include "ErrorCodes.h"
#include "clibs.h"
#include "posdfont.h"

#define NULL (void*)0

             // ARGB             
#define ORANGE 0xffffa500
#define CYAN   0xff00ffff
#define RED    0xffff0000
#define GREEN  0xff00ff00
#define BLUE   0xff0000ff
#define GRAY   0xff888888
#define WHITE  0xffffffff
#define BLACK  0xff000000

EFI_HANDLE ImageHandle;
EFI_SYSTEM_TABLE* SystemTable;
EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
EFI_GRAPHICS_OUTPUT_BLT_PIXEL GraphicsColor;
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
void* OSBuffer_Handle;

typedef struct PIXELPOSITIONS
{
    UINT32 PixelxPos;
    UINT32 PixelyPos;
} PIXELPOSITIONS;

PIXELPOSITIONS* pixelpos;

void SetTextPosition(UINT32 col, UINT32 row) {
    // Sets the column and the row of the text pos on screen
    SystemTable->ConOut->SetCursorPosition(SystemTable->ConOut, col, row);
}

void ClearScreen() {
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
}

void SetCursorVisible(int val) {
    SystemTable->ConOut->EnableCursor(SystemTable->ConOut, val);
}

void ResetScreen()
{
    // This resets the whole console ( A.K.A. your display screen ) interface.
    SystemTable->ConOut->Reset(SystemTable->ConOut, 1);
}

void SetColor(UINTN Attribute)
{
    // We set the color for the text output.
    SystemTable->ConOut->SetAttribute(SystemTable->ConOut, Attribute);
}

void Print(CHAR16* str)
{
    // Text Output  ( A.K.A. ConOut is Console Out )
    SystemTable->ConOut->OutputString(SystemTable->ConOut, str);
}

void HitAnyKey()
{
    // This clears the keyboard buffer.
    SystemTable->ConIn->Reset(SystemTable->ConIn, 1);

    // We setup the struct to take keyboard input.
    EFI_INPUT_KEY Key;

    // In a while loop to see if the keyboard has a key stroke in the buffer.
    while((SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key)) == EFI_NOT_READY);
}

// This sets the color of the pixels ( Graphics Color )
void SetGraphicsColor(UINT32 color)
{
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL GColor;
    GColor.Reserved = color >> 24;
    GColor.Red      = color >> 16;
    GColor.Green    = color >> 8;
    GColor.Blue     = color;
    GraphicsColor   = GColor;
}

UINT32 ScreenWidth;
UINT32 ScreenHeight;

EFI_STATUS EnableGraphics() {
    EFI_STATUS status = SystemTable->BootServices->LocateProtocol(&EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID, 0, (void**)&gop);
    ScreenWidth = gop->Mode->Info->HorizontalResolution;
    ScreenHeight = gop->Mode->Info->VerticalResolution;
    return status;
}

// This positions the pixel in the row and column ( X and Y )
void SetPixel(UINT32 xPos, UINT32 yPos)
{
    // TODO : Add in a choice instead of defaulting to EfiBltVideoFill.
    gop->Blt(gop, &GraphicsColor, EfiBltVideoFill, 0, 0, xPos, yPos, 1, 1, 0);
}

// This creates a filled box of pixels.
// NOTE : Call SetGraphicsColor prior to using this function.
void CreateFilledBox(UINT32 xPos, UINT32 yPos, UINT32 w, UINT32 h)
{
    // TODO: Add in a choice instead of defaulting to EfiBltVideoFill.
    gop->Blt(gop, &GraphicsColor, EfiBltVideoFill, 0, 0, xPos, yPos, w, h, 0);
}

void Timeout(UINTN miliseconds) {
    SystemTable->BootServices->Stall(miliseconds * 1000);
}

void TimeoutSecs(UINTN secs) {
    SystemTable->BootServices->Stall(secs * (1000 * 1000));
}

void COLD_REBOOT() {
    // Hardware reboot
    SystemTable->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, 0);
}

void WARM_REBOOT() {
    // Software reboot
    SystemTable->RuntimeServices->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, 0);
}

void SHUTDOWN() {
    // Shutdown
    SystemTable->RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, 0);
}

void ResetKeyboard() {
    // Reset keyboard
    SystemTable->ConIn->Reset(SystemTable->ConIn, 1);
}

EFI_INPUT_KEY CheckKeyStroke;
BOOLEAN GetKey(CHAR16 key) {
    if(CheckKeyStroke.UnicodeChar == key) {
        return 1;
    } else {
        return 0;
    }
}

EFI_STATUS CheckKey() {
    return SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &CheckKeyStroke);
}

EFI_STATUS InitializeFilesystem() { 
    EFI_STATUS status;
    SetColor(EFI_BROWN);
    Print(L"Loaded Image...");
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
    status = SystemTable->BootServices->HandleProtocol(ImageHandle, &EFI_LOADED_IMAGE_PROTOCOL_GUID, (void**)&LoadedImage);
    SetColor(EFI_CYAN);
    Print(CheckStandardEFIError(status));

    SetColor(EFI_BROWN);
    Print(L"DevicePath... ");
    EFI_DEVICE_PATH_PROTOCOL *DevicePath;
    status = SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &EFI_DEVICE_PATH_PROTOCOL_GUID, (void**)&DevicePath);
    SetColor(EFI_CYAN);
    Print(CheckStandardEFIError(status));
    
    SetColor(EFI_BROWN);
    Print(L"Volume... ");
    status = SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID, (void**)&Volume);
    SetColor(EFI_CYAN);
    Print(CheckStandardEFIError(status));
    return status;
}

EFI_FILE_PROTOCOL* openFile(CHAR16* FileName)
{
    // This opens a file from the EFI FAT32 file system volume.
    // It loads from root, so you must supply full path if the file is not in the root.
    // Example : "somefolder//myfile"  <--- Notice the double forward slash.
    EFI_STATUS status;
    SetColor(EFI_BROWN);
    Print(L"RootFS... ");
    EFI_FILE_PROTOCOL* RootFS;
    status = Volume->OpenVolume(Volume, &RootFS);
    if(status == EFI_SUCCESS) {
        SetColor(EFI_CYAN);
    } else {
        SetColor(EFI_RED);
    }
    Print(CheckStandardEFIError(status));
    SetColor(EFI_CYAN);
    
    SetColor(EFI_BROWN);
    Print(L"Opening File... ");
    EFI_FILE_PROTOCOL* FileHandle = NULL;
    status = RootFS->Open(RootFS, &FileHandle, FileName, 0x0000000000000001, 0);
    if(status == EFI_SUCCESS) {
        SetColor(EFI_CYAN);
    } else {
        SetColor(EFI_RED);
    }

    Print(CheckStandardEFIError(status));
    
    return FileHandle;
}

void closeFile(EFI_FILE_PROTOCOL* FileHandle)
{
    // This closes the file.
    EFI_STATUS status;
    SetColor(EFI_BROWN);
    Print(L"Closing File ... ");
    status = FileHandle->Close(FileHandle);
    SetColor(EFI_CYAN);
    Print(CheckStandardEFIError(status));
}


// We create the buffer, allocate memory for it, then read
// the rile into the buffer. After which, we close the file.
// Currently we are using a fixed size. Eventually we will fix that.
// Currently we have a fixed Buffer Handle as well. Eventually we will fixe that.
void readFile(CHAR16* filename) {
    EFI_FILE_PROTOCOL* FileHandle = openFile(filename);
    if(FileHandle != NULL) {
        SetColor(EFI_YELLOW);
        Print(L"Allocating pool...");
        EFI_STATUS status = SystemTable->BootServices->AllocatePool(EfiLoaderData, 0x00100000, (void**)&OSBuffer_Handle);
        if(status == EFI_SUCCESS) SetColor(EFI_CYAN);
        else SetColor(EFI_RED);
        Print(CheckStandardEFIError(status));
        SetColor(EFI_YELLOW);

        UINT64 filesize = 0x00100000;
        Print(L"Reading file...");
        status = FileHandle->Read(FileHandle, &filesize, OSBuffer_Handle);
        if(status == EFI_SUCCESS) SetColor(EFI_CYAN);
        else SetColor(EFI_RED);
        Print(CheckStandardEFIError(status));
        closeFile(FileHandle);

        UINT8* test = (UINT8*)OSBuffer_Handle;

        SetColor(EFI_WHITE);
        for(int m = 0; m < 20; m++) {
            UINT8 q = *test;
            UINT16 s[2];
            itoa(q, s, HEX);
            Print(s);
            Print(L" ");
            test++;
        }
        SetColor(EFI_YELLOW);
    }
}

void GetCharacter(UINT32 character, UINT32 xPos, UINT32 yPos, UINT32 fs /*Font size not filesystem :)*/) {
    pixelpos->PixelxPos = xPos;
    pixelpos->PixelyPos = yPos;
    UINT32 SelectASCII = character;

    UINT32 mcX = pixelpos->PixelxPos;
    UINT32 mcY = pixelpos->PixelyPos;
    UINT32 pPos = 0;
    UINT32 sPos = 0;
    UINT32 FontSize = (UINT32)(fs / 4);

    for(UINT32 t = (SelectASCII * 128); t < ((SelectASCII * 128) + 128); t++) {
        if(pPos > 7) {
            pPos = 0;
            mcY += FontSize;
            mcX = pixelpos->PixelxPos;
            sPos++;
            if(sPos > 15) {
                sPos = 0;
                mcY += FontSize;
            }
        }
        UINT32 ASCIIColor = asciifont[t];
        switch (ASCIIColor)
        {
            case 0: 
            {
                break;
            }

            case 1:
            {
                CreateFilledBox(mcX, mcY, FontSize, FontSize);
            }
        }
        mcX += FontSize;
        pPos++;
    }
}

#endif // EFILIBS_H