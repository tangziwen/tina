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
/*
此文件用以实现tina的模块机制

*/
#include <string.h>
#include "module.h"
#include "token.h"
#include "script_struct.h"
#include "function.h"
/*模块名,默认情况下表示为全局*/
static  char module_context_name[100]= {0};

static int module_index=0;

static int import_index=0;
/*模块列表*/
static char module_list[10][100]= {0};

static char import_list[10][100]={0};

static void set_context(char * str);

/*获得当前一共创建了的模块数量*/
int module_GetMoudleCount()
{
return module_index;
}

void module_parse_using(int * pos)
{
	TokenInfo t_k;
	do
	{
		token_Get(pos,&t_k);
		switch(t_k.type)
		{
			/*拷入要导入的包*/
		case TOKEN_TYPE_SYMBOL:
            module_index++;
			strcpy(module_list[module_index],t_k.content);
			break;
		case TOKEN_TYPE_COMMA:
			break;
		case TOKEN_TYPE_SEMICOLON:
			break;
		case TOKEN_TYPE_EOF:
			break;
		default:
			printf("illigal import !\n");
			exit(0);
			break;
		}
	}
	while(t_k.type!=TOKEN_TYPE_EOF && t_k.type!=TOKEN_TYPE_SEMICOLON);
}



/*扫描模块*/
void module_parse_declare(int * pos)
{
	TokenInfo t_k;
	token_Get(pos,&t_k);
	/*更新模块名*/
	set_context(t_k.content);
	/*跳过左边的花括号*/
	token_Get(pos,&t_k);
	int brace=-1;
	{
		/*扫描模块内部的声明*/
		do
		{
			token_Get(pos,&t_k);
			switch(t_k.type)
			{
			case TOKEN_TYPE_EOF:
				break;
			case TOKEN_TYPE_LEFT_BRACE:
				brace--;
				break;
			case TOKEN_TYPE_RIGHT_BRACE:
				brace++;
				if(brace==0)
				{
					t_k.type=TOKEN_TYPE_EOF;
				}
				break;
				/*发现函数的定义符号,声明函数*/
			case TOKEN_TYPE_FUNC_DEF:
				func_ParseDeclare(pos);
				break;
				/*发现类的定义，声明类*/
			case TOKEN_TYPE_STRUCT:
				struct_ParseDeclare(pos);
				token_SkipBlock(pos);/*略过类定义块*/
				break;
			default :
				break;
			}
		}
		while(t_k.type!=TOKEN_TYPE_EOF);
	}
	/*将当前模块重置为全局*/
	set_context("\0");
}
/*扫描模块的定义*/
void module_parse_def(int * pos)
{
	TokenInfo t_k;
	token_Get(pos,&t_k);
	/*更新模块名*/
	set_context(t_k.content);
	token_Get(pos,&t_k);
	int brace=-1;
	/*第二遍扫描,扫描所有函数以及类的定义*/
	{
		do
		{
			token_Get(pos,&t_k);
			switch(t_k.type)
			{
			case TOKEN_TYPE_EOF:
				break;
				/*发现函数的定义符号,定义函数*/
			case TOKEN_TYPE_FUNC_DEF:
				func_ParseDef(pos);
				break;
			case TOKEN_TYPE_USING:
				module_parse_using(pos);
				break;
				/*发现类的定义*/
			case TOKEN_TYPE_STRUCT:
				struct_ParseDefine(pos);/*解析类*/
				break;
			case TOKEN_TYPE_LEFT_BRACE:
				brace--;
				break;
			case TOKEN_TYPE_RIGHT_BRACE:
				brace++;
				if(brace==0)
				{
					t_k.type=TOKEN_TYPE_EOF;
				}
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
	/*将当前模块重置为全局*/
	set_context("\0");
}

/*查询当前符号被模块修饰后的名字*/
char * module_MangledName(int module_id,const char * the_symbol)
{
	static char  result[100];
	memset(result,0,sizeof(result));
	strcpy(result,module_list[module_id]);
	strcat(result,"#");
	strcat(result,the_symbol);
	return result;
}


/*查询当前符号在当前上下文模块的修饰后的名字*/
char * module_ContextMangledName(const char * the_symbol)
{
	static char  result[100];
	memset(result,0,sizeof(result));
	if(strcmp(module_context_name,"\0")!=0)
	{
		strcpy(result,module_context_name);
		strcat(result,"#");
		strcat(result,the_symbol);
	}
	else
	{
		strcpy(result,the_symbol);
	}
	return result;
}
static void set_context(char * str)
{
	strcpy(module_context_name,str);
}


static int plain_ImportedSearch(char * str)
{
    int i=1;
    for(;i<=import_index;i++)
    {
        if(strcmp(str,import_list[i])==0)
            return -i;
    }
    return 0;
}

/*检查模块*/
int module_ImportedSearch(char *name)
{
    int result=0;
    /*搜查当前模块*/
    result=plain_ImportedSearch( module_ContextMangledName(name));
    if(result==0)
    {
        int i=0;
        /*检查被using的模块*/
        for( ; i<module_index; i++)
        {
            result=plain_ImportedSearch(module_MangledName(i,name));
            if(result!=0)
            {
                break;
            }
        }
    }
    /*无限定全名*/
    if(result==0)
    {
        result=plain_ImportedSearch(name);
    }
    return result;
}

/*导入模块*/
void module_ImportParse(int * pos)
{
    TokenInfo t_k;
    do
    {
        token_Get (pos,&t_k);
        switch(t_k.type)
        {
            /*拷入要导入的包*/
        case TOKEN_TYPE_SYMBOL:
            import_index++;
            strcpy(import_list[import_index],t_k.content);
            break;
        case TOKEN_TYPE_COMMA:
            break;
        case TOKEN_TYPE_SEMICOLON:
            break;
        case TOKEN_TYPE_EOF:
            break;
        default:
            printf("illigal import !\n");
            exit(0);
            break;
        }
    }
    while(t_k.type!=TOKEN_TYPE_EOF && t_k.type!=TOKEN_TYPE_SEMICOLON);
}

/*将导入信息写入字节码中去*/
void module_ImportedCompile(FILE *f)
{
    int i=1;
    for( ;i<=import_index;i++)
    {
        fprintf(f,"IMPORT %s\n",import_list[i]);
    }
}
#define UNRESOLVED_LIST_MAX 64
static int unresolved_index=0;
static char unresolved_list[UNRESOLVED_LIST_MAX][32];
struct unresolved_atom_element
{
    char name[32];
    int * address[32];
    int account;
};

static int atom_list_count=0;
struct unresolved_atom{
    struct unresolved_atom_element element[UNRESOLVED_LIST_MAX];
}atom_list;

/*从字节码中读入导入信息*/
void module_ImportedLoad(char *str)
{
    char import_str[2][32];
    sscanf (str,"%s %s",import_str[0],import_str[1]);
    unresolved_index++;
    strcpy (unresolved_list[unresolved_index],import_str[1]);/*拷贝入未解析的符号表中*/
    return;
}

/*通过索引获取为解析的符号*/
char * module_GetUnresolvedSymbol(int index)
{
    return unresolved_list[index];
}

/*将字符串压入到未解析的原子表中*/
void module_PutUnresolvedAtom(char * symbol,int * index_address)
{
    int i=1;
    for(;i<=atom_list_count;i++)
    {
        if(strcmp (symbol,atom_list.element[i].name)==0)/*当前原子已存在？，把地址挂到该原子上*/
        {
            atom_list.element[i].address[atom_list.element[i].account]=index_address;
            atom_list.element[i].account++;
            atom_list.element[i].address[atom_list.element[i].account]=NULL;
            return;
        }
    }
    /*不存在，则是新的字符串，需要扩展原子表*/
    atom_list_count++;
    strcpy(atom_list.element[atom_list_count].name,symbol);
    atom_list.element[atom_list_count].account=0;
    atom_list.element[atom_list_count].address[0]=index_address;
    atom_list.element[atom_list_count].account++;
    atom_list.element[atom_list_count].address[1]=NULL;
}


/*判断一个字符串是否在原子表中，若是存在的话，则将他们替换成索引的值，并将其从表中删除*/
void module_CheckUnresolvedAtomListt(char *symbol,int index)
{
    int i=1;
    for(;i<=atom_list_count;i++)
    {

        if(strcmp (symbol,atom_list.element[i].name)==0)/*当前原子已存在*/
        {
            int j=0;
            for( ;j<atom_list.element[atom_list_count].account;j++)
            {
                if(atom_list.element[atom_list_count].address[j]!=NULL)
                {
                    *(atom_list.element[atom_list_count].address[j])=index;
                }
                else/*遇到NULL，则说明替换已经完成，把该原子从中删除*/
                {
                    int k=j;
                    for(;k+1<=atom_list_count;k++)
                    {
                        atom_list.element[k]=atom_list.element[k+1];
                    }
                    atom_list_count--;
                    return;
                }
            }
            return;
        }
    }
}

/*返回当前共有的未解析的标识符*/
int module_GetUnresolvedCount()
{
return unresolved_index;
}
