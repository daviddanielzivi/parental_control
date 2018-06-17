#include <stdio.h>
#include <string.h>
#include "PolicyReader.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
CPolicyReader::CPolicyReader()
{
    //nothing to do for the moment
}
/////////////////////////////////////////////////////////////////////////////////////////////////
CPolicyReader::~CPolicyReader()
{
    m_policy_list.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////////
int32_t CPolicyReader::ParsePolicyFile(char* path)
{
    FILE *fp = NULL;
    Policy policy;
    char line[512];
    char policy_file_name[512];
    int32_t status = POLICY_MANAGER_FAILURE;

    //build the full path policy name
    sprintf(policy_file_name, "%s//policy.txt", path);


    fp = fopen(policy_file_name, "r");

    if (fp != NULL)
    {
        while (fgets(line, sizeof(line), fp))
        {
            if (ParseOneLine(line, policy) == POLICY_MANAGER_SUCCESS)
            {
                m_policy_list.push_back(policy);
            }
        }
        status = POLICY_MANAGER_SUCCESS;
    }

    if (fp)
    {
        fclose(fp);
    }
    return status;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
int32_t CPolicyReader::ParseOneLine(char* line, Policy &policy)
{
    char *pch = NULL;
    int32_t status = POLICY_MANAGER_FAILURE;
    char separators[] = " :,";


    memset(&policy, 0, sizeof(Policy));

    do
    {
        pch = strtok(line, separators);

        if (pch == NULL) //blank line baby
        {
            break;
        }

        if (*pch == '#') //comment
        {
            break;
        }

        if (strlen(pch) > MAX_USER_NAME_SIZE)
        {
            break;
        }

        //extract user name
        memcpy(policy.user_name, pch, strlen(pch));


        //extract start hours    
        pch = strtok(NULL, separators);
        policy.start_allowed_time_hours = atoi(pch);

        //extract start minutes
        pch = strtok(NULL, separators);
        policy.start_allowed_time_minutes = atoi(pch);

        //extract end hours    
        pch = strtok(NULL, separators);
        policy.end_allowed_time_hours = atoi(pch);

        //extract end minutes
        pch = strtok(NULL, separators);
        policy.end_allowed_time_minutes = atoi(pch);

        //extract total allowed time
        pch = strtok(NULL, separators);
        policy.total_allowed_time_minutes = atoi(pch);

        status = POLICY_MANAGER_SUCCESS;
    } while (0);

    return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
int32_t CPolicyReader::GetPolicy(char *user_name, Policy &policy)
{
    int32_t status = POLICY_MANAGER_FAILURE;

    list<Policy>::iterator ListIterator;

    for (ListIterator = m_policy_list.begin(); ListIterator != m_policy_list.end(); ++ListIterator)
    {
        if (strcmp(ListIterator->user_name, user_name) == 0)
        {

            strncpy(policy.user_name, ListIterator->user_name, MAX_USER_NAME_SIZE);
            policy.start_allowed_time_hours = ListIterator->start_allowed_time_hours;
            policy.start_allowed_time_minutes = ListIterator->start_allowed_time_minutes;
            policy.end_allowed_time_hours = ListIterator->end_allowed_time_hours;
            policy.end_allowed_time_minutes = ListIterator->end_allowed_time_minutes;;
            policy.total_allowed_time_minutes = ListIterator->total_allowed_time_minutes;
            status = POLICY_MANAGER_SUCCESS;
        }
    }
    return status;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
void CPolicyReader::PrintAllUsersPolicy()
{
    list<Policy>::iterator ListIterator;

    for (ListIterator = m_policy_list.begin(); ListIterator != m_policy_list.end(); ++ListIterator)
    {
        printf("%s: ", ListIterator->user_name);
        printf("%d:%02d --> ", ListIterator->start_allowed_time_hours, ListIterator->start_allowed_time_minutes);
        printf("%d:%02d ", ListIterator->end_allowed_time_hours, ListIterator->end_allowed_time_minutes);
        printf("total allowed time: %d\n", ListIterator->total_allowed_time_minutes);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////
