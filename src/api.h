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
#ifndef TINA_API_H
#define TINA_APT_H
#include "var.h"
#define API_FUNC_NAME_MAX 32
#define API_MAX 64
#define API_MAX_ARG 8
#include "def.h"
typedef struct api_func_t
{
	char file_name[API_FUNC_NAME_MAX];
	void (*func_ptr)();
	int tag;/*给用户使用的一个标记量*/
} API_Func;



API_Func API_func_list[API_MAX];

/*传入API的实参列表*/
Var API_argument_list[API_MAX_ARG];

/*向虚拟机中注册一个函数,输入它在脚本中的别名,以及它的函数指针,以及参数个数*/
/*函数的样式为 void API_FUNC ();*/
int Tina_API_Register( const char * func_name,void (*func_ptr)(),int tag);

/*搜索该字符串是否为一个API函数*/
/*若是,则返回其索引,若不是则返回 -1*/
int API_Search(const char * the_string);

/*执行API函数,并以Var型返回该API的返回值*/
Var API_InovkeByIndex(int index);

/*设置当前执行的参数的个数*/
void API_SetArgCount(int arg_count);

/*返回当前的API参数的个数*/
int API_GetArgCount();

/*获得指定的API参数*/
Var API_GetArg(int index);

/*搜索字符串所指示的结构体类型是否存在，若存在返回其*/
/*构造API*/
int API_Search_cnstructor(char * struct_name);


/*清除所有的数据资源*/
void API_Dump();

/*提供一个快速注册同名函数的方法*/
#define Tina_API_REG( FUNC_REG) Tina_API_Register ( #FUNC_REG , FUNC_REG , 0 )
#endif
