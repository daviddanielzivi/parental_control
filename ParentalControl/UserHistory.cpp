#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "DebugPrint.h"
#include "UserHistory.h"


////////////////////////////////////////////////////////////////////////////////////////////
CUserHistory::CUserHistory(char* user_name)
{
    time_t now;
    tm *local_time;

    now = time(0);
    local_time = localtime(&now);

    m_current_year = local_time->tm_year + 1900; 
    m_current_month = local_time->tm_mon + 1; 
    m_current_day = local_time->tm_mday;

    m_fp = fopen(user_name, "a+");

    if (m_fp == NULL)
    {
        DebugPrint("failed to open user history filename: %s", user_name);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
CUserHistory::~CUserHistory()
{
    if (m_fp)
    {
        fclose(m_fp);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////
uint32_t CUserHistory::GetCurrentTimeConsumption()
{
    char line[512];
    uint32_t current_time_consumption = 0; 

    if (m_fp != NULL)
    {
        while (fgets(line, sizeof(line), m_fp))
        {
            char *pch = NULL;
            char separators[] = " :,";
            uint32_t day;
            uint32_t month;
            uint32_t year;
            uint32_t time_in_minutes;

            do
            {
                pch = strtok(line, separators);

                if (pch == NULL) //blank line baby
                {
                    break;
                }

                //extract the day
                day = atoi(pch);

                //extract month
                pch = strtok(NULL, separators);
                month = atoi(pch);

                //extract year    
                pch = strtok(NULL, separators);
                year = atoi(pch);

                //extract current consumption    
                pch = strtok(NULL, separators);
                time_in_minutes = atoi(pch);

                if ((day == m_current_day) && (month == m_current_month) && (year == m_current_year))
                {
                    current_time_consumption = time_in_minutes;
                }
            } while (0);
        }
    }
    return current_time_consumption;
}
////////////////////////////////////////////////////////////////////////////////////////////
void CUserHistory::UpdateCurrentTimeConsumption(uint32_t time)
{
    if (m_fp)
    {
        fprintf(m_fp, "%02d %02d %04d %02d\n", m_current_day, m_current_month, m_current_year, time);
        fflush(m_fp);
    }
}
////////////////////////////////////////////////////////////////////////////////////////////



