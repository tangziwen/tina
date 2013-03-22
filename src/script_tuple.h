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
#ifndef TINA_ARRAY_H
#define TINA_ARRAY_H
#include "type.h"
#include "def.h"
/*获得数组指定元素的值*/
Var * tuple_GetValue(Var array_obj,int index );
/*数组初始化*/
void tuple_init();

/*为数组的指定元素赋值*/
void tuple_SetValue(Var array_obj,int index ,Var new_value);
void TupleRefCountIncrease(void * ptr);
void TupleRefCountDecrease(void * ptr);
/*元组构造器*/
Var the_tuple_creator(int size,Var init_arg[]);
/*清除数组临时对象池*/
void tuple_CleanTmpPool();
#endif /* TINA_ARRAY_H*/
