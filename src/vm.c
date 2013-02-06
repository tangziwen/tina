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
//栈区用于在真正运行时分配局部变量
Var vm_stack_var[1024];
//用于记住每一层的深度,有利于回收
int vm_stack_layer[128];
int vm_stack_offset=0;
int vm_stack_index=0;

void vm_rt_stack_var_cast_set(int the_index,Var source)
{
	vm_stack_var[vm_stack_offset+the_index]=source;
}


//获得指定索引处的栈局部变量
Var  vm_rt_stack_var_get(int the_index)
{
	return  vm_stack_var[vm_stack_offset+the_index];
}

//获得指定索引处的栈局部变量的指针
Var *  vm_rt_stack_var_get_ptr(int the_index)
{
	return  &(vm_stack_var[vm_stack_offset+the_index]);
}

//在运行时设置虚拟机中的栈变量
void vm_rt_stack_var_set ( int the_index,Var source )
{
	int dist_the_index=vm_get_absolutely(the_index);
	//若欲被赋值的对象是一个引用,那我们实际上改变的是其所指向的变量的值
	if ( vm_stack_var[dist_the_index].content.type==VAR_TYPE_REF )
		{
			Var * a=vm_rt_stack_var_get_abs ( vm_stack_var[dist_the_index].content.handle_value );
			if ( source.content.type!=VAR_TYPE_REF )   //只有被赋值的对象是引用
				{
					( *a ) =source;
				}
			else     //两者都是引用
				{
					Var *b;
					b =vm_rt_stack_var_get_abs ( source.content.handle_value );
					( *a ) = ( *b );
				}
		}
	else     //右值不是引用的情况
		{
			if(source.content.type!=VAR_TYPE_REF)
				{
					//右值是一个字符串？
					if(source.content.type==VAR_TYPE_STR)
						{
							//如果左值也是，那么需要先销毁再使用
							if(vm_stack_var[dist_the_index].content.type==VAR_TYPE_STR)
								{
									free(vm_stack_var[dist_the_index].content.str);
								}
							vm_stack_var[dist_the_index].content.str=malloc(strlen(source.content.str)+1);
							strcpy(vm_stack_var[dist_the_index].content.str,source.content.str);
						}
					vm_stack_var[dist_the_index]=source;
				}
			else
				{
					Var *b;
					b =vm_rt_stack_var_get_abs ( source.content.handle_value );
					vm_stack_var[dist_the_index]=(*b);
				}
		}
}

//从虚拟机中获得当前层指定索引的栈变量的指针
Var * vm_rt_stack_var_get_abs ( int the_index )
{
	return & ( vm_stack_var[the_index] );
}

//通过当前层的索引找到该局部变量的绝对索引
int vm_get_absolutely ( int the_index )
{
	return ( vm_stack_offset+the_index );
}
//运行时的栈加深
void vm_rt_stack_push()
{
	vm_stack_offset+=vm_stack_layer[vm_stack_index];
	vm_stack_index++;

}

//运行时的栈弹出(销毁)
void vm_rt_stack_pop()
{
	vm_stack_index--;
	vm_stack_offset-=vm_stack_layer[vm_stack_index];
}
