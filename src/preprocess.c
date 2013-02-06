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
#include "preprocess.h"
#include "loader.h"

static void clear_comments(char *buffer)
{

	int i;
	for( i=0; i<buffer_size; i++)
		{
			//遇见单行注释
			if(buffer[i]=='/' && buffer[i+1]=='/')
				{
					buffer[i]=' ';
					buffer[i+1]=' ';
					int j=i+2;
					while(buffer[j]!='\n')
						{
							buffer[j]=' ';
							j++;
						}
					i=j;
				}
			//遇见块注释
			if(buffer[i]=='/'&& buffer[i+1]=='*')
				{
					buffer[i]=' ';
					buffer[i+1] = ' ';
					int j =i+2;
					while(buffer[j]!='*' && buffer[j+1]!='/')
						{
							buffer[j]=' ';
							j++;
						}
					buffer[j]=' ';
					buffer[j+1]= ' ';
					i=j;
				}

		}

}

void preprocess( char * buffer)
{
	clear_comments(buffer);

}

