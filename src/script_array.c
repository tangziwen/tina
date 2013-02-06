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
#include "script_array.h"
#include "var.h"
#include "api.h"
#include<assert.h>
#include <stdlib.h>
typedef struct
{
	int array_size;
	Var * var_handle;
} array_info;




array_info * create_array(int array_size)
{
	array_info * new_array=malloc(sizeof(array_info));
	new_array->array_size=array_size;
	new_array->var_handle= malloc(sizeof(Var) * array_size);
	Var * array= ( Var * ) new_array->var_handle;
	{
		int i;
		//初始化
		for ( i=0; i<array_size; i++ )
			{
				array[i].content.type=VAR_TYPE_INT;
				array[i].content.int_value=0;
			}
	}
	return new_array;
}
//获得数组指定元素的值
Var array_GetValue(Var array_obj,int index )
{
	array_info * a;
	a=(array_info *)array_obj.content.handle_value;
	if(index <a->array_size)
	{
	Var * the_array =a->var_handle;
	return the_array[index];
	}
	else
	{
	STOP("array getter: index out of bound!");
	}
}

//为数组的指定元素赋值
void array_SetValue(Var array_obj,int index ,Var new_value)
{
	array_info * a;
	a=(array_info *)array_obj.content.handle_value;
	if(index <a->array_size)
	{
	Var * array =a->var_handle;
	array[index]=new_value;
	}
	else
	{
	STOP("array setter: index out of bound!");
	}
}

static void the_array_creator()
{
	Var result;
	result.content.type=VAR_TYPE_ARRAY;
	Var  num = API_argument_list[0];
	if(API_GetArgCount()!=1)
		{
			printf("the \" array \" function only need one args!!\n");
			exit(0);
		}
	if(num.content.type!=VAR_TYPE_INT  || num.content.int_value<0)
		{
			printf("the array size must be an  positive   integer");
			exit(0);
		}
	int size =num.content.int_value;
	result.content.handle_value= create_array(size);

	Tina_API_SetReturnValue ( result );

}

//获取数组的长度
static void get_array_length()
{
	Var result;
	result.content.type=VAR_TYPE_INT;
	if(API_GetArgCount()!=1)
		{
			printf("the \" array_size \" function only need one args!!\n");
			exit(0);
		}
	Var  the_array_var = API_argument_list[0];
	if(the_array_var.content.type!=VAR_TYPE_ARRAY)
		{
			printf("array_size only support array type!\n");
			exit(0);
		}
	array_info * a=the_array_var.content.handle_value;
	result.content.int_value=a->array_size;
	Tina_API_SetReturnValue ( result );
}

void script_array_init()
{
	Tina_API_Register ( "array",the_array_creator,0);
	Tina_API_Register ( "length",get_array_length,0);
}
