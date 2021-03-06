#include "tina.h"
#include "string.h"
#include "compile_error.h"
static int version=20;/*小数点后两位*/



extern void Tina_SetErrorMode(int mode_flag)
{
     CompileError_SetMode( mode_flag);
}

extern int Tina_GetVersion()
{
    return version;
}

extern void Tina_Init(int ver)
{
    version=ver;
    INIT(script_struct);
    INIT(script_tuple);
    INIT(vm);
}

/*载入标准库*/
void Tina_LoadStdLib()
{
    char std_lib[128];
    strcpy (std_lib,getenv ("TINA_LIB"));
    strcat(std_lib,"/std");
    Tina_Load (std_lib);
}
