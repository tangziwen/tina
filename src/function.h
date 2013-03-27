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
#ifndef TINA_FUNCTION_H
#define TINA_FUNCTION_H
#define FUNCTION_MAX 64
#include "type.h"
#include "def.h"
#define FUNC_GLOBAL 0 /*全局函数*/
#define FUNC_MEMBER 1 /*成员函数*/


/*解析函数的定义*/
Function *  func_ParseDef(int *pos);

/*解析函数声明*/
void func_ParseDeclare(int *pos);

/*通过名称检索函数,如果存在,返回索引*/
/*如果不存在,则返回-1*/
int func_get_index_by_name(const char *func_name);


/*通过索引返回函数的指针*/
Function *func_get_by_index(int index);


/*通过一个字符串来运行一个函数*/
void Tina_Run(const char *name);

/*通过索引值来调用函数,返回值为该函数的返回值*/
Var  func_invoke(int index);

/*获得当前正在被解析的函数*/
Function * func_get_current();

/*获得当前已经被解析过的函数个数*/
int Tina_FuncGetCount();

/*解析方法（成员函数）的声明*/
void method_parse_declare(int *postion,char * class_name);


/*解析方法（成员函数）的定义*/
Function * method_parse_def(int *postion,char * class_name );
/*设置脚本函数的参数*/
Var Tina_CallScriptFunc(int func_id);
/*调用脚本函数*/
void Tina_SetScriptFuncArg(Var arg,int id);
/*
平凡性调用,用于具有聚合原子性的地方
*/
Var  func_PlainInvoke(int index);

//函数编译成字节码
void func_WriteByteCode(FILE * f);
/*从字节码中载入函数定义*/
void func_Load(char *str);

/*将函数的数据清空，以方便下次编译*/
void func_Dump();
#endif
