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
#ifndef TINA_TOKEN_H
#define TINA_TOKEN_H
#include "type.h"
#include "def.h"

/*有些运算符的自然形式不是一个字符能表示的,我们使用这套宏
去代替它,这样统一为一个字符表示*/
#define OP_EQUAL  1
#define OP_LARGE_OR_EQUAL 2
#define OP_LESS_OR_EQUAL 3
#define OP_ASSIGN_PLUS 4
#define OP_ASSIGN_MINUS 5
#define OP_ASSIGN_MULTIPLE 6
#define OP_ASSIGN_DIVIDE 7
#define OP_NOT_EQUAL 8
#define OP_AND 9
#define OP_OR 10
#define OP_CALL 11
#define OP_SUBSCRIPT 12 /*取索引*/

//标志类型
#define TOKEN_TYPE_INVALID 0
#define TOKEN_TYPE_EOF 1
#define TOKEN_TYPE_IF 2
#define TOKEN_TYPE_WHILE 3
#define TOKEN_TYPE_SYMBOL 4
#define TOKEN_TYPE_NUM 5
#define TOKEN_TYPE_MESSAGE 6 /*消息*/
#define TOKEN_TYPE_OP 7
#define TOKEN_TYPE_LEFT_BRACE 8
#define TOKEN_TYPE_RIGHT_BRACE 9
#define TOKEN_TYPE_SEMICOLON 10
#define TOKEN_TYPE_COMMA 11
#define TOKEN_TYPE_VAR_DEF 12
#define TOKEN_TYPE_REAL 13
#define TOKEN_TYPE_RIGHT_PARENTHESIS 14 //右小括号
#define TOKEN_TYPE_ELSE 15
#define TOKEN_TYPE_PRINT 16
#define TOKEN_TYPE_API 17 //api函数
#define TOKEN_TYPE_TRUE 18
#define TOKEN_TYPE_FALSE 19
#define TOKEN_TYPE_FUNC_DEF 20
#define TOKEN_TYPE_FUNC 21 //用户定义的脚本函数
#define TOKEN_TYPE_RETURN 22 //返回
#define TOKEN_TYPE_REF 23 //引用标记
#define TOKEN_TYPE_RIGHT_BRACKET 24 //右方括号
#define TOKEN_TYPE_FOR  25//for语句
#define TOKEN_TYPE_BREAK 26 //break 语句
#define TOKEN_TYPE_CONTINUE 27 //continue语句
#define TOKEN_TYPE_STRUCT 28
#define TOKEN_TYPE_SELF 29 //指向自身的指针
#define TOKEN_TYPE_NIL 30 //空
#define TOKEN_TYPE_PRIVATE 32
#define TOKEN_TYPE_MODULE 34
#define TOKEN_TYPE_USING 35
#define TOKEN_TYPE_STRUCT_NAME 36
#define TOKEN_TYPE_DELETE 37 /*销毁*/
#define TOKEN_TYPE_CHAR 38
#define TOKEN_TYPE_STRING 39
#define TOKEN_TYPE_TUPLE 40
#define TOKEN_TYPE_IMPORT 41
#define TOKEN_TYPE_CARD 42
#define TOKEN_TYPE_TYPE_OF 43

/*从当前位置(pos)解析一个词法标记，并将词法标记的相关信息存储进t_k所指向的TokenInfo对象里
 解 析之后，当前位置会向后跳跃一个词法标记               *
 我们假设整个待输入的缓存中的词法是无错的(即不存在无效的词法单元)*/
void token_Get (int *pos,TokenInfo *t_k);

//跳过一个语句
void token_skip_statement(int *pos);

void token_skip_block(int * pos);

//带断言的解析词法标记,如果不符合规定的词法类型,则中断,并抛出一个错误的信息
void token_get_assert(int * pos,TokenInfo * t_k,int type, char * info );
#endif

