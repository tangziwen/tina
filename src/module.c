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
/*
此文件用以实现tina的模块机制

*/
#include <string.h>
#include "module.h"
#include "token.h"
#include "script_struct.h"
#include "function.h"
/*模块名,默认情况下表示为全局*/
static  char module_context_name[100]= {0};

static int module_index=0;
/*模块列表*/
static char module_list[10][100]= {0};

static void set_context(char * str);

/*获得当前一共创建了的模块数量*/
int module_GetMoudleCount()
{
return module_index;
}

void module_parse_using(int * pos)
{
	TokenInfo t_k;
	do
	{
		token_Get(pos,&t_k);
		switch(t_k.type)
		{
			/*拷入要导入的包*/
		case TOKEN_TYPE_SYMBOL:
            module_index++;
			strcpy(module_list[module_index],t_k.content);
			break;
		case TOKEN_TYPE_COMMA:
			break;
		case TOKEN_TYPE_SEMICOLON:
			break;
		case TOKEN_TYPE_EOF:
			break;
		default:
			printf("illigal import !\n");
			exit(0);
			break;
		}
	}
	while(t_k.type!=TOKEN_TYPE_EOF && t_k.type!=TOKEN_TYPE_SEMICOLON);
}



/*扫描模块*/
void module_parse_declare(int * pos)
{
	TokenInfo t_k;
	token_Get(pos,&t_k);
	/*更新模块名*/
	set_context(t_k.content);
	/*跳过左边的花括号*/
	token_Get(pos,&t_k);
	int brace=-1;
	{
		/*扫描模块内部的声明*/
		do
		{
			token_Get(pos,&t_k);
			switch(t_k.type)
			{
			case TOKEN_TYPE_EOF:
				break;
			case TOKEN_TYPE_LEFT_BRACE:
				brace--;
				break;
			case TOKEN_TYPE_RIGHT_BRACE:
				brace++;
				if(brace==0)
				{
					t_k.type=TOKEN_TYPE_EOF;
				}
				break;
				/*发现函数的定义符号,声明函数*/
			case TOKEN_TYPE_FUNC_DEF:
				func_ParseDeclare(pos);
				break;
				/*发现类的定义，声明类*/
			case TOKEN_TYPE_STRUCT:
				struct_ParseDeclare(pos);
				token_SkipBlock(pos);/*略过类定义块*/
				break;
			default :
				break;
			}
		}
		while(t_k.type!=TOKEN_TYPE_EOF);
	}
	/*将当前模块重置为全局*/
	set_context("\0");
}
/*扫描模块的定义*/
void module_parse_def(int * pos)
{
	TokenInfo t_k;
	token_Get(pos,&t_k);
	/*更新模块名*/
	set_context(t_k.content);
	token_Get(pos,&t_k);
	int brace=-1;
	/*第二遍扫描,扫描所有函数以及类的定义*/
	{
		do
		{
			token_Get(pos,&t_k);
			switch(t_k.type)
			{
			case TOKEN_TYPE_EOF:
				break;
				/*发现函数的定义符号,定义函数*/
			case TOKEN_TYPE_FUNC_DEF:
				func_ParseDef(pos);
				break;
			case TOKEN_TYPE_USING:
				module_parse_using(pos);
				break;
				/*发现类的定义*/
			case TOKEN_TYPE_STRUCT:
				struct_ParseDefine(pos);/*解析类*/
				break;
			case TOKEN_TYPE_LEFT_BRACE:
				brace--;
				break;
			case TOKEN_TYPE_RIGHT_BRACE:
				brace++;
				if(brace==0)
				{
					t_k.type=TOKEN_TYPE_EOF;
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
	}
	/*将当前模块重置为全局*/
	set_context("\0");
}

/*查询当前符号被模块修饰后的名字*/
char * module_MangledName(int module_id,const char * the_symbol)
{
	static char  result[100];
	memset(result,0,sizeof(result));
	strcpy(result,module_list[module_id]);
	strcat(result,"#");
	strcat(result,the_symbol);
	return result;
}


/*查询当前符号在当前上下文模块的修饰后的名字*/
char * module_ContextMangledName(const char * the_symbol)
{
	static char  result[100];
	memset(result,0,sizeof(result));
	if(strcmp(module_context_name,"\0")!=0)
	{
		strcpy(result,module_context_name);
		strcat(result,"#");
		strcat(result,the_symbol);
	}
	else
	{
		strcpy(result,the_symbol);
	}
	return result;
}
static void set_context(char * str)
{
	strcpy(module_context_name,str);
}


