#include <Windows.h>
#include <memory.h>

#include "statuses.h"
#include "AgentMonitor.h"
#include "UserHistory.h"
#include "DebugPrint.h"
#include "commands.h"


#define POLL_INTERVAL       60*1000
#define LOG_OFF_INTERVAL    1000


////////////////////////////////////////////////////////////////////////////
static void AgentWorkerThread(CAgentMonitor* agent_monitor_object)
{
    agent_monitor_object->Monitor();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CAgentMonitor::CAgentMonitor(HANDLE pipe_handle)
{
    m_agent_status = UNINITIALIZED;
    m_thread_id = 0;
    m_stop_monitoring = false;
    m_pipe_handle = pipe_handle;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CAgentMonitor::~CAgentMonitor()
{
    //TODO free everything

}
////////////////////////////////////////////////////////////////////////////////////////////////////
uint32_t CAgentMonitor::Init()
{
    uint32_t status = STATUS_FAIL;

    do
    {
        if (m_agent_status != UNINITIALIZED) //don't initialize twice
        {
            DebugPrint("Already initialized Bambino...");
            break;
        }
               
        m_agent_status = INITIALIZED;

        //create the monitor thread
        HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AgentWorkerThread, this, 0, &m_thread_id);

        if (hThread == NULL)
        {
            DebugPrint("Failed to create monitor thread...");
            break;
        }
        status = STATUS_OK;
    } while (0);
    return status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAgentMonitor::StopMonitoring()
{
    m_stop_monitoring = true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
AgentMonitorStatus CAgentMonitor::GetStatus()
{
    return m_agent_status;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CAgentMonitor::Monitor()
{
    uint32_t status;
    char read_buf[512];
    char write_buf[512];
    char user_name[512];

    unsigned long read_size = 0;
    unsigned long write_size = 0;
    unsigned long allowed_consumption = 0;
    unsigned long current_consumption = 0;
    Policy policy;

    do
    {
        //read policy file
        status = m_policy_reader.ParsePolicyFile(".\\");

        if (status)
        {
            DebugPrint("Failed to read policy file\n");
            break;
        }
        
        //get user name in order to check policy
        DebugPrint("Sending command: %s", COMMAND_GET_USER_NAME);
        WriteFile(m_pipe_handle, COMMAND_GET_USER_NAME, sizeof(COMMAND_GET_USER_NAME), &write_size, NULL);
        
        status = ReadFile(m_pipe_handle, user_name, sizeof(user_name), &read_size, NULL);
        
        if ((status == true) && (read_size != 0))
        {
            DebugPrint("Response: %s", user_name);
        }

        if (m_policy_reader.GetPolicy(user_name, policy) == 0)
        {
            m_agent_status = RUNNING;
            allowed_consumption = policy.total_allowed_time_minutes;
            
            CUserHistory user_history(user_name);
            current_consumption = user_history.GetCurrentTimeConsumption();

            
            while (m_stop_monitoring == false)
            {
                WriteFile(m_pipe_handle, COMMAND_PING, sizeof(COMMAND_PING), &write_size, NULL);
                status = ReadFile(m_pipe_handle, read_buf, sizeof(read_buf), &read_size, NULL);

                if ((status == false) || (read_size == 0))
                {
                    user_history.UpdateCurrentTimeConsumption(current_consumption);
                    break; //client already get out
                }
                else if (strncmp(read_buf, COMMAND_PING_RESPONSE, strlen(COMMAND_PING_RESPONSE)) == 0)
                {
                    current_consumption++;
                }

                if (current_consumption >= allowed_consumption)
                {
                    user_history.UpdateCurrentTimeConsumption(current_consumption);
                    break;
                }
                Sleep(POLL_INTERVAL);
            }
        }
        else
        {
            DebugPrint("No such user: %s", user_name);
        }

        m_agent_status = EXPIRED;
        WriteFile(m_pipe_handle, COMMAND_LOG_OFF, sizeof(COMMAND_LOG_OFF), &write_size, NULL);
        Sleep(LOG_OFF_INTERVAL);
        CancelIo(m_pipe_handle);
        CloseHandle(m_pipe_handle);
    } while (0);
}


