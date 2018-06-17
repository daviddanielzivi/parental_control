#include <stdint.h>
#include <Windows.h>
#include <stdio.h>
#include "Shlwapi.h"

#include "commands.h"
#include "DebugPrint.h"
#include "AgentManager.h"


#define PIPE_NAME               "\\\\.\\pipe\\parental_control_pipe"
#define SERVICE_NAME            "Parental Control Service"



SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);



static CAgentManager s_agent_manager;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SetWorkingPathToRunningDir()
{
    char file_path[MAX_PATH];
    int bytes = GetModuleFileName(NULL, file_path, MAX_PATH); // Get the full path of current exe file
    PathRemoveFileSpec(file_path);                            // Strip the exe filename from path and get folder name
    SetCurrentDirectory(file_path);                           // Set the current working directory
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    //don't want to work under \\windows\\system32\\ folder
    SetWorkingPathToRunningDir();

    DebugPrintInit();

    DebugPrint("Parental Control Service: Main: Entry");

    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
    {
        DebugPrint("Parental Control Service: Main: StartServiceCtrlDispatcher returned error");
        return GetLastError();
    }

    DebugPrint("Parental Control Service: Main: Exit");
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
    DWORD Status = E_FAIL;

    DebugPrint("Parental Control Service: ServiceMain: Entry");

    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL)
    {
        DebugPrint("Parental Control Service: ServiceMain: RegisterServiceCtrlHandler returned error");
        goto EXIT;
    }

    // Tell the service controller that we are starting
    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        DebugPrint("Parental Control Service: ServiceMain: SetServiceStatus returned error");
    }

    /*
    * Perform tasks necessary to start the service here
    */
    DebugPrint("Parental Control Service: ServiceMain: Performing Service Start Operations");

    // Create stop event to wait on later.
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    if (g_ServiceStopEvent == NULL)
    {
        DebugPrint("Parental Control Service: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error");

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            DebugPrint("Parental Control Service: ServiceMain: SetServiceStatus returned error");
        }
        goto EXIT;
    }

    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        DebugPrint("Parental Control Service: ServiceMain: SetServiceStatus returned error");
    }

    // Start the thread that will perform the main task of the service
    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    DebugPrint("Parental Control Service: ServiceMain: Waiting for Worker Thread to complete");

    // Wait until our worker thread exits effectively signaling that the service needs to stop
    WaitForSingleObject(hThread, INFINITE);

    DebugPrint("Parental Control Service: ServiceMain: Worker Thread Stop Event signaled");

    DebugPrint("Parental Control Service: ServiceMain: Performing Cleanup Operations");

    CloseHandle(g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        DebugPrint("Parental Control Service: ServiceMain: SetServiceStatus returned error");
    }

EXIT:
    DebugPrint("Parental Control Service: ServiceMain: Exit");
    DebugPrintShutdown();
    return;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
    DebugPrint("Parental Control Service: ServiceCtrlHandler control code %d", CtrlCode);

    switch (CtrlCode)
    {
    case SERVICE_CONTROL_STOP:

        DebugPrint("Parental Control Service: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request");

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            break;


        // Perform tasks necessary to stop the service here
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            DebugPrint("Parental Control Service: ServiceCtrlHandler: SetServiceStatus returned error");
        }

        // This will signal the worker thread to start shutting down
        SetEvent(g_ServiceStopEvent);

        break;

    default:
        break;
    }
    
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    HANDLE hPipe = NULL;
    HANDLE io_event = NULL;
    uint32_t counter = 0;
    bool status;
    bool connection_request = false;
    OVERLAPPED ol;

    DebugPrint("Parental Control Service: ServiceWorkerThread: Entry");

    SECURITY_ATTRIBUTES pSecAttrib;
    SECURITY_DESCRIPTOR* pSecDesc;
    
    pSecDesc = (SECURITY_DESCRIPTOR*)LocalAlloc(LPTR,SECURITY_DESCRIPTOR_MIN_LENGTH);
    InitializeSecurityDescriptor(pSecDesc,SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(pSecDesc, TRUE, (PACL)NULL, FALSE);
    pSecAttrib.nLength = sizeof(SECURITY_ATTRIBUTES);
    pSecAttrib.bInheritHandle = TRUE;
    pSecAttrib.lpSecurityDescriptor = pSecDesc;

    
    hPipe = CreateNamedPipe(
        PIPE_NAME,                // pipe name 
        PIPE_ACCESS_DUPLEX |      // two way pipe
        FILE_FLAG_OVERLAPPED,	  // don't block on read/write/open
        PIPE_TYPE_BYTE |          // message type pipe 
        PIPE_READMODE_BYTE |      // message-read mode 
        PIPE_WAIT,                // non blocking mode 
        PIPE_UNLIMITED_INSTANCES, // max. instances  
        512,                      // output buffer size 
        512,                      // input buffer size 
        NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
        &pSecAttrib);              // no security attribute 

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        DWORD dwError = ::GetLastError();
        DebugPrint("Parental Control Service: Failed to create named pipe");
        return ERROR_SUCCESS;
    }
    else
    {
        DebugPrint("Parental Control Service: Succeed to create named pipe");
    }

    io_event = CreateEvent(NULL, FALSE, FALSE, NULL);

    // periodically check if the service has been requested to stop
    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {
        ZeroMemory(&ol, sizeof(ol));
        ol.hEvent = io_event;

        // periodically check if new connection is required 
        status = ConnectNamedPipe(hPipe, &ol);

        if (status == 0)
        {
            switch (GetLastError())
            {
            case ERROR_PIPE_CONNECTED:
                connection_request = true;
                DebugPrint("PIPE CONNECTED");
                break;

            case ERROR_IO_PENDING:
                //if pending wait for timeout 
                if (WaitForSingleObject(ol.hEvent, 1000) == WAIT_OBJECT_0)
                {
                    unsigned long dwIgnore;
                    connection_request = GetOverlappedResult(hPipe, &ol, &dwIgnore, false);
                    DebugPrint("PIPE IO PENDING");
                }
                else
                {
                    CancelIo(hPipe);
                }
                break;
            }
        }

        if (connection_request == true)
        {
            //start a new agent monitor for the new connection 
            s_agent_manager.StartNewAgent(hPipe);

            //prepare next connection 
            hPipe = CreateNamedPipe(
                PIPE_NAME,                // pipe name 
                PIPE_ACCESS_DUPLEX |      // two way pipe
                FILE_FLAG_OVERLAPPED,	  // don't block on read/write/open
                PIPE_TYPE_BYTE |          // message type pipe 
                PIPE_READMODE_BYTE |      // message-read mode 
                PIPE_WAIT,                // non blocking mode 
                PIPE_UNLIMITED_INSTANCES, // max. instances  
                512,                      // output buffer size 
                512,                      // input buffer size 
                NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
                0);                       // no security attribute 
            connection_request = false;

            //before opening a new connection try to release expired ones
            s_agent_manager.ReleaseExpiredAgents();
        }
        Sleep(1000);
    }

    s_agent_manager.StopAllAgents();
    CloseHandle(io_event);
    return ERROR_SUCCESS;
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
