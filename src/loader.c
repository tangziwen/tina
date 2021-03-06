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
#include <string.h>
#include "loader.h"
char * buffer =NULL;
int buffer_size=0;
static char buffer_file_name[32];
/*获得当前被读取的文件名*/
const char * loader_GetFileName()
{
    return buffer_file_name;
}


/*输入指定的字符位置，返回其所在当前被读入的buffer的行数*/
int loader_GetLineNumber(int pos)
{
    int lines=1;
    int i=0;
    for( ;i<=pos;i++)
    {
        if(buffer[i]=='\n')
        {
            lines++;
        }
    }
    return lines;
}

void loader_load_buf(const char * file_name)
{
	FILE * buffer_file;
	buffer_file= fopen(file_name,"rb");
    strcpy(buffer_file_name,file_name);
    if(!buffer_file)
    {
        STOP("invalid file input!!! %s\n",file_name);
    }
	fseek(buffer_file,0,SEEK_END);
	buffer_size =ftell(buffer_file);
	rewind(buffer_file);
	buffer = (char*)malloc((buffer_size+1)*sizeof(char));
	{
		int i;
		for(i=0; !feof(buffer_file); i++)
		{
			fread(buffer+i,sizeof(char),1,buffer_file);
			if(buffer[i]=='\r')
			{
				buffer[i]=' ';
			}
		}
	}
	buffer[buffer_size]='\0';
    fclose(buffer_file);
}

