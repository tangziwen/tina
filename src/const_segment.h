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
#ifndef TINA_CONST_SEGMENT_H
#define TINA_CONST_SEGMENT_H
#include "var.h"
/*将指定的变量加入常量段,并返回其索引*/
int ConstSegmentPush(Var const_var);
/*返回当前常量段中拥有的常量的数量*/
int ConstSegmentGetCount();
/*返回指定索引的常量段中的变量*/
Var * ConstSegmentGetVar(int index);

/*把常量段写入字节码文件中*/
void ConstSegmentWrite(FILE *f);

/*把常量段从字节码中读取到内存中*/
void ConstSegmentLoad(char *str);

/*清除已有的记录，方便下次编译*/
void ConstSegmentDump();
#endif // CONST_SEGMENT_H
