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
#ifndef TINA_TYPE_H
#define TINA_TYPE_H

#define ENV_GLOBAL -1
#define CLASS_PUBLIC   0
#define CLASS_PRIVATE   1


#define CLASS_NOT_SEALED  0
#define CLASS_SEALED 1
//环境索引
extern  int env_index;
//the maximun token's name size
#define TOKEN_NAME_SIZE 32

//a struct which hold the info of the current token we parsed
typedef struct token_info_t
{
	int type;
	char content[TOKEN_NAME_SIZE ];
	char * str;//专门为字符串变量准备的指针
} TokenInfo;


#define FUNC_NORMAL 0
#define FUNC_API  1
struct func_info
{
int func_type;
int func_index;
int args;
};

struct var_content
{
	int type;
	int int_value;
	double real_value;
	void * handle_value;
	int bool_value;
	struct func_info func;
	char *str;
};

typedef struct
{
	char name[TOKEN_NAME_SIZE];
	void * address;
	struct var_content content;
	int index;
	int class_id;
	int layer;//变量所在的层数
} Var;


//表征四元式中的操作数元素,它可能是代表一个变量,一个函数,
//或者是编译生成的临时变量
typedef struct
{
	int type;
	int index;
	Var value;
	union
	{
		int array_index;//数组取下标的临时变量的索引
		int function_indexs[10];//函数取下标的临时变量的索引
	};
} IL_element;

//四元式表达式(中间代码中用于计算的形式)
//四元式
//形如 : T = A op B
typedef struct IL_exp_t
{
	int tmp_index;//临时变量的地址
	IL_element A;//操作数A
	IL_element B;//操作数B
	char op;//操作符
	struct IL_exp_t * next;//下一条语句
	int id;//代号
}  IL_exp;


//跳转节点,用于实现if语句
typedef struct
{
	int label;
} IL_jmp;


//打印节点,是一个原语,用于打印当前变量的重要信息
typedef struct
{
	char a;
} IL_print;


//返回节点,当执行到此节点的时候,取前一个表达式的结果作为返回值,返回给上层函数
typedef struct
{
	int dummy;
} IL_return;

typedef struct IL_node_t
{
	int type;
	union
	{
		IL_exp *exp;
		IL_jmp *jmp;
		IL_print * prnt;
		IL_return * retrn;
	};
	struct IL_node_t * next;
	struct IL_node_t * pre;
} IL_node;

//中间语言执行序列
typedef struct
{
	Var tmp_var_list [32];
	IL_node * head;
} IL_list;

//函数
typedef struct
{
	IL_list list;
	Var var_list[128];
	int var_counts;
	int arg_counts;
	char name[TOKEN_NAME_SIZE];
} Function;





#endif
