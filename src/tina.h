/************************************************************************************
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.

此程序为自由软件,你可以在遵循由自由软件基金会发布的GPL 通用公共许可
证第三或更高版本的约束下再发布和(或)修改
(对GPL许可证的细节有疑问可参看位于gpl-CN.txt里的非官方翻译)

发布此程序是希望它有助于您,但是作者对其不做任何担保,即使是商业上或
合于特定用途的隐式担保亦无
*********************tzw <tzwtangziwen@gmail.com>******************************/
#ifndef TINA_H
#define TINA_H
#include "type.h"
#include "def.h"
#include "var.h"
#define TINA_ERROR_NONE 0
#define TINA_ERROR_STDOUT 1
#define TINA_ERROR_STDERR 2
#define TINA_ERROR_LOG 4
/*运行一个函数*/
void Tina_Run(const char *name);

//通过指定一个tina脚本文件并将其编译为中间代码,放入内存中以便运行
void Tina_Compile(const char * file_name);

/*载入标准库*/
void Tina_LoadStdLib();
/*向脚本注册一个API*/
int Tina_API_Register( const char * func_name,void (*func_ptr)(),int tag);

/*设置返回值*/
void Tina_API_SetReturn(Var return_value);

/*获得参数*/
Var TIna_API_GetArg(int index);

int Tina_API_GetTag();

/*获得当前已经被解析过的函数个数*/
int Tina_FuncGetCount();

//根据函数名称打印中间代码
void Tina_PrintIL(FILE*f,const char * func_name);

/*创建一个结构体原型,返回其索引*/
extern int Tina_CreateProtype(const char * name, int index, int initial_acc);


/*为指定结构体原型添加成员,并指定其值.
返回其索引
*/
extern int Tina_ProtypeAddMember(int id, const char * member_name, Var value);

/*
未指定的结构体原型的指定成员赋值
*/
extern void Tina_ProtypeSetMember(int struct_id,int member_id,Var value);

/*载入一个字节码文件*/
void Tina_Load(const char *file_name);
//初始化tina
extern void Tina_Init(int ver);

/*载入字节码链表，并执行main 函数*/
void Tina_ExcuteByteCodeList(const char * file);
/*根据字节码列表文件，逐个编译*/
void Tina_Buid(const char * file);
/*获取当前版本号*/
extern int Tina_GetVersion();
/*设置编译器报错级别*/
extern void Tina_SetErrorMode(int mode_flag);
#endif
