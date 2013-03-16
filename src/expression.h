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
#ifndef TINA_EXP_H
#define TINA_EXP_H
#include "def.h"
#define EXP_NORMAL 1 /*普通的表达式以";"结尾*/
#define EXP_INIT 2 /*初始化中的表达式,以";"或","结尾*/
#define EXP_CONTROL 3 /*控制表达式,以 "{" 结尾*/
#include "tina.h"
#include "var.h"
typedef struct element_t
{
	int type;
	Var var_value;
	char op;
	int index;
	union
	{
		int array_index;/*数组取下标的临时变量的索引*/
		int function_indexs[10];/*函数取下标的临时变量的索引*/
	};
} Element;
/*计算表达式*/
/*形如 a=3*(c+b)/2.5;*/
void exp_Parse(int * pos,int mode,int layer);

/*把一个数字压入逆波兰表达式队列中*/
void put_num_to_RPN(Var number);
/*把一个变量压入逆波兰表达式中*/
void put_var_to_RPN(int index);
/*把一个操作符压入转换逆波兰表达式的辅助栈中*/
/*输入的是该运算符的字符形式.*/
void push_op_to_RPN_stack(char op);

/*把一个API压入转换逆波兰的辅助栈中,输入的是他的索引值*/
void push_API_to_RPN_stack(int index);

/*把一个脚本函数压入转换逆波兰的辅助栈中,输入的是他的索引值*/
void push_func_to_RPN_stack(int index,int mode);

/*把一个数组压入转换逆波兰的辅助栈中,输入的是他的索引值*/
void push_array_to_RPN_stack(int index);

/*获得中间栈顶的类型*/
int RPN_GetStackTopType();
/*获得队列尾部信息*/
int GetRPNQueueTail();
/*获得逆波兰表达式辅助栈栈顶的运算符的优先级*/
int get_top_priority_RPN();

/*检测逆波兰表达式的栈是否为空栈是否为空*/
int isRPN_StackEmpty();

/*转移栈顶的操作符进入输出队列*/
void TransferStackTop();


/*把输出队列的末尾的元素移入栈中*/
void TransferQueueEnd(int type);
/*销毁栈顶的元素*/
void RemoveRPN_StackTop();

/*获取操作符的优先级*/
int GetOpPriority( char op);

/*打印逆波兰队列*/
void PrintRPN();

/*生成中间代码*/
void generate_IL();
/*重设计算表达式的各种数据及变量,为下次计算做准备*/
void exp_reset();

/*获得栈中最上面的元素*/
Element * GetRPNStackTop();

extern int is_exp_parsing;
#endif
