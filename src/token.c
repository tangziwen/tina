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
#include <ctype.h>
#include <string.h>
#include "tina.h"
#include "loader.h"
#include "token.h"
#include "api.h"
#include "assert.h"
#include "function.h"
#include "module.h"
#include "script_vec.h"


#define KEYWORD_MAX 24
#define KEYWORD_IF 0
#define KEYWORD_WHILE 1
#define KEYWORD_VAR 2
#define KEYWORD_ELSE 3
#define KEYWORD_PRINT 4
#define KEYWORD_TRUE 5
#define KEYWORD_FALSE 6
#define KEYWORD_FUNC_DEF 7
#define KEYWORD_RETURN 8
#define KEYWORD_AND 9
#define KEYWORD_OR 10
#define KEYWORD_FOR 11
#define KEYWORD_BREAK 12
#define KEYWORD_CONTINUE 13
#define KEYWORD_STRUCT 14
#define KEYWORD_SELF 15
#define KEYWORD_NIL 16
#define KEYWORD_PRIVATE 17
#define KEYWORD_SEALED 18
#define KEYWORD_MODULE 19
#define KEYWORD_USING 20
#define KEYWORD_DELETE 21
#define KEYWORD_VECTOR 22
#define KEYWORD_TUPLE 23
static char pre_is_dot=0;
static char * key_word_list[KEYWORD_MAX]=
{
	"if","while","var","else","print","true",
    "false","function","return","and","or","for","break","continue","struct","self","nil"
    ,"private","sealed","module","using","delete","vector","tuple"
};

static int lookup_keyword ( char * _str )
{
	int i=0;
	for ( i=0; i<KEYWORD_MAX; i++ )
	{
		if ( strcmp ( key_word_list[i],_str ) ==0 )
		{
			return i;
		}
	}
	return -1;
}
/*判断一个字符是否为操作符*/
static int is_op ( char a )
{
	if (a=='.' || a=='+'|| a== '-'|| a== '*' || a== '/' || a== '(' ||a== '=' || a=='>' ||a=='<' || a=='[')
	{

		return 1;
	}

	else
		return 0;
}
/*判断一个字符是否为操作符*/
static int is_op_withoutdot ( char a )
{
	if (a=='+'|| a== '-'|| a== '*' || a== '/' || a== '(' ||a== '=' || a=='>' ||a=='<' || a=='[')
	{

		return 1;
	}

	else
		return 0;
}

/*判断一个序列是否为复合运算符*/
static int is_compo_op ( char *a,TokenInfo * t_k )
{
	if ( a[0]=='>'&& a[1]=='=' )
	{
		t_k->type=TOKEN_TYPE_OP;
		t_k->content[0]=OP_LARGE_OR_EQUAL;
		t_k->content[1]='\0';
		return 1;
	}
	if ( a[0]=='<'&& a[1]=='=' )
	{
		t_k->type=TOKEN_TYPE_OP;
		t_k->content[0]=OP_LESS_OR_EQUAL;
		t_k->content[1]='\0';
		return 1;
	}
	if ( a[0]=='='&& a[1]=='=' )
	{
		t_k->type=TOKEN_TYPE_OP;
		t_k->content[0]=OP_EQUAL;
		t_k->content[1]='\0';
		return 1;
	}

	if ( a[0]=='+'&& a[1]=='=' )
	{
		t_k->type=TOKEN_TYPE_OP;
		t_k->content[0]=OP_ASSIGN_PLUS;
		t_k->content[1]='\0';

		return 1;
	}
	if ( a[0]=='-'&& a[1]=='=' )
	{
		t_k->type=TOKEN_TYPE_OP;
		t_k->content[0]=OP_ASSIGN_MINUS;
		t_k->content[1]='\0';
		return 1;
	}
	if ( a[0]=='*'&&a[1]=='=' )
	{
		t_k->type=TOKEN_TYPE_OP;
		t_k->content[0]=OP_ASSIGN_MULTIPLE;
		t_k->content[1]='\0';
		return 1;
	}
	if ( a[0]=='/'&&a[1]=='=' )
	{
		t_k->type=TOKEN_TYPE_OP;
		t_k->content[0]= OP_ASSIGN_DIVIDE;
		t_k->content[1]='\0';
		return 1;
	}
	if ( a[0]=='!'&&a[1]=='=' )
	{
		t_k->type=TOKEN_TYPE_OP;
		t_k->content[0]= OP_NOT_EQUAL;
		t_k->content[1]='\0';
		return 1;
	}
	return 0;
}

