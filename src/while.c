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
	token_get ( pos,&t_k );//略过while的判断表达式的左小括号
	if(t_k.type!=TOKEN_TYPE_OP && t_k.content[0]!='(')
		{
			printf("error ,miss '(' in while expression\n");
			exit(0);
		}
	int exp_pos = ( *pos ); //保留表达式的位置,稍后再解析
	int brace =-1;
	//parse_exp(pos,EXP_CONTROL);
	int l1=label_index+1;
	int l2 =l1+1;
	int l3 =l2+1;
	label_index+=3;
	//第一个跳转
	IL_node * n1= IL_node_create_jmp ( l1,IL_NODE_JNE );
	IL_node * n2 = IL_node_create_jmp ( l2,IL_NODE_JMP );
	IL_node * n3 =IL_node_create_jmp ( l3,IL_NODE_JMP );
	//创建与第一个跳转对应的标记
	IL_node * lab1= IL_node_create_jmp ( l1,IL_NODE_LAB );
	IL_node * lab2= IL_node_create_jmp ( l2,IL_NODE_LAB );
	IL_node * lab3= IL_node_create_jmp ( l3,IL_NODE_LAB );
	IL_ListInsertNode ( n2 );
	IL_ListInsertNode ( lab1 );
	int postion = ( *pos );

	layer++;
	token_get ( &postion,&t_k );
	//跳过条件表达式,先解析语句
	while ( t_k.type!=TOKEN_TYPE_LEFT_BRACE )
		{
			token_get ( &postion,&t_k );
		}
	while ( 1 )
		{
			TokenInfo t_k;
			//因为需要更多的信息,所以我们有两个变量控制
			//步进关系
			int test_pos=postion;
			token_get ( &test_pos,&t_k );

			switch ( t_k.type )
				{

				case TOKEN_TYPE_EOF:
					exit ( 0 );
					break;
				case TOKEN_TYPE_BREAK:
					postion=test_pos;
					token_get(&postion,&t_k);
					IL_ListInsertNode(n3);
					break;
				case TOKEN_TYPE_CONTINUE:
				{
					postion=test_pos;
					token_get(&postion,&t_k);
					IL_node * node_continue=IL_node_create_jmp ( l2,IL_NODE_JMP );
					IL_ListInsertNode(node_continue);
				}
				break;
				case TOKEN_TYPE_SELF:
					if(mode ==FUNC_GLOBAL)
						{
							printf("can not use \"self\" in global function \n");
							exit(0);
						}
					else
						{
							if(mode==FUNC_MEMBER)
								{
									exp_Parse(&postion,EXP_NORMAL,layer);
								}

						}
					break;
				case TOKEN_TYPE_PRINT:
					postion =test_pos;
					print_parse ( &postion,layer );
					break;
				case TOKEN_TYPE_VAR_DEF:
					postion=test_pos;
					exp_Parse ( &postion,EXP_NORMAL,layer );
					break;
				case TOKEN_TYPE_TRUE:
					//回退一个,解析表达式
					exp_Parse ( &postion,EXP_NORMAL,layer );
					break;
				case TOKEN_TYPE_FALSE:
					//回退一个,解析表达式
					exp_Parse ( &postion,EXP_NORMAL,layer );
					break;
				case TOKEN_TYPE_NUM:
					//回退一格,解析表达式
					exp_Parse ( &postion,EXP_NORMAL,layer );
					break;
				case TOKEN_TYPE_SYMBOL:
					//回退一个,解析表达式
					exp_Parse ( &postion,EXP_NORMAL,layer );
					break;
				case TOKEN_TYPE_RETURN://解析返回值表达式
					postion=test_pos;
					return_parse ( &postion,layer );
					break;
				case TOKEN_TYPE_API:
					//回退一个,解析表达式
					exp_Parse ( &postion,EXP_NORMAL,layer );
					break;
				case TOKEN_TYPE_STRUCT_NAME:
					//回退一个,解析表达式
					exp_Parse(&postion,EXP_NORMAL,layer);
					break;
				case TOKEN_TYPE_FUNC:
					//回退一个,解析表达式
					exp_Parse ( &postion,EXP_NORMAL,layer );
					break;
					//遇到左括号,当前层次增加
				case TOKEN_TYPE_LEFT_BRACE:
					brace--;
					postion=test_pos;
					layer++;
					break;
					//遇到右括号当前层次减少,整段的销毁原先最内层次的变量.
				case TOKEN_TYPE_RIGHT_BRACE:
					brace++;
					layer--;
					postion=test_pos;
					if ( brace==0 )
						{

							goto end;
						}
					break;
					//解析if语句
				case TOKEN_TYPE_IF:
					postion =test_pos;
					//这里直接修改外部的位置了
					if_parse( &postion,layer,l3,l2,mode );
					break;
				case TOKEN_TYPE_FOR:
					postion =test_pos;
					for_parse ( &postion,layer ,mode);
					break;
				case TOKEN_TYPE_WHILE:
					postion =test_pos;
					while_parse ( &postion,layer,mode );
					break;
				case TOKEN_TYPE_OP:
					//如果是左小括号的话,回退一格,进行表达式求值,如果不是
					//则是一个编译错误
					if ( t_k.content[0]=='(' )
						{
							exp_Parse ( &postion,EXP_NORMAL,layer );
						}
					else
						{
							printf ( "illigal \" ( \" \n" );
							exit ( 0 );
						}
					break;
				default :
					printf ( "error !!\n" );
					printf ( "tok %d is unknown %s\n",t_k.type,t_k.content );
					exit ( 0 );
					break;
				}
		}
end:
	;
	( *pos ) =postion;
	IL_ListInsertNode ( lab2 );
	exp_Parse ( &exp_pos,EXP_CONTROL,layer );
	IL_ListInsertNode ( n1 );
	IL_ListInsertNode ( lab3);
}
