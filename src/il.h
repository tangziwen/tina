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
#ifndef TINA_IL_H
#define TINA_IL_H
/**il.h包含了关于中间代码的一些定义及其一些操作
 * 中间代码以链表的形式储存,每一个节点都是一个四元表达式
 * 形如 result = A op B
 * A 和 B或为该函数的局部变量,或为一个常量,或为一个临时变量.
 * 当A为局部变量时,其储存的是其相对于函数的相对索引,具体的定位将在
 * 执行过程中根据函数调用的嵌套来决定
 */
#include "var.h"
#include "def.h"
#define IL_VAR_TMP 1
#define IL_VAR_GLOBAL 2

#define IL_NODE_EXP 1 /*表示此节点为一个表达式*/
#define IL_NODE_JE 2  /*表示此节点为一个成假跳转(等于零跳转),只会向下寻找*/
#define IL_NODE_JNE 3 /*表示此节点为一个成真跳转(非零跳转),只会向上寻找*/
#define IL_NODE_JMP 4 /*表示此节点为一个强制跳转,只会向下寻找*/
#define IL_NODE_LAB 5 /*表示此节点为一个跳转标记*/
#define IL_NODE_PRNT 6 /*现实原语*/
#define IL_NODE_RETURN 7 /*返回节点*/
#define IL_NODE_DELETE 8 /**/

#define ELEMENT_VAR 1
#define ELEMENT_NUM 2
#define ELEMENT_OP 3
#define ELEMENT_TMP 4
#define ELEMENT_API 5
#define ELEMENT_FUNC 6
#define ELEMENT_ARRAY 7
#define ELEMENT_CALL_BY_PTR 8
#define ELEMENT_CALL_BY_MEMBER 9
#define ELEMENT_SELF 10
#define ELEMENT_STRUCT 11
/*当前的标签数*/
extern int label_index;



extern IL_node* IL_node_create_exp(IL_exp * exp);

extern IL_node * IL_node_create_jmp(int label ,int mode);

extern IL_node * IL_node_create_prnt( );



extern IL_list list;

extern Var self_ptr;

/*当前的表*/
IL_list * current_list;

/*创建一个中间语言表达式*/
IL_exp * IL_exp_create(char op,int tmp_var_index,IL_element var_a,IL_element var_b);

/*向中间语言的执行序列中增加一个表达式*/
void IL_ListInsert(IL_exp *exp);

void IL_ListInsertNode(IL_node *node);

/*打印中间代码序列*/
void IL_list_print(Function * func);

/*执行中间代码*/
Var IL_exec(Function *func);

/*重设临时变量*/
void IL_tmp_reset();

/*往中间代码中加入返回节点*/
void IL_ListInsertReturn();

#endif
