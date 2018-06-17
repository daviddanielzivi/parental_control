
#ifndef _POLICY_MANAGER_H
#define _POLICY_MANAGER_H

#include <list>
#include <stdint.h>

using namespace std;

#define MAX_USER_NAME_SIZE  512

#define POLICY_MANAGER_FAILURE   1
#define POLICY_MANAGER_SUCCESS   0

////////////////////////////////////////////////////////////////////////////////////
struct Policy
{
    Policy() { memset(user_name, 0, MAX_USER_NAME_SIZE);
               start_allowed_time_hours = 0; start_allowed_time_minutes = 0;
               end_allowed_time_hours = 0; end_allowed_time_minutes = 0;
               total_allowed_time_minutes = 0; }
    
    char user_name[MAX_USER_NAME_SIZE];
    uint8_t start_allowed_time_hours;
    uint8_t start_allowed_time_minutes;

    uint8_t end_allowed_time_hours;
    uint8_t end_allowed_time_minutes;

    uint32_t total_allowed_time_minutes;
};

////////////////////////////////////////////////////////////////////////////////////
class CPolicyReader
{
public:
    CPolicyReader();
    ~CPolicyReader();

    int32_t ParsePolicyFile(char* path);
    int32_t GetPolicy(char *user_name, Policy &policy);
    void PrintAllUsersPolicy();

private:
    list<Policy> m_policy_list;
    int32_t ParseOneLine(char* line, Policy &policy);

};





#endif
