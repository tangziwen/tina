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
#ifndef TINA_VAR_H
#define TINA_VAR_H
#include "tina.h"
#include "token.h"
#include "def.h"
#define GLOBAL_VAR_MAX 64

#define VAR_TYPE_VOID 0
#define VAR_TYPE_INT 1
#define VAR_TYPE_HANDLE 2
#define VAR_TYPE_BOOL 3
#define VAR_TYPE_NULL 4 //空类型用于在表达式只剩一个值或变量或函数的时候,如:a ,则将其转换为a+null;
#define VAR_TYPE_STR 5 //字符串型变量
#define VAR_TYPE_REF 6
#define VAR_TYPE_ARRAY 7
#define VAR_TYPE_REAL 8
#define VAR_TYPE_FUNC 9 //函数指针类型
#define VAR_TYPE_STRUCT_NAME 10 //函数名

//null只能作为右操作数,a+null返回a的值




///通过名字查找当前被扫描函数的局部变量的索引/通过变量获得它的类型名称.
int var_get_local_index(const char *var_name,int layer);

//获得当前被扫描函数的局部变量的个数
int get_vars_count();

//销毁最内层的变量
void destory_layer();

//新建一个层次
void create_layer();

//获得当前的数值,以浮点型计
double get_value(int index);

double var_get_value(Var a);

//计算两数相加
Var var_add(Var a,Var b);
//两数相减
Var var_minus(Var a,Var b);
//两数相乘
Var var_multiple(Var a,Var b);
//两数相除
Var var_divide(Var a, Var b);

Var var_large(Var a,Var b);

Var var_less(Var a,Var b);

Var var_equal(Var a,Var b);

Var var_not_equal(Var a,Var b);

Var var_less_or_equal(Var a,Var b);

Var var_large_or_qual(Var a,Var b);

Var var_refer(Var a);

Var var_and(Var a, Var b);

Var var_or(Var a, Var b);

Var var_point_to(Var a,Var b);
//设置当前函数
void func_set_current(Function * func);
//变量模块初始化
void var_parse_local(int *pos,int arg_counts);

//解引用变量,变量的类型必须为VAR_TYPE_REF
Var * var_derefer(Var ref);

//获得Var变量的浮点值
double var_GetDouble(Var a);
//获得Var变量的整型值
int var_GetInt(Var a);
//获得Var变量的字符串值
char * var_GetStr(Var a);
//获得变量的布尔值
int var_GetBool(Var a);
//获得变量的值类型
int var_GetType(Var a);
//设置Var变量的实数值，如果其不为实数，类型将会强制转换
void var_SetReal(Var *a,double value);
//设置Var变量的整数值，如果其不为整数，类型将会强制转换
void var_SetInt(Var *a,int value);
//设置Var变量的布尔值，如果其不为布尔，类型将会强制转换
void var_SetBool(Var *a,int value);
//在stdout里打印Var变量的值，注意不带换行
void var_Print(Var a);


#endif
