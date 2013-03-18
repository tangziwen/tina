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
#ifndef TINA_VECTOR_H
#define TINA_VECTOR_H
#include "type.h"
#include "def.h"
/*获得数组指定元素的值*/
Var vector_GetValue(Var array_obj,int index );
/*数组初始化*/
void vector_init();

/*为数组的指定元素赋值*/
void vector_SetValue(Var array_obj,int index ,Var new_value);
void VectorRefCountIncrease(void * ptr);
void VectorRefCountDecrease(void * ptr);
/*清除数组临时对象池*/
void vector_CleanTmpPool();
/*以指定字符串作为资源，创建一个向量，该向量的各个维度为字符串中的各字符，成员包括末尾的终结符*/
void * vector_CreateByString(const char * str);
/*打印一个向量的所有成员*/
void vector_Print(Var vec);
/*获得指定向量维护的类型*/
int vector_GetType(Var vec);
/*获得指定向量所存在的成员个数*/
int vector_GetSize(Var vec);
/*将一个向量的元素，若其维护的值为VAR_TYPE_CHAR的话，转移至一个char型数组里。该空间有使用者回收*/
char * vector_ToString(Var vec);
/*创建一个向量*/
Var the_vector_creator(int size,Var init_arg[]);
/*从原位置克隆一个向量*/
Var vector_Clone(Var src);
#endif
