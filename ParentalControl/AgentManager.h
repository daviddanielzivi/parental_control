#ifndef _AGENT_MANAGER_H
#define _AGENT_MANAGER_H

#include "AgentMonitor.h"

#define MAX_NUM_OF_AGENTS   10

class CAgentManager
{
public:
    CAgentManager();
    ~CAgentManager();
    uint32_t StartNewAgent(HANDLE pipe_handle);
    void ReleaseExpiredAgents();
    void StopAllAgents();
private:
    CAgentMonitor *m_agents[MAX_NUM_OF_AGENTS];
};





#endif
