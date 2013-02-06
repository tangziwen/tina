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
#include <string.h>
#include "print.h"
#include "type.h"
#include "expression.h"
#include "function.h"
#include "var.h"
#include "il.h"
#include "return.h"
#include "while.h"
#include "if.h"
#include "for.h"
#include "module.h"
int func_index=0;
//当前的被扫描函数
Function * current_func=NULL;


//通过名称检索函数,如果存在,返回索引
//如果不存在,则返回-1
static int plain_get_index_by_name(  char * func_name)
{
	int i;
	for(i=0; i<func_index; i++)
		{
			if(strcmp(function_list[i].name,func_name)==0)
				{
					return i;
				}
		}
	return -1;
}
int func_get_index_by_name( char * func_name)
{
	char name[128]= {0};
	int result=-1, i;
	//先检查本模块内部
	strcpy(name,module_ContextMangledName(func_name));
	result=plain_get_index_by_name(name);
	if(result>=0)
		{
			return result;
		}

	//再检查导入模块是否有该名字
	for(i=0; i<module_index; i++)
		{
			result=plain_get_index_by_name(module_MangledName(i,func_name));
			if(result>=0)
				{
					return result;
				}
		}
	//最后进行全局检查,用户使用完全限定的标识符也在此类
	if(result<0)
		{
			result=plain_get_index_by_name(func_name);
		}
	return result;
}

//获得当前已经被解析过的函数个数
int Tina_FuncGetCount()
{
	return func_index;
}
//解析函数的声明
void func_ParseDeclare(int *postion)
{
	TokenInfo t_k;
	token_get(postion,&t_k);//获取函数名称
	//加上模块前缀
	char  unique_name[32]= {0};
	strcpy(unique_name,module_ContextMangledName(t_k.content));
	//检查有无重名函数
	{
		int i;
		for(i=0; i<func_index; i++)
			{
				if(strcmp(function_list[i].name,unique_name)==0)
					{
						printf("error !! the function has already been defined!!\n");
						exit(0);
					}
			}
	}
	//创建函数
	strcpy(function_list[func_index].name,unique_name);
	func_index++;
}

//解析方法（成员函数）的声明
void method_parse_declare(int *postion,char * class_name)
{
	TokenInfo t_k;
	token_get(postion,&t_k);//获取函数名称
	//加上后缀
	strcat(t_k.content,"#");
	strcat(t_k.content,class_name);
	//检查有无重名函数
	{
		int i;
		for(i=0; i<func_index; i++)
			{
				if(strcmp(function_list[i].name,t_k.content)==0)
					{
						printf("error !! the function has already been defined!!\n");
						exit(0);
					}
			}
	}
	//创建函数
	strcpy(function_list[func_index].name,t_k.content);
	func_index++;
}

Function * func_get_by_index(int index)
{
	return &(function_list[index]);
}

Var func_invoke(int index)
{
	return IL_exec(&function_list[index]);
}
int is_parsing_func_def=0;

