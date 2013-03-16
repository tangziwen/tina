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
#include "tina.h"
#include "type.h"
#include "build.h"
#include "loader.h"
#include "preprocess.h"
#include "token.h"
#include "var.h"
#include "function.h"
#include  "module.h"
void Tina_Build(const char *file_name)
{
	loader_load_buf(file_name);
	preprocess(buffer);

	{

		int postion =0;
		TokenInfo t_k;

		/*第一遍扫描,扫描所有的全局函数的声明*/
		do
		{
			/*因为需要更多的信息,所以我们有两个变量控制*/
			/*步进关系*/
			int test_pos=postion;
			token_get(&test_pos,&t_k);
			switch(t_k.type)
			{
				/*发现模块定义标识符，扫描模块的声明*/
			case TOKEN_TYPE_MODULE:
				postion =test_pos;
				module_parse_declare(&postion);
				break;
			case TOKEN_TYPE_EOF:
				break;
				/*发现全局的函数的定义符号,声明函数*/
			case TOKEN_TYPE_FUNC_DEF:
				postion =test_pos;
				func_ParseDeclare(&postion);
				break;
				/*发现全局的类的定义，声明类*/
			case TOKEN_TYPE_STRUCT:
				postion=test_pos;
				struct_ParseDeclare(&postion);
				token_SkipBlock(&postion);/*略过类定义块*/
				break;
			default :
				postion=test_pos;
				break;
			}
		}
		while(t_k.type!=TOKEN_TYPE_EOF);
	}

	/*第二遍扫描,扫描所有函数以及类的定义*/
	{
		int postion =0;
		TokenInfo t_k;
		do
		{
			/*因为需要更多的信息,所以我们有两个变量控制*/
			/*步进关系*/
			int test_pos=postion;
			token_get(&test_pos,&t_k);
			switch(t_k.type)
			{
			case TOKEN_TYPE_EOF:
				break;
				/*插入引用*/
			case TOKEN_TYPE_USING:
				postion =test_pos;
				module_parse_using(&postion);
				break;
				/*发现模块标识符，扫描模块内部的声明*/
			case TOKEN_TYPE_MODULE:
				postion =test_pos;
				module_parse_def(&postion);
				break;
				/*发现函数的定义符号,定义函数*/
			case TOKEN_TYPE_FUNC_DEF:
				postion =test_pos;
				func_parse_def(&postion);
				break;
				/*发现类的定义*/
			case TOKEN_TYPE_STRUCT:
				struct_ParseDefine(&test_pos);/*解析类*/
				postion =test_pos;
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
}