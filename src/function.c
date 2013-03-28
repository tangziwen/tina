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
#include <string.h>
#include "print.h"
#include "type.h"
#include "expression.h"
#include "function.h"
#include "var.h"
#include "vm.h"
#include "il.h"
#include "script_struct.h"
#include "script_tuple.h"
#include "module.h"
#include "debug.h"
#include "block.h"
/*函数列表*/
Function function_list[FUNCTION_MAX];
int current_func_index=0;
/*当前的被扫描函数*/
Function * current_func=NULL;

/*通过名称检索函数,如果存在,返回索引
如果不存在,则返回0*/
static int plain_get_index_by_name(const  char * func_name)
{
    int i;
    for(i=1; i<=current_func_index; i++)
    {
        if(strcmp(function_list[i].name,func_name)==0)
        {
            return i;
        }
    }
    return 0;
}


static void CreateFunctionFromByteCode(int args,char *name )
{
    current_func_index++;
    function_list[current_func_index].arg_counts=args;
    if(plain_get_index_by_name(name)!=0)
    {
        STOP("function name conflict %s\n",name);
    }
    strcpy (function_list[current_func_index].name,name);
    func_set_current( & (function_list[current_func_index]));
}




int func_get_index_by_name(const char * func_name)
{
	char name[128]= {0};
    int result=0, i;
	/*先检查本模块内部*/
	strcpy(name,module_ContextMangledName(func_name));
	result=plain_get_index_by_name(name);
    if(result>0)
	{
		return result;
	}

	/*再检查导入模块是否有该名字*/
    for(i=1; i<=module_GetMoudleCount (); i++)
	{
		result=plain_get_index_by_name(module_MangledName(i,func_name));
        if(result>0)
		{
			return result;
		}
	}
	/*最后进行全局检查,用户使用完全限定的标识符也在此类*/
    if(result<=0)
	{
		result=plain_get_index_by_name(func_name);
	}
	return result;
}

/*获得当前已经被解析过的函数个数*/
int Tina_FuncGetCount()
{
    return current_func_index;
}
/*解析函数的声明*/
void func_ParseDeclare(int *postion)
{
	TokenInfo t_k;
    token_Get(postion,&t_k);/*获取函数名称*/
	/*加上模块前缀*/
	char  unique_name[32]= {0};
	strcpy(unique_name,module_ContextMangledName(t_k.content));

    int result=plain_get_index_by_name(unique_name);
    if(result!=0)
    {
        STOP("error !! the function %s  has already been defined!!\n",unique_name);
    }
     current_func_index++;
	/*创建函数*/
    strcpy(function_list[current_func_index].name,unique_name);
}

/*解析方法（成员函数）的声明*/
void method_parse_declare(int *postion,char * class_name)
{
	TokenInfo t_k;
    char unique_name [32];
    token_Get(postion,&t_k);/*获取函数名称*/
    strcpy (unique_name,t_k.content);
	/*加上后缀*/
    strcat(unique_name,"#");
    strcat(unique_name,class_name);
    int result=plain_get_index_by_name(unique_name);
    if(result!=0)
    {
        STOP("error !! the method  has already been defined!!\n");
    }
    current_func_index++;
	/*创建函数*/
    strcpy(function_list[current_func_index].name,unique_name);
}

Function * func_get_by_index(int index)
{
	return &(function_list[index]);
}

/*
平凡性调用,用于具有聚合原子性的地方
*/
Var  func_PlainInvoke(int index)
{
 Var return_value=IL_exec(&function_list[index]);
CleanCurrentLocalVar();
return return_value;
}

Var func_invoke(int index)
{
	Var return_value=func_PlainInvoke(index);
	struct_CleanTmpPool();
	tuple_CleanTmpPool();
	return return_value;
}


