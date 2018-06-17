#ifndef _USER_HISTORY_H
#define _USER_HISTORY_H

#include <stdint.h>


class CUserHistory
{
public:
    CUserHistory(char* user_name);
    ~CUserHistory();

    uint32_t GetCurrentTimeConsumption();
    void UpdateCurrentTimeConsumption(uint32_t time);


private:
    FILE *m_fp;
    uint32_t m_current_year;
    uint32_t m_current_month;
    uint32_t m_current_day;


};



























#endif 
