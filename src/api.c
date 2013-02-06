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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "api.h"
#include "type.h"
#include "for.h"
#include "module.h"
static int API_list_index=0;
//函数的返回值
Var API_return_value;

static int current_tag=0;


//传入API的实参列表
Var API_argument_list[API_MAX_ARG];

//获取脚本代码输入的参数
Var TIna_API_GetArg ( int index )
{
	return API_argument_list[index];
}

//向虚拟机中注册一个函数,输入它在脚本中的别名,以及它的函数指针,还有参数个数
//函数的样式为 void API_FUNC ();
int Tina_API_Register ( const char * func_name,void ( *func_ptr ) () ,int tag)
{
	strcpy ( API_func_list[API_list_index].file_name,func_name );
	API_func_list[API_list_index].func_ptr=func_ptr;
	//API_func_list[API_list_index].arg_count=arg_count;
	API_func_list[API_list_index].tag=tag;
	API_list_index++;
	return API_list_index-1;
}
int  Tina_API_GetTag()
{
	return current_tag;
}


//搜索该字符串是否为一个API函数
//若是,则返回其索引,若不是则返回 -1
static int plain_API_Search (  char * the_string )
{
	int i;
	for ( i=0; i<API_list_index; i++ )
		{
			if ( strcmp ( the_string,API_func_list[i].file_name ) ==0 )
				{
					return i;
				}
		}
	return -1;
}

int API_Search ( const char * the_string )
{
	char name[128]={0};
	int result=-1,i;
	//当前模块内部的检查
	result=plain_API_Search(module_ContextMangledName(the_string));
	if(result>=0)
	{
		return result;
	}
	//再检查导入模块是否有该名字
	for(i=0;i<module_index;i++)
	{
	result=plain_API_Search(module_MangledName(i,the_string));
	if(result>=0)
		{
			return result;
		}
	}
	//如果上述的检查都无法进行，那么进行在全局模块进行检查(或者是此符号已经被完全限定了)
	if(result<0)
	{
		result=plain_API_Search(the_string);
	}
	return result;
}
Var  API_InovkeByIndex ( int index )
{
	current_tag =API_func_list[index].tag;
	API_func_list[index].func_ptr();
	return API_return_value;
}

static int API_arg_count;
//设置当前执行的参数的个数
void API_SetArgCount(int arg_count)
{
	API_arg_count=arg_count;
}

//返回当前的API参数的个数
int API_GetArgCount()
{
	return API_arg_count;
}
void Tina_API_SetReturnValue ( Var a )
{
	API_return_value=a;
}
//获得指定的API参数
Var API_GetArg(int index)
{
	return API_argument_list[index];
}

//搜索字符串所指示的结构体类型是否存在，若存在返回其
//构造API
int API_Search_cnstructor(char * struct_name)
{
char  result[128]={0};
strcpy(result,struct_name);
strcat(result,"@new");
return API_Search(result);
}


