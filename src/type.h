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
#define STRUCT_PUBLIC   0
#define STRUCT_PRIVATE   1


#define STRUCT_NOT_SEALED  0
#define STRUCT_SEALED 1
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
    void* literal_handle;
} TokenInfo;


#define FUNC_NORMAL 0
#define FUNC_API  1
#define FUNC_DYNAMIC 2
#define FUNC_METHOD 3
struct func_info
{
    int func_type;
    int func_index;
    int args;
};



struct element_info
{
    int list_index;
    int var_id;
};

union var_value_t {
    int int_value;
    double real_value;
    void * handle_value;
    int bool_value;
    struct func_info func;
    char *str;
    char char_value;
    struct element_info element;
    int struct_id;
};


struct var_content
{
    char type;
    union var_value_t  var_value;
};

typedef struct
{
    char name[TOKEN_NAME_SIZE];
    struct var_content content;
    int class_id;
    int layer;//变量所在的层数
} Var;


union element_info_t {
    int func_args;
    int array_index_tmp;
};



//表征四元式中的操作数元素,它可能是代表一个变量,一个函数,
//或者是编译生成的临时变量
typedef struct
{
    int type;
    int index;
    union element_info_t info;
} IL_element;

//四元式表达式(中间代码中用于计算的形式)
//四元式
//形如 : T = A op B
typedef struct IL_exp_t
{

    IL_element A;//操作数A
    IL_element B;//操作数B
    char op;//操作符
    int id;//代号
}  IL_exp;


/*调用列表节点*/
typedef struct IL_call_list_t
{
    unsigned int args;//参数个数
    int list_id;//调用的列表索引号.
}IL_CallNode;

/*列表构造器*/
typedef struct IL_list_creator_t
{
    int init_args;/*初始化的参数*/
} IL_ListCreatorNode;

/*结构体构造器*/
typedef struct IL_struct_creator_t
{
    int id;/*结构体原型索引*/
    int init_args; /*构造函数参数个数*/
} IL_StructCreatorNode;

//跳转节点,用于实现if语句
typedef struct
{
    int label;
} IL_jmp;


//引用数组的节点
typedef struct
{
    int var_index; /*储存变量索引*/
} IL_ListNode;

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
    int tmp_index;//临时变量的索引
    union
    {
        IL_exp *exp;
        IL_jmp *jmp;
        IL_print * prnt;
        IL_return * retrn;
        IL_CallNode * call;
        IL_ListNode * list_element;
        IL_ListCreatorNode * list_creator;
        IL_StructCreatorNode * struct_creator;
    };
    struct IL_node_t * next;
    struct IL_node_t * pre;
} IL_node;

#define TMP_ARITH 1 /*算术类型*/
#define TMP_DEREFER 2 /*解引用类型，比如下标运算，"."运算*/

struct tmp_result
{
    Var * result;/*存放临时结果的地址*/
    int tmp_type;/*临时过程产生的类型*/
};

//中间语言执行序列
typedef struct
{
    struct tmp_result tmp_var_list [32];
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
