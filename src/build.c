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
#include "token.h"
#include "api.h"
#include "compile_error.h"
#define MAX_KEYWORD_LIST 20
#define BYTE_KEYWORD_CONST 0
#define BYTE_KEYWORD_FUNC 1
#define BYTE_KEYWORD_EXP 2
#define BYTE_KEYWORD_PRINT 3
#define BYTE_KEYWORD_RETURN 4
#define BYTE_KEYWORD_INVOKE_FUNCTION 5
#define BYTE_KEYWORD_LABEL 6
#define BYTE_KEYWORD_JE 7
#define BYTE_KEYWORD_GOTO 8
#define BYTE_KEYWORD_JNE 9
#define BYTE_KEYWORD_DYNAMIC 10
#define BYTE_KEYWORD_STRUCT 11
#define BYTE_KEYWORD_MEMBER 12
#define BYTE_KEYWORD_METHOD 13
#define BYTE_KEYWORD_STRUCT_CREATOR 14
#define BYTE_KEYWORD_VECTOR 15
#define BYTE_KEYWORD_TUPLE 16
#define BYTE_KEYWORD_IMPORT 17
#define BYTE_KEYWORD_CARD 18
#define BYTE_KEYWORD_TYPE_OF 19
static char * ByteCodeKeyWordList[MAX_KEYWORD_LIST]={"C","F",
                                                     "E","P",
                                                     "R","i",
                                                     "L","JE",
                                                     "G","JNE",
                                                     "D","S",
                                                     "m","M",
                                                     "c","V",
                                                     "T","I",
                                                     "CA","t"};
static void OffsetReset();

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
    case BYTE_KEYWORD_INVOKE_FUNCTION:
        IL_CallFuncLoad(str);
        break;
    case BYTE_KEYWORD_LABEL:
        IL_LableLoad (str);
        break;
    case BYTE_KEYWORD_JE:
        IL_JELoad (str);
        break;
    case BYTE_KEYWORD_GOTO:
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
    case BYTE_KEYWORD_TUPLE:
        IL_TupleCreatorLoad (str);
        break;
    case BYTE_KEYWORD_IMPORT:
        module_ImportedLoad(str);
        break;
    case BYTE_KEYWORD_CARD:
        IL_CardLoad(str);
        break;
    case BYTE_KEYWORD_TYPE_OF:
        IL_TypeOfLoad(str);
        break;
    default:
        printf("this kind of byte code not support yet");
        break;
    }
}

/*将所有的编译相关数据清空，以方便连续的编译*/
static void Dump()
{
    struct_Dump ();
    API_Dump();
    ConstSegmentDump ();
    func_Dump ();
    module_Dump();
    OffsetReset();
}

/*编译成字节码文件*/
static void WriteByteCode(const char *file_name)
{
    /*添加.tzw的后缀名*/
    char file_tzw[128];
    strcpy (file_tzw,file_name);
    strcat(file_tzw,".tzw");
    FILE * f=fopen (file_tzw,"wb");

    /*导入写入*/
    module_ImportedCompile( f);
    //常量写入
    ConstSegmentWrite(f);
    //结构体写入
    struct_WriteByteCode(f);

    //函数写入
    func_WriteByteCode(f);

    fclose(f);
}

static int struct_offset=0;
static int unresolved_offset=0;
static int function_offset=0;
static int const_offset=0;
/*载入一个字节码文件*/
void Tina_Load(const char *file_name)
{
    /*添加.tzw的后缀名*/
    char file_tzw[128];
    strcpy (file_tzw,file_name);
    strcat(file_tzw,".tzw");
    FILE * f=fopen (file_tzw,"rb");
    if(f==NULL)
    {
        STOP("INVALID BYTE CODE FILE:%s\n",file_tzw);
    }
    //逐行读入字节码
    for(; ;){

        char str_buff[128];
        memset (str_buff,0,sizeof str_buff);
        fgets(str_buff,128,f);
        if(feof(f)!=0) break;
        ParseByteCodeLine(str_buff);

    }
    struct_offset= struct_GetCount();
    unresolved_offset=module_GetUnresolvedCount();
    function_offset= Tina_FuncGetCount();
    const_offset=ConstSegmentGetCount ();
}
int build_GetStructOffset()
{
    return struct_offset;
}