//解析函数的定义
Function * func_parse_def(int *postion )
{
	is_parsing_func_def=1;
	TokenInfo t_k;
	//获得函数的名字
	token_get(postion,&t_k);
	//我们已经预扫描过了函数的声明.现在我们直接通过函数的名字获取该函数的结构
	Function *f =func_get_by_index(func_get_index_by_name(t_k.content));
	int index =func_get_index_by_name(t_k.content);
	//设置该函数为当前扫描函数
	func_set_current(f);
	//解析参数形如  (a,b,c)
	//我们把参数当作函数的最外层的最前面的
	//几个变量.
	token_get(postion,&t_k);
	if(t_k.type!=TOKEN_TYPE_OP&&t_k.content[0]!='(')
		{
			printf("error miss '(' in function args list \n");
			exit(0);
		}
	int type=VAR_TYPE_INT;
	do
		{
			token_get(postion,&t_k);
			switch(t_k.type)
				{
				case TOKEN_TYPE_SYMBOL:
				{
					int i=0;
					for(i=0; i<f->var_counts; i++)
						{
							if(strcmp(t_k.content,f->var_list[i].name)==0)
								{
									printf("error the args name conflict!!!\n");
									exit(0);
								}
						}
					strcpy(f->var_list[f->var_counts].name
					       , t_k.content);
					f->var_list[f->var_counts].content.type=type;
					f->var_list[f->var_counts].layer=0;
					f->arg_counts++;

					f->var_counts++;
					type=VAR_TYPE_INT;
				}
				break;
				case TOKEN_TYPE_REF:
					type=VAR_TYPE_REF;
					break;
				case TOKEN_TYPE_COMMA:
					break;
				case TOKEN_TYPE_RIGHT_PARENTHESIS:
					goto end;
					break;

				default:
					printf("error in function arg list \n");
					exit(0);
					break;
				}

		}
	while(t_k.type!=TOKEN_TYPE_RIGHT_PARENTHESIS);
end:
	;

	int current_layer=0;
	int scan_pos=*postion;
	var_parse_local(&scan_pos,f->arg_counts);//初始化变量模块
	token_get_assert(postion,&t_k,TOKEN_TYPE_LEFT_BRACE,"invalid function body");//跳过一个左花括号
	int brace =-1;
	do
		{
			//因为需要更多的信息,所以我们有两个变量控制
			//步进关系
			int test_pos=*postion;
			token_get(&test_pos,&t_k);
			switch(t_k.type)
				{
				case TOKEN_TYPE_EOF:
					break;
				case TOKEN_TYPE_VAR_DEF:

					(*postion)=test_pos;
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_NUM:
					//回退一格,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_SELF:
					printf("can not use \"self\" in global function \n");
					exit(0);
					break;
				case TOKEN_TYPE_SYMBOL:

					;
					//回退一个,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_TRUE:
					//回退一个,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_RETURN://解析返回值表达式
					(*postion)=test_pos;
					return_parse(postion,current_layer);
					break;
				case TOKEN_TYPE_FALSE:
					//回退一个,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_API:
					//回退一个,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_STRUCT_NAME:
					//回退一个,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_FUNC:
					//回退一个,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
					//遇到左括号,当前层次增加
				case TOKEN_TYPE_LEFT_BRACE:
					(*postion)=test_pos;
					current_layer++;
					brace--;
					break;
					//遇到右括号当前层次减少,整段的销毁原先最内层次的变量.
				case TOKEN_TYPE_RIGHT_BRACE:
					(*postion)=test_pos;
					current_layer--;
					brace++;
					//括号平衡的话,则函数定义结束
					if(brace==0)
						{
							is_parsing_func_def=0;
							return f;
						}
					break;
				case TOKEN_TYPE_WHILE:
					(*postion) =test_pos;
					while_parse(postion,current_layer,FUNC_GLOBAL);
					break;
				case TOKEN_TYPE_FOR:
					(*postion)=test_pos;
					for_parse(postion,current_layer,FUNC_GLOBAL);
					break;
					//解析if语句
				case TOKEN_TYPE_IF:
					(*postion) =test_pos;
					if_parse(postion,current_layer,-1,-1,FUNC_GLOBAL);
					break;
				case TOKEN_TYPE_PRINT:
					(*postion) =test_pos;
					print_parse(postion,current_layer);
					break;
				case TOKEN_TYPE_OP:
					//如果是左小括号的话,回退一格,进行表达式求值,如果不是
					//则是一个编译错误
					if(t_k.content[0]=='(')
						{
							exp_Parse(postion,EXP_NORMAL,current_layer);
						}
					else
						{
							printf("illigal '(' \n");
							exit(0);
						}
					break;
				default :
					printf("the token %d is unknown %s\n",t_k.type,t_k.content);
					printf("error !!\n");
					exit(0);
					break;
				}
		}
	while(t_k.type!=TOKEN_TYPE_EOF);
	return f;
}


