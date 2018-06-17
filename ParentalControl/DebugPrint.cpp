#include <stdarg.h>
#include <Windows.h>
#include <stdio.h>


static FILE *fp = NULL;
////////////////////////////////////////////////////////////////////
void DebugPrintInit()
{
    if (fp == NULL)
    {
        fp = fopen("parental_control.log", "w+");
    }
}
////////////////////////////////////////////////////////////////////
void DebugPrint(char * format, ...)
{
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    if (fp != NULL)
    {
        fprintf(fp, "%s\n", buffer);
        fflush(fp);
    }
    
}
/////////////////////////////////////////////////////////////////////////
void DebugPrintShutdown()
{
    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }
}
/////////////////////////////////////////////////////////////////////////