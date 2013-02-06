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
#include "il.h"
#include "token.h"
#include "var.h"
#include "expression.h"
static int is_int(char * str)
{
	int i=0;
	while(str[i]!='\0')
		{
			if(str[i]>'9' || str[i]<'0')
				{
					return -1;
				}
			i++;
		}
	return 1;
}

void print_parse(int *pos,int layer)
{
	exp_Parse(pos,EXP_NORMAL,layer);
	IL_ListInsertNode( IL_node_create_prnt());
}
