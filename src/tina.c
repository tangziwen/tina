#include "tina.h"
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