/*从指定位置开始向后跳过空白符，*/
/*直到遇到其他的字符，并返回第一个遇见的字符的位置*/
static int skip_space(int pos)
{
	for( ; isspace(buffer[pos]); pos ++);
	return pos;
}
static int escape_char(int c)
{
    switch(c)
    {
    case 'a':/* \a */
        return  '\a' ;
        break;
    case 'b':/* \b */
        return '\b';
        break;
    case 'f' : /* \f */
        return '\f';
        break;
    case 'n':/* \n */
        return '\n';
        break;
    case 'r': /* \r */
        return '\r';
        break;
    case 't': /* \t */
        return '\t';
        break;
    case 'v':
        return '\v';
        break;
    case '\\':
    return '\\';
    break;
    case '?':
        return '\?';
        break;
    case '\'':
        return '\'';
        break;
    case '\"':
        return '\"';
        break;
    case '0':
        return '\0';
        break;
    default:
        STOP("invalid escape char!!!");
        break;
    }
}

/*从当前位置(pos)解析一个词法标记，并将词法标记的相关信息存储进t_k所指向的TokenInfo对象里
解析之后，当前位置会向后跳跃一个词法标记
我们假设整个待输入的缓存中的词法是无错的(即不存在无效的词法单元)*/
void token_Get ( int * pos,TokenInfo * t_k )
{
	static int pre_tk_type=TOKEN_TYPE_INVALID;
	if ( t_k==NULL )
	{
		return;
	}
	/*刷新t_k的字符串的内容*/
	{
		int i=0;
		for ( i=0; i<TOKEN_NAME_SIZE-1; i++ )
		{
			t_k->content[i]=1;
		}
		t_k->content[i+1]='\0';
	}
	/*临时储存词法标记的字符串*/
	char tmp_token[TOKEN_NAME_SIZE];
	/*临时字符串清零*/
	{
		int i;
		for ( i =0; i<TOKEN_NAME_SIZE; i++ )
		{
			tmp_token[i]='\0';
		}
	}
	int postion= ( *pos );
	/*检测是否已经到了文件尾*/
	if ( postion>=buffer_size )
	{
		t_k->type =TOKEN_TYPE_EOF;
		pre_tk_type=t_k->type;
		return;
	}
	/*删除前导空白符（回车，换行，空格..）*/
	while ( isspace ( buffer[postion] ) )
	{
		postion++;
		/*这时亦有可能到达文件尾*/
		if ( postion>=buffer_size )
		{
			t_k->type=TOKEN_TYPE_EOF;
			pre_tk_type=t_k->type;
			return ;
		}
	}
	/*检查复合运算符*/
	if ( is_compo_op ( buffer+postion,t_k ) )
	{
		postion+=2;
		( *pos ) =postion;
		return;
	}
	/*检查引用标记*/
	if ( buffer[postion]=='&' )
	{
		postion++;
		( *pos ) =postion;
		t_k->type=TOKEN_TYPE_REF;
		t_k->content[0]='&';
		t_k->content[1]='\0';
		pre_tk_type=t_k->type;
		return;
	}
	if(buffer[postion]==']')
	{
		postion++;
		( *pos ) =postion;
		t_k->type=TOKEN_TYPE_RIGHT_BRACKET;
		t_k->content[0]=']';
		t_k->content[1]='\0';
		pre_tk_type=t_k->type;
		return;

	}
	/*检查运算符*/
	if ( is_op ( buffer[postion] ) )
	{
		t_k->type=TOKEN_TYPE_OP;
		t_k->content[0]=buffer[postion];
		t_k->content[1]='\0';

		/*考虑出现负常量，如-123.123*/
        if( pre_tk_type!=TOKEN_TYPE_NUM&&pre_tk_type!= TOKEN_TYPE_FUNC&& pre_tk_type!=TOKEN_TYPE_API&& pre_tk_type!=TOKEN_TYPE_MESSAGE&&buffer[postion]=='-')
		{
			int tes_pos=postion+1;
			if(isdigit (buffer [skip_space(tes_pos)]))
			{
				postion=skip_space(tes_pos);
				t_k->content[0]='-';
				t_k->content[1]='\0';
				int i=0;
				char tmp_token[32];
				for(; i<32; i++) tmp_token[i]='\0';
				i=0;
				while ( !isspace ( buffer[postion] ) &&
						!is_op_withoutdot ( buffer[postion] )
						&&buffer[postion]!=';'
						&&buffer[postion]!='{'
						&&buffer[postion]!='}'
						&&buffer[postion]!=','
						&&buffer[postion]!=')'
						&&buffer[postion]!=']'
						&&buffer[postion]!='&'
						&&!is_compo_op(buffer+postion,t_k)
				      )
				{
					tmp_token[i]=buffer[postion];
					i++;
					postion++;
				}
				/*把数字拷入*/
				strcat(t_k->content,tmp_token);
				t_k->type=TOKEN_TYPE_NUM;
				pre_tk_type=t_k->type;
				( *pos ) =postion;
				return;
			}
		}
		if(buffer[postion]=='.')
		{
			extern int is_exp_parsing;
			extern int is_parsing_func_def;
			if(is_exp_parsing==1 && is_parsing_func_def==1)
			{
				pre_is_dot=1;
			}
		}
		postion++;
		( *pos ) =postion;
		return ;
	}
	/*检查括号*/
	/**/
	if ( buffer[postion]==')' )
	{
		t_k->type=TOKEN_TYPE_RIGHT_PARENTHESIS;
		t_k->content[0]=')';
		t_k->content[1]='\0';
		postion++;
		( *pos ) =postion;
		pre_tk_type=t_k->type;
		return;
	}

	if ( buffer[postion]=='{' )
	{
		t_k->type=TOKEN_TYPE_LEFT_BRACE;
		t_k->content[0]='{';
		t_k->content[1]='\0';
		postion++;
		( *pos ) =postion;
		pre_tk_type=t_k->type;
		return;
	}
	if ( buffer[postion]=='}' )
	{
		t_k->type=TOKEN_TYPE_RIGHT_BRACE;
		t_k->content[0]='}';
		t_k->content[1]='\0';
		postion++;
		( *pos ) =postion;
		pre_tk_type=t_k->type;
		return;
	}
	/*检查分号*/
	if ( buffer[postion]==';' )
	{
		t_k->type=TOKEN_TYPE_SEMICOLON;
		t_k->content[0]=';';
		t_k->content[1]='\0';
		postion++;
		( *pos ) =postion;
		pre_tk_type=t_k->type;
		return;
	}
	/*检查逗号*/
	if ( buffer[postion]==',' )
	{
		t_k->type=TOKEN_TYPE_COMMA;
		t_k->content[0]=',';
		t_k->content[1]='\0';
		postion++;
		( *pos ) =postion;
		pre_tk_type=t_k->type;
		return;
	}
	if(isdigit(buffer[postion]))
	{
		int i=0;
		while ( !isspace ( buffer[postion] ) &&
				!is_op_withoutdot(buffer[postion] )
				&&buffer[postion]!=';'
				&&buffer[postion]!='{'
				&&buffer[postion]!='}'
				&&buffer[postion]!=','
				&&buffer[postion]!=')'
				&&buffer[postion]!=']'
				&&buffer[postion]!='&'
				&&!is_compo_op(buffer+postion,t_k)
		      )
		{
			tmp_token[i]=buffer[postion];
			i++;
			postion++;
		}
		tmp_token[i]='\0';
		{
			t_k->type= TOKEN_TYPE_NUM;
			strcpy ( t_k->content,tmp_token );
			( *pos ) =postion;
			pre_tk_type=t_k->type;
			return;
		}

	}
	/*如果遇到了字母*/
	if ( isalpha ( buffer[postion] ) )
	{
		int i=0;
		while ( !isspace ( buffer[postion] ) &&
				!is_op ( buffer[postion] )
				&&buffer[postion]!=';'
				&&buffer[postion]!='{'
				&&buffer[postion]!='}'
				&&buffer[postion]!=','
				&&buffer[postion]!=')'
				&&buffer[postion]!=']'
				&&buffer[postion]!='&'
				&&!is_compo_op(buffer+postion,t_k)
		      )
		{
			tmp_token[i]=buffer[postion];
			i++;
			postion++;
		}

		tmp_token[i]='\0';
		int tmp =lookup_keyword ( tmp_token );
		strcpy ( t_k->content,tmp_token );
		/*检查是否为一个关键字*/
        switch (tmp)
		{
		case -1:/*不是一个关键字*/
			/*判断是否为一个引用成员运算*/
		{
			extern int is_exp_parsing;
			extern int is_parsing_func_def;
            /*如果之前遇到 "."运算符，则此处为一个消息.*/
			if(pre_is_dot==1 &&is_exp_parsing==1 &&is_parsing_func_def==1)
			{
				t_k->str=malloc(strlen(tmp_token)+1);
				strcpy(t_k->str,tmp_token);
                t_k->type=TOKEN_TYPE_MESSAGE;
				pre_is_dot=0;
				break;
			}
		}

		/*检查是否为一个API函数*/
		if ( API_Search ( tmp_token ) >=0)
		{
			t_k->type=TOKEN_TYPE_API;
			break;
		}
		else	if ( func_get_index_by_name ( tmp_token ) >=0 )/*检查是否为一个脚本函数*/
		{
			t_k->type=TOKEN_TYPE_FUNC;
			break;
		}
        else if( get_class_id(tmp_token)!=0) /*检查是否为一个结构体名称*/
		{
			t_k->type=TOKEN_TYPE_STRUCT_NAME;
			break;
		}
		else
		{
			/*若都不是,则为一个自定义标识符*/
			t_k->type=TOKEN_TYPE_SYMBOL;
			break;
		}
		break;
		case KEYWORD_IF:
			t_k->type=TOKEN_TYPE_IF;
			break;
		case KEYWORD_WHILE:
			t_k->type=TOKEN_TYPE_WHILE;
			break;
		case KEYWORD_ELSE:
			t_k->type=TOKEN_TYPE_ELSE;
			break;
		case KEYWORD_PRINT:
			t_k->type=TOKEN_TYPE_PRINT;
			break;
		case KEYWORD_VAR:
			t_k->type=TOKEN_TYPE_VAR_DEF;
			break;
		case KEYWORD_TRUE:
			t_k->type=TOKEN_TYPE_TRUE;
			break;
		case KEYWORD_FALSE:
			t_k->type=TOKEN_TYPE_FALSE;
			break;
		case KEYWORD_FUNC_DEF:
			t_k->type=TOKEN_TYPE_FUNC_DEF;
			break;
		case KEYWORD_RETURN:
			t_k->type=TOKEN_TYPE_RETURN;
			break;
		case KEYWORD_AND:
			t_k->type=TOKEN_TYPE_OP;
			t_k->content[0]=OP_AND;
			t_k->content[1]='\0';
			break;
		case KEYWORD_OR:
			t_k->type=TOKEN_TYPE_OP;
			t_k->content[0]=OP_OR;
			t_k->content[1]='\0';
			break;
		case KEYWORD_FOR:
			t_k->type=TOKEN_TYPE_FOR;
			break;
		case KEYWORD_BREAK:
			t_k->type=TOKEN_TYPE_BREAK;
			break;
		case KEYWORD_CONTINUE:
			t_k->type=TOKEN_TYPE_CONTINUE;
			break;
		case KEYWORD_STRUCT:
			t_k->type=TOKEN_TYPE_STRUCT;
			break;
		case KEYWORD_SELF:
			t_k->type=TOKEN_TYPE_SELF;
			break;
		case KEYWORD_NIL:
			t_k->type=TOKEN_TYPE_NIL;
			break;
		case KEYWORD_PRIVATE:
			t_k->type=TOKEN_TYPE_PRIVATE;
			break;
		case KEYWORD_MODULE:
			t_k->type=TOKEN_TYPE_MODULE;
			break;
		case KEYWORD_USING:
			t_k->type=TOKEN_TYPE_USING;
			break;
		case KEYWORD_DELETE:
			t_k->type=TOKEN_TYPE_DELETE;
			break;
        case KEYWORD_VECTOR:
            t_k->type=TOKEN_TYPE_VECTOR;
            break;
        case KEYWORD_TUPLE:
            t_k->type=TOKEN_TYPE_TUPLE;
            break;
		default :
			break;
		}

		pre_tk_type=t_k->type;
		( *pos ) =postion;
		return ;

	}
	else
	{
        /*字符串字面量?*/
		if ( buffer[postion]=='"' )
		{
			char * tmp_str;
			postion++;
			int i=postion;
			do
			{
				i++;
			}
			while ( buffer[i]!='"' );
			/*拷贝字符串*/
			tmp_str= ( char* ) malloc ( sizeof ( char ) * ( i-postion+1 ) );
			memcpy ( tmp_str,buffer+postion,sizeof ( char ) * ( i-postion+1 ) );
			tmp_str[i-postion]='\0';

            t_k->type=TOKEN_TYPE_STRING;
            t_k->str=tmp_str;
			postion=i;
			postion++;
			( *pos ) =postion;
			return ;
		}
        else
            if(buffer[postion]=='\'')/*字符变量？*/
            {
                postion++;
                t_k->type=TOKEN_TYPE_CHAR;
                t_k->content[0]=buffer[postion];
                postion++;
                if(buffer[postion]!='\''  && buffer[postion-1]!= 92)
                {
                    STOP("invalid char");
                }
                else if( buffer[postion-1]== '\\')/*遇见转义字符*/
                {
                    t_k->content[0]=escape_char (buffer[postion]);
                    postion++;
                    if(buffer[postion]!='\'')
                    {
                        STOP("invalid char");
                    }
                }
                postion++;
                (*pos)=postion;
            }
	}
}


