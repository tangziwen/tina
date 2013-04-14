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


#include <stdio.h>
#include <assert.h>
#include "vm.h"
#include "var.h"
#include <stdlib.h>
#include <string.h>
#include "script_struct.h"
#include "script_tuple.h"
#define MAX_LOCAL_VAR 1024
#define MAX_LAYER 128
/*栈区用于在真正运行时分配局部变量*/
static Var local_var[MAX_LOCAL_VAR];
/*用于记住每一层的深度,有利于回收*/
static int vm_stack_layer[MAX_LAYER];
static int current_offset=0;
static int current_layer=0;

/*清除当前层所有的局部变量的值,
这些局部变量将被设为Nil类型
*/
void CleanCurrentLocalVar()
{
	int i=current_offset;
	for(; i<current_offset+vm_stack_layer[current_layer]; i++)
	{
		/*如果临时变量维护一个引用,则在清空局部变量时,使引用计数亦减一*/
		int t=var_GetType(local_var[i]);
		var_SetNil( &(local_var[i] ));
	}
}

/*获取当前运行时,所执行的局部块当前层数*/
int vm_GetCurrentLayer()
{

	return current_layer;
}

void vm_rt_stack_var_cast_set(int the_index,Var source)
{
	local_var[current_offset+the_index]=source;
}

void vm_init()
{
	int i=0;
	for(; i<MAX_LOCAL_VAR; i++)
	{
		var_SetNil( &(local_var[i] ));
	}
}

/*设置指定块层次的局部变量数目*/
void vm_SetLayerVarAmount(int index ,int amount)
{
	vm_stack_layer[index]=amount;
}

/*获得指定索引处的栈局部变量*/
Var  vm_rt_stack_var_get(int the_index)
{
    return  local_var[current_offset+the_index];
}

/*获得指定索引处的栈局部变量的指针*/
Var *  vm_rt_stack_var_get_ptr(int the_index)
{
	return  &(local_var[current_offset+the_index]);
}

/*在运行时设置虚拟机中的栈变量*/
void vm_rt_stack_var_set ( int the_index,Var source )
{
	int dist_the_index=vm_GetAbs(the_index);
	local_var[dist_the_index]=source;
}

/*从虚拟机中获得当前层指定索引的栈变量的指针*/
Var * vm_RTstackVarGetAbs ( int the_index )
{
	return & ( local_var[the_index] );
}

/*通过当前层的索引找到该局部变量的绝对索引*/
int vm_GetAbs ( int the_index )
{
	return ( current_offset+the_index );
}



/*运行时的栈加深*/
void vm_RTstackPush()
{
    current_offset+=(vm_stack_layer[current_layer]+1);
	current_layer++;

}




/*运行时的栈弹出(销毁)*/
void vm_RTstackPop()
{
	current_layer--;
    current_offset-=(vm_stack_layer[current_layer]+1);
}
