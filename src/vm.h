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
#ifndef TINA_VM_H
#define TINA_VM_H
#include "type.h"
#include "def.h"
/*vm.h描述了在tina脚本运行时栈中的变量及其操作
 * 在扫描函数的时候,我们为每一个代码快中的局部变量
 * 都指定了一个相对于该函数的索引
 * 在解析表达式的时候我们将这个相对的索引带入了中间代码中.
 * 但是在中间代码执行时,因为函数调用的栈性质,我们需要定位相对索引的绝对位置
 */


//栈区用于在真正运行时分配局部变量
extern Var vm_stack_var[1024];
//用于记住每一层的深度,有利于回收
extern int vm_stack_layer[128];


//在运行时对虚拟机中的栈变量赋值,但是该变量的类型
//以及值将会做适当的改变,如果是一个引用的话,将会取其指向的变量的值
//
void vm_rt_stack_var_set(int index,Var source);


//获得指定索引处的栈局部变量
Var  vm_rt_stack_var_get(int index);



//获得指定索引处的栈局部变量的指针
Var *  vm_rt_stack_var_get_ptr(int index);



//在运行时中按照强制转换对栈变量进行赋值,所以该变量的所包含的一切信息(包括不限于 类型,和值)将
//与源变量绝对保持一致
void vm_rt_stack_var_cast_set(int index,Var source);


//从虚拟机中获得当前层指定索引的栈变量
Var * vm_rt_stack_var_get_abs(int index);

//通过当前层的索引找到该局部变量的绝对索引
int vm_get_absolutely(int index);

//运行时的栈加深
void vm_rt_stack_push();

//运行时的栈弹出(销毁)
void vm_rt_stack_pop();
//
extern int vm_stack_offset;
extern int vm_stack_index;
#endif
