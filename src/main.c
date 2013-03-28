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
//#define NDEBUG
#include "debug.h"



void SayHello()
{
    printf("holla you boy\n");
}


/*解释器主函数*/
/*如果未加说明则解释执行本地的test.tina脚本*/
int main ( int argc,char * argv[] )
{

    Tina_Init();
    switch(argc)
    {
    case 1:
    {
        printf("Tina is a dynamic type script language compiler & virtual machine\n");
        Tina_Load ("a");
        Tina_Load ("main");
        Tina_Run ("main");
    }
        break;
    case 3:
    {
        /*单个文件编译*/
        if(strcmp (argv[1],"-c")==0||strcmp (argv[1],"--compile")==0)
        {
            Tina_Compile(argv[2]);
            break;
        }
        /*文件链执行*/
        if(strcmp (argv[1],"-x")==0 ||strcmp (argv[1],"--excute")==0)
        {
            Tina_ExcuteByteCodeList(argv[2]);
            break;
        }
        /*单个字节码文件执行*/
        if(strcmp (argv[1],"-r") ==0||strcmp (argv[1],"--run")==0)
        {
            Tina_Load (argv[2]);
            Tina_Run ("main");
            break;
        }
        /*文件链编译*/
        if(strcmp (argv[1],"-b") ==0||strcmp (argv[1],"--build")==0)
        {
            Tina_Buid (argv[2]);
            break;
        }
        STOP("INVALID ARG");
    }
        break;
    default:
        STOP("INVALID ARG!");
        break;
    }
    return 0;
}
