//解析方法（成员函数）的定义
Function * method_parse_def(int *postion,char * class_name )
{
	is_parsing_func_def=1;
	TokenInfo t_k;
	token_get(postion,&t_k);
	strcat(t_k.content,"#");
	strcat(t_k.content,class_name);

	//我们已经预扫描过了函数的声明.现在我们直接通过函数的名字获取该函数的结构
	Function *f =func_get_by_index(func_get_index_by_name(t_k.content));
	int index =func_get_index_by_name(t_k.content);
	//设置该函数为当前扫描函数
	func_set_current(f);
	//解析参数形如  (a,b,c)
	//我们把参数当作函数的最外层的最前面的
	//几个变量.
	token_get(postion,&t_k);
	if(t_k.type!=TOKEN_TYPE_OP&&t_k.content[0]!='(')
		{
			printf("error miss '(' in function args list \n");
			exit(0);
		}
	int type=VAR_TYPE_INT;
	do
		{
			token_get(postion,&t_k);
			switch(t_k.type)
				{
				case TOKEN_TYPE_SYMBOL:
				{
					int i=0;
					for(i=0; i<f->var_counts; i++)
						{
							if(strcmp(t_k.content,f->var_list[i].name)==0)
								{
									printf("error the args name conflict!!!\n");
									exit(0);
								}
						}
					strcpy(f->var_list[f->var_counts].name
					       , t_k.content);
					f->var_list[f->var_counts].content.type=type;
					f->var_list[f->var_counts].layer=0;
					f->arg_counts++;

					f->var_counts++;
					type=VAR_TYPE_INT;
				}
				break;
				case TOKEN_TYPE_REF:
					type=VAR_TYPE_REF;
					break;
				case TOKEN_TYPE_COMMA:
					break;
				case TOKEN_TYPE_RIGHT_PARENTHESIS:
					goto end;
					break;

				default:
					printf("error in function arg list \n");
					exit(0);
					break;
				}

		}
	while(t_k.type!=TOKEN_TYPE_RIGHT_PARENTHESIS);
end:
	;

	int current_layer=0;
	int scan_pos=*postion;
	var_parse_local(&scan_pos,f->arg_counts);//初始化变量模块
	token_get_assert(postion,&t_k,TOKEN_TYPE_LEFT_BRACE,"invalid function body");//跳过一个左花括号
	int brace =-1;
	do
		{
			//因为需要更多的信息,所以我们有两个变量控制
			//步进关系
			int test_pos=*postion;
			token_get(&test_pos,&t_k);
			switch(t_k.type)
				{
				case TOKEN_TYPE_EOF:
					break;
				case TOKEN_TYPE_VAR_DEF:

					(*postion)=test_pos;
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_NUM:
					//回退一格,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_SELF:
					//回退一格,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_SYMBOL:

					;
					//回退一个,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_TRUE:
					//回退一个,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_RETURN://解析返回值表达式
					(*postion)=test_pos;
					return_parse(postion,current_layer);
					break;
				case TOKEN_TYPE_FALSE:
					//回退一个,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_API:
					//回退一个,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_STRUCT_NAME:
					//回退一个,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
				case TOKEN_TYPE_FUNC:
					//回退一个,解析表达式
					exp_Parse(postion,EXP_NORMAL,current_layer);
					break;
					//遇到左括号,当前层次增加
				case TOKEN_TYPE_LEFT_BRACE:
					(*postion)=test_pos;
					current_layer++;
					brace--;
					break;
					//遇到右括号当前层次减少,整段的销毁原先最内层次的变量.
				case TOKEN_TYPE_RIGHT_BRACE:
					(*postion)=test_pos;
					current_layer--;
					brace++;
					//括号平衡的话,则函数定义结束
					if(brace==0)
						{
							is_parsing_func_def=0;
							return f;
						}
					break;
				case TOKEN_TYPE_WHILE:
					(*postion) =test_pos;
					while_parse(postion,current_layer,FUNC_MEMBER);
					break;
				case TOKEN_TYPE_FOR:
					(*postion)=test_pos;
					for_parse(postion,current_layer,FUNC_MEMBER);
					break;
					//解析if语句
				case TOKEN_TYPE_IF:
					(*postion) =test_pos;
					if_parse(postion,current_layer,-1,-1,FUNC_MEMBER);
					break;
				case TOKEN_TYPE_PRINT:
					(*postion) =test_pos;
					print_parse(postion,current_layer);
					break;
				case TOKEN_TYPE_OP:
					//如果是左小括号的话,回退一格,进行表达式求值,如果不是
					//则是一个编译错误
					if(t_k.content[0]=='(')
						{
							exp_Parse(postion,EXP_NORMAL,current_layer);
						}
					else
						{
							printf("illigal '(' \n");
							exit(0);
						}
					break;
				default :
					printf("the token %d is unknown %s\n",t_k.type,t_k.content);
					printf("error !!\n");
					exit(0);
					break;
				}
		}
	while(t_k.type!=TOKEN_TYPE_EOF);
	return f;
}

//运行一个函数
void Tina_Run(const char *name)
{
	func_invoke(func_get_index_by_name(name));
}


void func_set_current(Function * func)
{
	current_func=func;
}


Function * func_get_current()
{
	return current_func;
}

Var ScriptFuncArgList[32]={0};
//设置脚本函数的参数
void Tina_SetScriptFuncArg(Var arg,int id)
{
ScriptFuncArgList[id]=arg;
}

//调用脚本函数
Var Tina_CallScriptFunc(int func_id)
{
	Var result;
	vm_rt_stack_push();
	Function * f =func_get_by_index ( func_id);
	//为函数的实参赋值
	int i;
	for ( i=0;
			i<f->arg_counts;
			i++)
		{
			vm_rt_stack_var_set ( i,ScriptFuncArgList[i]);
		}
	//保存当前的表环境
	IL_list * old_list =current_list;
	//调用
	result =func_invoke ( func_id);
	//环境恢复
	current_list=old_list;
	vm_rt_stack_pop();
	return result;
}

