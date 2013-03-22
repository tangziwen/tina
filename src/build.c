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
#include "type.h"
#include "il.h"
#include "build.h"
#include "loader.h"
#include "preprocess.h"
#include "token.h"
#include "var.h"
#include "function.h"
#include  "module.h"
#include "const_segment.h"
#include "script_struct.h"
#define MAX_KEYWORD_LIST 17
#define BYTE_KEYWORD_CONST 0
#define BYTE_KEYWORD_FUNC 1
#define BYTE_KEYWORD_EXP 2
#define BYTE_KEYWORD_PRINT 3
#define BYTE_KEYWORD_RETURN 4
#define BYTE_KEYWORD_CALLFUNC 5
#define BYTE_KEYWORD_LABEL 6
#define BYTE_KEYWORD_JE 7
#define BYTE_KEYWORD_JMP 8
#define BYTE_KEYWORD_JNE 9
#define BYTE_KEYWORD_DYNAMIC 10
#define BYTE_KEYWORD_STRUCT 11
#define BYTE_KEYWORD_MEMBER 12
#define BYTE_KEYWORD_METHOD 13
#define BYTE_KEYWORD_STRUCT_CREATOR 14
#define BYTE_KEYWORD_VECTOR 15
#define BYTE_KEYWORD_TUPLE 16
static char * ByteCodeKeyWordList[MAX_KEYWORD_LIST]={"C","F",
                                                    "EXP","PRNT",
                                                     "RETRN","CALLF",
                                                     "LABEL","JE",
                                                     "JMP","JNE",
                                                     "DYNAMIC","S",
                                                     "M","METHOD",
                                                     "STRUCT_CREATOR","VECTOR","TUPLE"};

static int LookUpByteCodeType(char *str)
{
    char str_type[128];
    sscanf (str,"%s",str_type);
    int i=0;
    for ( i=0; i<MAX_KEYWORD_LIST; i++ )
    {
        if ( strcmp ( ByteCodeKeyWordList[i],str_type) ==0 )
        {
            return i;
        }
    }
    return -1;
}


static void ParseByteCodeLine(char *str)
{
    int type=LookUpByteCodeType(str);
    switch(type)
    {
    case BYTE_KEYWORD_CONST:
        ConstSegmentLoad(str);
        break;
    case BYTE_KEYWORD_EXP:
        IL_ExpLoad(str);
        break;
    case BYTE_KEYWORD_FUNC:
        func_Load(str);
        break;
    case BYTE_KEYWORD_PRINT:
         IL_PRNTLoad();
        break;
    case BYTE_KEYWORD_RETURN:
        IL_RETRNLoad();
        break;
    case BYTE_KEYWORD_CALLFUNC:
         IL_CallFuncLoad(str);
        break;
    case BYTE_KEYWORD_LABEL:
        IL_LableLoad (str);
        break;
    case BYTE_KEYWORD_JE:
        IL_JELoad (str);
        break;
    case BYTE_KEYWORD_JMP:
        IL_JMPLoad (str);
        break;
    case BYTE_KEYWORD_JNE:
        IL_JNELoad (str);
        break;
    case BYTE_KEYWORD_DYNAMIC:
        IL_DynamicLoad (str);
        break;
    case BYTE_KEYWORD_STRUCT:
        struct_Load (str);
        break;
    case BYTE_KEYWORD_MEMBER:
        struct_MemberLoad(str);
        break;
    case BYTE_KEYWORD_METHOD:
        IL_MethodCallLoad(str);
        break;
    case BYTE_KEYWORD_STRUCT_CREATOR:
        IL_StructCreatorLoad(str);
        break;
    case BYTE_KEYWORD_VECTOR:
        IL_VectorCreatorLoad (str);
        break;
    case BYTE_KEYWORD_TUPLE:
        IL_TupleCreatorLoad (str);
        break;
    default:
        printf("this kind of byte code not support yet");
        break;
    }
}

/*编译成字节码文件*/
static void Compile(const char *file_name)
{
    /*添加.tzw的后缀名*/
    char file_tzw[128];
    strcpy (file_tzw,file_name);
    strcat(file_tzw,".tzw");
    FILE * f=fopen (file_tzw,"w");

    //常量写入
    ConstSegmentWrite(f);

    //结构体写入
    struct_Compile(f);

    //函数写入
    func_Compile(f);

    fclose(f);
}

/*载入一个字节码文件*/
void Tina_Load(const char *file_name)
{
    /*添加.tzw的后缀名*/
    char file_tzw[128];
    strcpy (file_tzw,file_name);
    strcat(file_tzw,".tzw");
    FILE * f=fopen (file_tzw,"r");
    if(f==NULL)
    {
        STOP("INVALID BYTE CODE FILE");
    }
    //逐行读入字节码

    for(; ;){

        char str_buff[128];
        memset (str_buff,0,sizeof str_buff);
        fgets(str_buff,128,f);
                if(feof(f)!=0) break;
        ParseByteCodeLine(str_buff);
    }
}

void Tina_Build(const char *file_name)
{
    char file_tina[128];
    strcpy (file_tina,file_name);
    strcat(file_tina,".tina");
    loader_load_buf(file_tina);
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
    /*编译成字节码*/
    Compile(file_name);
}
