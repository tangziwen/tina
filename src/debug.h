#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

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
证第三或更高版本的约束下再发布和(或)修改遵循GPL v3或更新的版本,
(对GPL许可证的细节有疑问可参看位于gpl-CN.txt里的非官方翻译)

此程序的发布是希望它有助于您,但是作者对其不做任何担保,即使是商业上或
合于特定用途的隐式担保亦无
*********************tzw <tzwtangziwen@gmail.com>******************************/
#ifndef NDEBUG
#define test(A) \
		do { \
		A \
		 } while(0)
#else
#define test(A) do{\
		;\
		}while(0)
#endif
#define alert(fmt,args...)  printf("File:%s    Function:%s    Line:%d\n>>"fmt, __FILE__, __func__, __LINE__, ##args)
#endif // DEBUG_H_INCLUDED
