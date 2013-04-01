#ifndef COMPILE_ERROR_H
#define COMPILE_ERROR_H

#endif


/*设置编译器报错模式*/
void CompileError_SetMode(int mode_flag);

/*编译器报错*/
void CompileError_ShowError(int pos,const char * error_info);
