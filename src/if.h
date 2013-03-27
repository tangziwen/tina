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
#ifndef TINA_IF_H
#define TINA_IF_H
#include "def.h"
/*解析if语句,要求当前pos的初始位置从*/
/*if这个标号之后开始,他会修改原有的位置到整个:*/
/*if(..)*/
/*{.......}*/
/*else*/
/*{.......}*/
/*if代码块结束的位置之后*/
void if_parse(int * pos,int layer,int break_label,int continue_label,int mode);
#endif