int build_GetUnresolvedOffset()
{
    return unresolved_offset;
}
int build_GetFunctionOffset()
{
    return function_offset;
}

int build_GetConstOffset()
{
    return const_offset;
}
static void OffsetReset()
{
    struct_offset=0;
    unresolved_offset=0;
    function_offset=0;
    const_offset=0;
}

void Tina_Compile(const char *file_name)
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
            token_Get(&test_pos,&t_k);
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
            token_Get(&test_pos,&t_k);
            switch(t_k.type)
            {
            case TOKEN_TYPE_EOF:
                break;
                /*检查导入*/
            case TOKEN_TYPE_IMPORT:
                postion =test_pos;
                module_ImportParse(&postion);
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
                func_ParseDef(&postion);
                break;
				/*发现类的定义*/
			case TOKEN_TYPE_STRUCT:
				struct_ParseDefine(&test_pos);/*解析类*/
				postion =test_pos;
				break;
			default :
            {
                CompileError_ShowError (postion,"unknown global symbol");
				exit(0);
            }
				break;
			}
		}
		while(t_k.type!=TOKEN_TYPE_EOF);
	}
    /*编译成字节码*/
    WriteByteCode(file_name);
    Dump();
}


/*载入字节码链表，并执行main 函数*/
void Tina_ExcuteByteCodeList(const char * file)
{
    char file_name[128];
    strcpy(file_name,file);
    strcat (file_name,".tbl");
    FILE * f =fopen(file_name,"r");
    if(!f)
    {
        STOP("there is no such %s Tina bytecode list file\n",file_name);
    }
    /*解析作者*/
    {
        char author_name[32];
        char attribute[32];
        fscanf (f,"%s = %s",attribute,author_name);
        if(strcmp (attribute,"AUTHOR")!=0)
        {
            STOP("invalid tbl file\n");
        }
        else
        {
            printf("the author is %s\n",author_name);
        }
    }
    /*解析版本号*/
    {
        char ver_str[32];
        char attribute[32];
        fscanf (f,"%s = %s",attribute,ver_str);
        if(strcmp (attribute,"VERSION")!=0)
        {
            STOP("invalid tbl file\n");
        }
        else
        {
            int version=atoi(ver_str);
            if(version !=Tina_GetVersion ())
            {
                STOP("the version is not compatible \n");
            }

        }
    }
    for(;;)
    {
        char attribute[32];
        char byte_code_name[32];
        fscanf (f,"%s = %s\n",attribute,byte_code_name);
        if(strcmp (attribute,"BYTE_CODE")!=0)
        {
             STOP("invalid tbl file\n");
        }
        Tina_Load (byte_code_name);
        if(feof(f)) break;
    }
    Tina_Run ("main");
}



/*根据字节码列表文件，逐个编译*/
void Tina_Buid(const char * file)
{
    char file_name[128];
    strcpy(file_name,file);
    strcat (file_name,".tbl");
    FILE * f =fopen(file_name,"r");
    if(!f)
    {
        STOP("there is no such %s Tina bytecode list file\n",file_name);
    }
    /*解析作者*/
    {
        char author_name[32];
        char attribute[32];
        fscanf (f,"%s = %s",attribute,author_name);
        if(strcmp (attribute,"AUTHOR")!=0)
        {
            STOP("invalid tbl file\n");
        }
        else
        {
            printf("the author is %s\n",author_name);
        }
    }
    /*解析版本号*/
    {
        char ver_str[32];
        char attribute[32];
        fscanf (f,"%s = %s",attribute,ver_str);
        if(strcmp (attribute,"VERSION")!=0)
        {
            STOP("invalid tbl file\n");
        }
        else
        {
            int version=atoi(ver_str);
            if(version !=Tina_GetVersion ())
            {
                STOP("the version is not compatible \n");
            }

        }
    }
    for(;;)
    {
        char attribute[32];
        char byte_code_name[32];
        fscanf (f,"%s = %s\n",attribute,byte_code_name);
        if(strcmp (attribute,"BYTE_CODE")!=0)
        {
             STOP("invalid tbl file\n");
        }
        Tina_Compile (byte_code_name);
        if(feof(f)) break;
    }
}
