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
#include "for.h"
#include <stdlib.h>
#include <stdio.h>
#include "il.h"
#include "tina.h"
#include "token.h"
#include "while.h"
#include "expression.h"
#include "var.h"
#include "print.h"
#include "return.h"
#include "if.h"
#include "function.h"
/*解析形如*/
/*for(exp1; exp2;exp3)*/
/*{*/
/*..........*/
/*}*/

void for_parse ( int *pos,int layer ,int mode)
{
	int l1=label_index+1;
	int l2 =l1+1;
	int l3=l2+1;
	int l4=l3+1;
	label_index+=4;
	layer+=1;

	IL_node * n1= IL_node_create_goto ( l1,IL_NODE_JNE );
	IL_node * n2 = IL_node_create_goto ( l2,IL_NODE_GOTO );
	IL_node * n3 = IL_node_create_goto ( l3,IL_NODE_GOTO );
	IL_node * n4 = IL_node_create_goto ( l4,IL_NODE_GOTO );
	/*创建与第一个跳转对应的标记*/
	IL_node * lab1= IL_node_create_goto ( l1,IL_NODE_LAB );
	IL_node * lab2= IL_node_create_goto ( l2,IL_NODE_LAB );
	IL_node * lab3= IL_node_create_goto ( l3,IL_NODE_LAB );
	IL_node * lab4= IL_node_create_goto ( l4,IL_NODE_LAB );
	TokenInfo t_k;
    token_Get(pos,&t_k);
	{
		/*允许for语句的首个表达式出现定义语句,*/
		/*所以要检查有无 var关键字*/
		int test=*pos;
        token_Get ( &test,&t_k );
		/*有var关键字?那么跳过*/
		if ( t_k.type==TOKEN_TYPE_VAR_DEF )
		{
			( *pos ) =test;
		}
	}
	exp_Parse ( pos,EXP_NORMAL,layer ); /*解析exp1*/
	int exp_2_pos = ( *pos ); /*保存exp2的位置,以待之后解析*/
	int exp_3_pos=exp_2_pos;/*第三个表达式的位置待以后解析*/
	/*略过exp2*/
	do
	{
        token_Get ( &exp_3_pos,&t_k );
	}
	while ( t_k.type!=TOKEN_TYPE_SEMICOLON );
	int postion=exp_3_pos;/*储存正式语句的位置*/


	IL_ListInsertNode ( n3 );
	IL_ListInsertNode ( lab1 );
        /*略过表达式3*/
    do
    {
        (*pos)=postion;
        token_Get ( &postion,&t_k );
    }
    while ( t_k.type!=TOKEN_TYPE_LEFT_BRACE );
    int result =block_Parse(pos,layer,l4,l2,mode);
    if(result!=0)
    {
        STOP("illegal for statement %s",block_GetLastStateStr ());
    }
	IL_ListInsertNode ( lab2 );
	/*注意这里的两个表达式都属于for内层的语句,但是因为是在*/
	/*跳出了for的括号之后才解析的,所以layer需要增加*/
	exp_Parse ( &exp_3_pos,EXP_CONTROL,layer+1 );
	IL_ListInsertNode ( lab3 );
	exp_Parse ( &exp_2_pos,EXP_NORMAL,layer+1 );

	IL_ListInsertNode ( n1 );
	IL_ListInsertNode ( lab4 );
}

