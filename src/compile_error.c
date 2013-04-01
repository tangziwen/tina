#include "compile_error.h"
#include <stdio.h>
#include <stdlib.h>
#include "loader.h"
#define ERROR_FLAG_NONE 0
#define ERROR_FLAG_STDOUT 1
#define ERROR_FLAG_STDERR 2
#define ERROR_FLAG_LOG 4
#define PATTERN_STRING "at lines %d,file %s:\n%s",loader_GetLineNumber (pos),loader_GetFileName (),error_info
static int flag=ERROR_FLAG_STDOUT;
/*显示一条消息*/
static void Error(int pos,const char * error_info,int mode_flag)
{
    if(mode_flag &ERROR_FLAG_NONE)
    {
        return ;
    }
    if(mode_flag & ERROR_FLAG_STDERR)
    {
        fprintf(stderr,PATTERN_STRING);
    }
    if(mode_flag & ERROR_FLAG_STDOUT)
    {
        printf(PATTERN_STRING);
    }
    if(mode_flag & ERROR_FLAG_LOG)
    {
        static FILE * f=NULL;
        if(!f)
        {
            f=fopen("./Tina_Log.log","w");
        }
        fprintf(f,PATTERN_STRING);
    }
}

/*设置编译器报错模式*/
void CompileError_SetMode(int mode_flag)
{
    flag|=mode_flag;
}


/*编译器报错*/
void CompileError_ShowError(int pos,const char * error_info)
{
    Error(pos,error_info,flag);
}