void token_SkipBlock(int * pos)
{
    TokenInfo t_k;
    do
    {
        token_Get ( pos,&t_k );
    }
    while(t_k.type!=TOKEN_TYPE_LEFT_BRACE  && t_k.type!=TOKEN_TYPE_EOF);

    int brace =-1;
    do
    {
        token_Get ( pos,&t_k );
        switch(t_k.type)
        {
        case TOKEN_TYPE_LEFT_BRACE:
            brace--;
            break;
        case TOKEN_TYPE_RIGHT_BRACE:
            brace++;
            break;
        }
        if(brace==0)
        {
            return;
        }
    }
    while(t_k.type!=TOKEN_TYPE_EOF);
}

/*越过一条语句*/
void token_skip_statement ( int *pos )
{
    while ( 1 )
    {
        TokenInfo t_k;
        token_Get ( pos,&t_k );
        switch ( t_k.type )
        {
        default:
            break;
        case TOKEN_TYPE_SEMICOLON:
            return ;
            break;
        }
    }
}

/*带断言的解析词法标记,如果不符合规定的词法类型,则中断,并抛出一个错误的信息*/
void token_get_assert(int * pos,TokenInfo * t_k,int type, char * info )
{
	token_Get(pos,t_k);
	/*不符合断言的类型,抛出错误*/
	if(t_k->type!=type)
	{
		printf("error !! %s %d %s\n",info,t_k->type,t_k->content);
		exit(0);
	}
}
