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
#include <stdlib.h>
#include "il.h"
#include "tina.h"
#include "token.h"
#include "while.h"
#include "expression.h"
#include "var.h"
#include "print.h"
#include "return.h"
#include "if.h"
#include "for.h"
#include "function.h"
void while_parse ( int *pos,int layer ,int mode  )
{
	TokenInfo t_k;
    token_Get ( pos,&t_k );/*略过while的判断表达式的左小括号*/
	if(t_k.type!=TOKEN_TYPE_OP && t_k.content[0]!='(')
	{
		printf("error ,miss '(' in while expression\n");
		exit(0);
	}
	int exp_pos = ( *pos ); /*保留表达式的位置,稍后再解析*/
	/*parse_exp(pos,EXP_CONTROL);*/
	int l1=label_index+1;
	int l2 =l1+1;
	int l3 =l2+1;
	label_index+=3;
	/*第一个跳转*/
	IL_node * n1= IL_node_create_goto ( l1,IL_NODE_JNE );
	IL_node * n2 = IL_node_create_goto ( l2,IL_NODE_GOTO );
	IL_node * n3 =IL_node_create_goto ( l3,IL_NODE_GOTO );
	/*创建与第一个跳转对应的标记*/
	IL_node * lab1= IL_node_create_goto ( l1,IL_NODE_LAB );
	IL_node * lab2= IL_node_create_goto ( l2,IL_NODE_LAB );
	IL_node * lab3= IL_node_create_goto ( l3,IL_NODE_LAB );
	IL_ListInsertNode ( n2 );
	IL_ListInsertNode ( lab1 );
	int postion = ( *pos );
    layer++;
    /*跳过条件表达式,先解析语句*/
    do{
        ( *pos )=postion;
        token_Get ( &postion,&t_k );
    }
    while ( t_k.type!=TOKEN_TYPE_LEFT_BRACE );

    int result =block_Parse(pos,layer,l3,l2,mode);
    if(result!=0)
    {
        STOP("illegal while statement %s",block_GetLastStateStr ());
    }
	IL_ListInsertNode ( lab2 );
	exp_Parse ( &exp_pos,EXP_CONTROL,layer );
	IL_ListInsertNode ( n1 );
	IL_ListInsertNode ( lab3);
}
