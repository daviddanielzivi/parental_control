#include "statuses.h"
#include "AgentManager.h"



////////////////////////////////////////////////////////////////////////////////////////
CAgentManager::CAgentManager()
{
    for (int i = 0; i < MAX_NUM_OF_AGENTS; i++)
    {
        m_agents[i] = NULL;
    }
}
////////////////////////////////////////////////////////////////////////////////////////
CAgentManager::~CAgentManager()
{
    for (int i = 0; i < MAX_NUM_OF_AGENTS; i++)
    {
        if (m_agents[i] != NULL)
        {
            delete m_agents[i];
            m_agents[i] = NULL;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////
uint32_t CAgentManager::StartNewAgent(HANDLE pipe_handle)
{
    uint32_t status = STATUS_FAIL;

    for (int i = 0; i < MAX_NUM_OF_AGENTS; i++)
    {
        if (m_agents[i] == NULL)
        {
            m_agents[i] = new CAgentMonitor(pipe_handle);
            m_agents[i]->Init();
            status = STATUS_OK;
            break;
        }
    }
    return status;
}
////////////////////////////////////////////////////////////////////////////////////////
void CAgentManager::ReleaseExpiredAgents()
{
    for (uint32_t i = 0; i < MAX_NUM_OF_AGENTS; i++)
    {
        if (m_agents[i] != NULL)
        {
            if (m_agents[i]->GetStatus() == EXPIRED)
            {
                delete m_agents[i];
                m_agents[i] = NULL;
            }
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////
void CAgentManager::StopAllAgents()
{
    for (uint32_t i = 0; i < MAX_NUM_OF_AGENTS; i++)
    {
        if (m_agents[i] != NULL)
        {
            m_agents[i]->StopMonitoring();
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////
