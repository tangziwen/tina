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
#include "tina.h"
//解释器主函数
//如果未加说明则解释执行本地的test.tina脚本
int main ( int argc,char * argv[] )
{
	Tina_Init();
	switch(argc)
		{
		case 1:
			PRINT("please input file");
			//for quick test
			//Tina_Build ("test.tina");
			//Tina_Run ( "main" );
			STOP("tzw :tzwtangziwen@163.com");
			break;
		case 2:
			Tina_Build (argv[1]);
			Tina_Run ( "main" );
			break;
		default:
			STOP("INVALID ARG!");
			break;
		}
	return 0;
}
