/*解析字节码*/
static void parse_parameter(Function *f,int *postion)
{
    /*设置该函数为当前扫描函数*/
    func_set_current(f);
    TokenInfo t_k;
    /*解析参数形如  (a,b,c)*/
    /*我们把参数当作函数的最外层的最前面的*/
    /*几个变量.*/
    token_Get(postion,&t_k);
    if(t_k.type!=TOKEN_TYPE_OP&&t_k.content[0]!='(')
    {
        printf("error miss '(' in function args list \n");
        exit(0);
    }
    int type=VAR_TYPE_INT;
    do
    {
        token_Get(postion,&t_k);
        switch(t_k.type)
        {
        case TOKEN_TYPE_SYMBOL:
        {
            int i=0;
            for(i=0; i<f->var_counts; i++)
            {
                if(strcmp(t_k.content,f->var_list[i].name)==0)
                {
                    printf("error the args name conflict!!!\n");
                    exit(0);
                }
            }
            strcpy(f->var_list[f->var_counts].name
                   , t_k.content);
            f->var_list[f->var_counts].content.type=type;
            f->var_list[f->var_counts].layer=0;
            f->arg_counts++;
            f->var_counts++;
            type=VAR_TYPE_INT;
        }
        break;
        case TOKEN_TYPE_COMMA:
            break;
        case TOKEN_TYPE_RIGHT_PARENTHESIS:
            goto end;
            break;
        default:
            printf("error in function arg list \n");
            exit(0);
            break;
        }

    }
    while(t_k.type!=TOKEN_TYPE_RIGHT_PARENTHESIS);
end:
    ;
}

/*解析函数的定义*/
Function * func_ParseDef(int *postion )
{
	TokenInfo t_k;
	/*获得函数的名字*/
    token_Get(postion,&t_k);
	/*我们已经预扫描过了函数的声明.现在我们直接通过函数的名字获取该函数的结构*/
	Function *f =func_get_by_index(func_get_index_by_name(t_k.content));
    parse_parameter(f,postion);
    int scan_pos=(*postion);
    var_parse_local(&scan_pos);/*初始化变量模块*/
    /*分析代码块*/
    int result =block_Parse(postion,0,-1,-1,FUNC_GLOBAL);
    if(result!=0)
    {
        STOP("illegal function define %s\n",block_GetLastStateStr ());
    }
    //在函数的结尾处，始终增加一个return
    IL_ListInsertReturn();
	return f;
}


/*解析方法（成员函数）的定义*/
Function * method_parse_def(int *postion,char * class_name )
{
	TokenInfo t_k;
    token_Get(postion,&t_k);
	strcat(t_k.content,"#");
	strcat(t_k.content,class_name);

	/*我们已经预扫描过了函数的声明.现在我们直接通过函数的名字获取该函数的结构*/
	Function *f =func_get_by_index(func_get_index_by_name(t_k.content));
    parse_parameter(f,postion);
	int scan_pos=*postion;
    var_parse_local(&scan_pos);/*初始化变量模块*/
    int result =block_Parse(postion,0,-1,-1,FUNC_MEMBER);
    if(result!=0)
    {
        STOP("illegal method define %s\n",block_GetLastStateStr ());
    }
	return f;
}

/*运行一个函数*/
void Tina_Run(const char *name)
{
	int id=func_get_index_by_name(name);
    if(id>0)
	{
		func_invoke(id);
	}
	else
	{
		printf("there is no function called %s",name);
		exit(0);
	}
}


void func_set_current(Function * func)
{
	current_func=func;
}


Function * func_get_current()
{
	return current_func;
}

Var ScriptFuncArgList[32]= {0};

/*设置脚本函数的参数*/
void Tina_SetScriptFuncArg(Var arg,int id)
{
	ScriptFuncArgList[id]=arg;
}

/*调用脚本函数*/
Var Tina_CallScriptFunc(int func_id)
{
	Var result;
	vm_RTstackPush();
	Function * f =func_get_by_index ( func_id);
	/*为函数的实参赋值*/
	int i;
	for ( i=0;
			i<f->arg_counts;
			i++)
	{
		vm_rt_stack_var_set ( i,ScriptFuncArgList[i]);
	}
	/*保存当前的表环境*/
	IL_list * old_list =current_list;
	/*调用*/
	result =func_invoke ( func_id);
	/*环境恢复*/
	current_list=old_list;
	vm_RTstackPop();
	return result;
}

//函数编译成字节码
void func_WriteByteCode(FILE * f)
{
    int i=1;
    for(; i<=current_func_index;i++)
    {
        fprintf(f,"F %d %s\n",function_list[i].arg_counts,function_list[i].name);
        IL_ListCompile(f,i);
    }
}

/*从字节码中载入函数定义*/
void func_Load(char *str)
{
    char func_char[3][32];
    sscanf (str,"%s %s %s",func_char[0],func_char[1],func_char[2]);
    int args=atoi(func_char[1]);
    CreateFunctionFromByteCode(args,func_char[2]);
}

/*将函数的数据清空，以方便下次编译*/
void func_Dump()
{
    int i=1;
    for(;i<=current_func_index;i++)
    {
        IL_FreeAllNode(&(function_list[i].list));
        function_list[i].arg_counts=0;
    }
    current_func=NULL;
    current_func_index=0;
    current_list=NULL;
}
