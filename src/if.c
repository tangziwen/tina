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


#include <stdlib.h>
#include <stdio.h>
#include "tina.h"
#include "print.h"
#include "token.h"
#include "if.h"
#include "var.h"
#include "expression.h"
#include "il.h"
#include "while.h"
#include "return.h"
#include "for.h"
#include "function.h"
#include "block.h"
void if_parse( int* pos, int layer, int break_label, int continue_label ,int mode)
{
	TokenInfo t_k;
    token_Get ( pos,&t_k );/*略过if的判断表达式的左小括号*/
	if(t_k.type!=TOKEN_TYPE_OP && t_k.content[0]!='(')
	{
		printf("error ,miss '(' in while expression\n");
		exit(0);
	}
	/*先解析if之后的条件表达式*/
	exp_Parse(pos,EXP_CONTROL,layer);
	int l1=label_index+1;
	int l2 =l1+1;
	label_index+=2;
	/*第一个跳转*/
	IL_node * n1= IL_node_create_goto(l1,IL_NODE_JE);
	IL_node * n2 = IL_node_create_goto(l2,IL_NODE_GOTO);
	/*创建与第一个跳转对应的标记*/
	IL_node * lab1= IL_node_create_goto(l1,IL_NODE_LAB);
	IL_node * lab2= IL_node_create_goto(l2,IL_NODE_LAB);
	IL_ListInsertNode(n1);
    {
    int result =block_Parse(pos,layer,break_label,continue_label,mode);
    if(result!=0)
    {
        STOP("illegal if statement %s",block_GetLastStateStr ());
    }
    }
	IL_ListInsertNode(n2);
	IL_ListInsertNode(lab1);
	/*检查是否有else语句*/
	/*是else就往前执行,不是else的话要退回一格*/
	/*因为已经读取了不该读取的词法单元,这里用postion作临时*/
	{
        int postion =*pos;
		TokenInfo t_k ;
        token_Get(&postion,&t_k);
		if(t_k.type==TOKEN_TYPE_ELSE)
		{
        (*pos)=postion;
           int result= block_Parse(pos,layer,break_label,continue_label,mode);
           if(result!=0)
           {
               STOP("illegal else %s",block_GetLastStateStr ());
           }
		}
	}
	/*插入第二次跳转标记*/
	IL_ListInsertNode(lab2);
}
