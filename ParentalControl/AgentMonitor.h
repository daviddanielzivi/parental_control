#ifndef _AGENT_MONITOR_H
#define _AGENT_MONITOR_H

#include <stdint.h>
#include <Windows.h>
#include "PolicyReader.h"

#define PIPE_CONNECTION_TIMEOUT    10*1000
#define PIPE_CONNECTION_INTERVAL   1000

enum AgentMonitorStatus
{
    UNINITIALIZED = 0,
    INITIALIZED = 1,
    RUNNING = 2,
    EXPIRED = 3,
};

class CAgentMonitor
{
public:
    CAgentMonitor(HANDLE pipe_handle);
    ~CAgentMonitor();
    uint32_t Init();
    void Monitor();
    void StopMonitoring();
    AgentMonitorStatus GetStatus();

private:
    bool m_stop_monitoring;
    AgentMonitorStatus m_agent_status;
    HANDLE m_pipe_handle;
    unsigned long m_thread_id;
    CPolicyReader m_policy_reader;




};





#endif
