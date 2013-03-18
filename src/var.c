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
#include "var.h"
#include "vm.h"
#include "token.h"
#include "expression.h"
#include "function.h"
#include "il.h"
#include "script_struct.h"
#include "script_vec.h"

static int layer_vars_count[128];/*栈层次*/
static int current_layer;/*当前的栈层次*/

/*获得当前被扫描函数的局部变量的个数*/
int get_vars_count()
{
	return func_get_current()->var_counts;
}




/*解析局部变量的定义,并确定其在函数中的相对索引*/
void var_define(int *pos, int *index,int offset,int layer)
{
	TokenInfo t_k;
	/*试图获得变量的名称,如果获得的不是变量名称,*/
	/*则为非法赋值*/
	token_get_assert(pos,&t_k,TOKEN_TYPE_SYMBOL,"invalid var define");

	/*检查在本层之内同名的变量是否已经被声明过.*/
	{
		int i;
		for(i=(*index)-1; i>=offset; i--)
		{
            if(strcmp(t_k.content,func_get_current()->var_list[i].name)==0)
			{
				/*被使用过了,抛出异常*/
                printf("the var name %s  has already used !!\n",t_k.content);
				exit(0);
			}
		}
	}
	if(func_get_current()->var_list[(*index)].layer==-1)   /*该位置还没用过,那么变量直接放在这就行了*/
	{
		func_get_current()->var_list[(*index)].layer=layer;
        strcpy(func_get_current()->var_list[(*index)].name,t_k.content);
		(*index)+=1;
		func_get_current()->var_counts+=1;
	}
	else
		/*如果该位置已经被占用了,那么这个位置的已有即以后的变量全部向后挪一位,*/
		/*腾出位置给新的变量.*/
	{
		int i;
		for(i=func_get_current()->var_counts; i>(*index); i--)
		{
			func_get_current()->var_list[i]=func_get_current()->var_list[i-1];
		}
		/*然后加入该变量*/
		func_get_current()->var_list[(*index)].layer=layer;
        func_get_current()->var_list[(*index)].content.type=VAR_TYPE_TUPLE;
        strcpy(func_get_current()->var_list[(*index)].name,t_k.content);
		(*index)+=1;
		func_get_current()->var_counts+=1;
        printf("new var %s %d define in layer %d\n",t_k.content,(*index),layer);
	}
}


/*预分析局部变量的声明*/
/*输入的参数当前栈的相对起始位置,以及当前的层数,和位置*/
static void local_declare_parse(int offset,int layer,int * pos)
{
	static int a=0;
	a++;
	int index =offset;/*索引,最开始在偏移量处*/
	TokenInfo t_k;
	token_get(pos,&t_k);/*先跳过本层的左括号*/
	do
	{
		/*因为需要更多的信息,所以我们有两个变量控制*/
		/*步进关系*/
		int test_pos=(*pos);
		token_get(&test_pos,&t_k);/*开始读取*/
		switch(t_k.type)
		{
			/*遇到变量的声明式*/
		case TOKEN_TYPE_VAR_DEF:
		{
			(*pos)=test_pos;
			var_define(pos,&index,offset,layer);
		}
		break;
		/*遇到左括号说明遇见了嵌套结构*/
		case TOKEN_TYPE_LEFT_BRACE:
			/*回退一格,递归调用本函数*/
			local_declare_parse(index,layer+1,pos);
			break;

			/*for语句较为特殊,因为其初始化表达式可能有定义式*/
		case TOKEN_TYPE_FOR:
			token_get(&test_pos,&t_k);/*跳过左小括号*/
			token_get(&test_pos,&t_k);/*判定for语句的初始化表达式是否有定义式*/
			(*pos)=test_pos;
			if(t_k.type==TOKEN_TYPE_VAR_DEF)
			{
				/*for语句初始化表达式所定义的变量属于内层变量*/
				/*所以它针对的起始偏移量要发生改变*/
				var_define(pos,&index,offset+index,layer+1);
			}
			break;
			/*遇到右括号则终结*/
		case TOKEN_TYPE_RIGHT_BRACE:
			(*pos)=test_pos;
			return;
			break;
			/*我们只检测声明式,所以遇见了其他的几号就跳过改句*/
		default:
			(*pos)=test_pos;
			break;

		}
	}
	while(t_k.type!=TOKEN_TYPE_EOF);
}
/*对函数体内的局部变量进行扫描*/
/*它最终会为每一个函数（除参数外）的局部变量*/
/*确定其相对索引，并储存在Function的var_list字段中。*/
void var_parse_local(int *pos,int arg_count)
{
	int i;
	for(i=func_get_current()->arg_counts; i<GLOBAL_VAR_MAX; i++)
	{
		func_get_current()->var_list[i].layer=-1;
	}
	local_declare_parse(arg_count,0,pos);
}



/*通过名字查找当前被扫描函数的局部变量的索引*/
int var_get_local_index(const char *var_name,int layer)
{
	/*让索引指向当前层的最后一个变量*/
	int index=0;
	for(index=0; func_get_current()->var_list[index].layer!=(layer+1)&&func_get_current()->var_list[index].layer!=-1; index++)
	{

	}
	index --;
	/*然后根据索引值从后往前找,这样的话.会先找到最近层次的变量,从而屏蔽外部层次的变量名*/
	int i=0;
	for(i=index; i>=0; i--)
	{
		if(strcmp(var_name,func_get_current()->var_list[i].name)==0)
		{
			return i;
		}
	}
	printf("%s is an undefined symbol\n",var_name);
	exit(0);
	return -3;
}



double get_value(int index)
{
    if(func_get_current()->var_list[index].content.type==VAR_TYPE_INT)
	{

        return (double)func_get_current()->var_list[index].content.var_value.int_value;
	}
	else
	{

        return (double)func_get_current()->var_list[index].content.var_value.real_value;
	}

}
Var var_point_to(Var a,Var b)
{
	Var result;

	/*对象的成员访问*/
    if(a.content.type==VAR_TYPE_HANDLE && b.content.type==VAR_TYPE_MESSAGE)
	{
		/*找不到成员*/
        if(get_index_of_member(a.class_id,b.content.var_value.str)==-1)
		{
            printf("there is no such member which's called \"%s\" in type \"%s\" \n",b.content.var_value.str,struct_get_name(a.class_id));
			exit(0);
		}
        result = *(GetObjectMemberAddress(a.content.var_value.handle_value,get_index_of_member(a.class_id,b.content.var_value.str)));
	}

	/*类的静态成员访问*/
    else if(a.content.type==VAR_TYPE_STRUCT_NAME && b.content.type==VAR_TYPE_MESSAGE)
	{
        int struct_id=get_class_id(a.content.var_value.str);
		/*找不到成员*/
        if(get_index_of_member(struct_id,b.content.var_value.str)==-1)
		{
            printf("there is no such member which's called \"%s\" in type \"%s\" \n",b.content.var_value.str,struct_get_name(struct_id));
			exit(0);
		}
        int member_id=get_index_of_member(struct_id,b.content.var_value.str);
		result=GetMember(struct_id,member_id);
	}
	else
	{
        printf("invalid dot operation %d    %d \n",a.content.type,b.content.type);
		exit(0);
	}
	return result;
}

/*计算两数相加*/
Var var_add(Var a,Var b)
{
	Var result;
	/*处理a+NULL的情景,这在单个表达式的时候会用到*/
    if(b.content.type==VAR_TYPE_NILL)
	{
		return a;
	}


if(a.content.type==VAR_TYPE_REAL && b.content.type==VAR_TYPE_REAL)
	{
        result.content.type=VAR_TYPE_REAL;
        result.content.var_value.real_value=a.content.var_value.real_value+b.content.var_value.real_value;
	}
    else if(a.content.type==VAR_TYPE_REAL && b.content.type==VAR_TYPE_INT)
	{
        result.content.type=VAR_TYPE_REAL;
        result.content.var_value.real_value=(a.content.var_value.real_value)+(1.0*b.content.var_value.int_value);
	}
    else if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_REAL)
	{
        result.content.type=VAR_TYPE_REAL;
        result.content.var_value.real_value=1.0*a.content.var_value.int_value+b.content.var_value.real_value;
	}
    else if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_INT)
	{
        result.content.type=VAR_TYPE_INT;
        result.content.var_value.int_value=a.content.var_value.int_value+b.content.var_value.int_value;
	}
    else if(a.content.type==VAR_TYPE_BOOL || b.content.type==VAR_TYPE_BOOL)
	{
		printf("error the boolean type's var can take part in plus-caculate\n");
		exit(0);
	}
	else
	{
		printf("this way of plus is not supported  yet %d %d\n",var_GetInt(a), var_GetType(b));
		exit(0);
	}

	return result;
}
Var var_and(Var a, Var b)
{
	Var result;
    if(a.content.type!=VAR_TYPE_BOOL || b.content.type!=VAR_TYPE_BOOL)
	{
		printf("runtime error ! 'and' can  only used in bool\n");
		exit(0);
	}
    result.content.type=VAR_TYPE_BOOL;
    result.content.var_value.bool_value =(a.content.var_value.bool_value&&b.content.var_value.bool_value);
	return result;
}

Var var_or(Var a, Var b)
{
	Var result;
    if(a.content.type!=VAR_TYPE_BOOL || b.content.type!=VAR_TYPE_BOOL)
	{
		printf("runtime error ! 'or' can  only used in bool\n");
		exit(0);
	}
    result.content.type=VAR_TYPE_BOOL;
    result.content.var_value.bool_value =(a.content.var_value.bool_value || b.content.var_value.bool_value);
	return result;
}
/*两数相减*/
Var var_minus(Var a,Var b)
{
	Var result;
    if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_INT)
	{
        result.content.type=VAR_TYPE_INT;
        result.content.var_value.int_value=a.content.var_value.int_value-b.content.var_value.int_value;
	}
    else if(a.content.type==VAR_TYPE_REAL && b.content.type==VAR_TYPE_REAL)
	{
        result.content.type=VAR_TYPE_REAL;
        result.content.var_value.int_value=a.content.var_value.real_value-b.content.var_value.real_value;
	}
    else if(a.content.type==VAR_TYPE_REAL && b.content.type==VAR_TYPE_INT)
	{
        result.content.type=VAR_TYPE_REAL;
        result.content.var_value.int_value=a.content.var_value.real_value-b.content.var_value.int_value;
	}
    else if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_REAL)
	{
        result.content.type=VAR_TYPE_REAL;
        result.content.var_value.int_value=a.content.var_value.int_value-b.content.var_value.real_value;
	}
    else if(a.content.type==VAR_TYPE_BOOL || b.content.type==VAR_TYPE_BOOL)
	{
		printf("error the boolean type's var can take part in minus-caculate\n");
		exit(0);
	}
	else
	{
		printf("this way of minusing is not supported  yet\n");
		exit(0);
	}
	return result;
}


/*两数相乘*/
Var var_multiple(Var a,Var b)
{
	Var result;
    if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_INT)
	{
        result.content.type=VAR_TYPE_INT;
        result.content.var_value.int_value=a.content.var_value.int_value*b.content.var_value.int_value;
	}
    else if(a.content.type==VAR_TYPE_REAL && b.content.type==VAR_TYPE_REAL)
	{
        result.content.type=VAR_TYPE_REAL;
        result.content.var_value.real_value=a.content.var_value.real_value*b.content.var_value.real_value;
	}
    else if(a.content.type==VAR_TYPE_REAL && b.content.type==VAR_TYPE_INT)
	{
        result.content.type=VAR_TYPE_REAL;
        result.content.var_value.real_value=a.content.var_value.real_value*b.content.var_value.int_value;
	}
    else if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_REAL)
	{
        result.content.type=VAR_TYPE_REAL;
        result.content.var_value.real_value=a.content.var_value.int_value*b.content.var_value.real_value;
	}
    else if(a.content.type==VAR_TYPE_BOOL || b.content.type==VAR_TYPE_BOOL)
	{
		printf("error the boolean type's var can take part in multiple-caculate\n");
		exit(0);
	}
	else
	{
        printf("this way of multipling is not supported  yet %d %d \n",a.content.type,b.content.type );
		exit(0);
	}
	return result;
}
/*两数相除*/
Var var_divide(Var a, Var b)
{
	Var result;
    if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_INT)
	{
		/*在整数相除时,如果两数不能整除则要转换成实数类型*/
        if(a.content.var_value.int_value%b.content.var_value.int_value==0)
		{
            result.content.type=VAR_TYPE_INT;
            result.content.var_value.int_value=a.content.var_value.int_value/b.content.var_value.int_value;
		}
		else
		{
            result.content.type=VAR_TYPE_REAL;
            result.content.var_value.real_value=var_get_value(a)/var_get_value(b);
		}
	}
    else if(a.content.type==VAR_TYPE_REAL && b.content.type==VAR_TYPE_REAL)
	{
        result.content.type=VAR_TYPE_REAL;
        result.content.var_value.real_value=a.content.var_value.real_value/b.content.var_value.real_value;
	}
    else if(a.content.type==VAR_TYPE_REAL && b.content.type==VAR_TYPE_INT)
	{
        result.content.type=VAR_TYPE_REAL;
        result.content.var_value.real_value=(a.content.var_value.real_value)/(1.0*b.content.var_value.int_value);
	}
    else if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_REAL)
	{
        result.content.type=VAR_TYPE_REAL;
        result.content.var_value.real_value=1.0*a.content.var_value.int_value/b.content.var_value.real_value;
	}
    else if(a.content.type==VAR_TYPE_BOOL || b.content.type==VAR_TYPE_BOOL)
	{
		printf("error the boolean type's var can take part in divide-caculate\n");
		exit(0);
	}
	else
	{
		printf("this way of dividing is not supported  yet\n");
		exit(0);
	}
	return result;
}

/*以浮点的形式返回一个变量的较精确值*/
double var_get_value(Var a)
{
    switch(a.content.type)
	{
	case VAR_TYPE_NILL:
		return 0;
		break;
	case VAR_TYPE_INT:
        return a.content.var_value.int_value;
		break;
	case VAR_TYPE_REAL:
        return a.content.var_value.real_value;
		break;
	case VAR_TYPE_BOOL:
        return a.content.var_value.bool_value;
		break;
    case VAR_TYPE_MESSAGE:
		return 0;
		break;
	case VAR_TYPE_HANDLE:
		return 0;
		break;
	case VAR_TYPE_FUNC:
		return 0;
		break;
    case VAR_TYPE_CHAR:
        return a.content.var_value.char_value;
        break;
    case VAR_TYPE_VECTOR:
        return 0;
        break;
	default:
        printf("other type's not support yet %d \n",a.content.type);
		exit(0);
		return -1;
		break;
	}

}



Var var_large(Var a,Var b)
{
	Var result;

    result.content.type=VAR_TYPE_BOOL;
    if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_INT)
	{
        result.content.var_value.bool_value=a.content.var_value.int_value>b.content.var_value.int_value;
	}
    else if(a.content.type==VAR_TYPE_BOOL && b.content.type==VAR_TYPE_BOOL)
	{
        result.content.var_value.bool_value=a.content.var_value.bool_value>b.content.var_value.bool_value;
	}

    else if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_BOOL)
	{
        result.content.var_value.bool_value=a.content.var_value.int_value>b.content.var_value.bool_value;
	}
    else if(a.content.type==VAR_TYPE_BOOL && b.content.type==VAR_TYPE_INT)
	{
        result.content.var_value.bool_value=a.content.var_value.bool_value>b.content.var_value.int_value;
	}
	else
	{
		printf("this way of \" large \" comparing is not supported  yet\n");
		exit(0);
	}
	return result;
}

Var var_less(Var a,Var b)
{

	Var result;
    result.content.type=VAR_TYPE_BOOL;
    if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_INT)
	{
        result.content.var_value.bool_value=a.content.var_value.int_value<b.content.var_value.int_value;
	}
    else if(a.content.type==VAR_TYPE_BOOL && b.content.type==VAR_TYPE_BOOL)
	{
        result.content.var_value.bool_value=a.content.var_value.bool_value<b.content.var_value.bool_value;
	}
    else if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_BOOL)
	{
        result.content.var_value.bool_value=a.content.var_value.int_value<b.content.var_value.bool_value;
	}
    else if(a.content.type==VAR_TYPE_BOOL && b.content.type==VAR_TYPE_INT)
	{
        result.content.var_value.bool_value=a.content.var_value.bool_value<b.content.var_value.int_value;
	}
	else
	{
		printf("this way of \" less \" comparing is not supported  yet\n");
		exit(0);
	}
	return result;
}
Var var_equal(Var a,Var b)
{
	Var result;

    result.content.type=VAR_TYPE_BOOL;
    if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_INT)
	{
        result.content.var_value.bool_value=a.content.var_value.int_value==b.content.var_value.int_value;
	}
    else if(a.content.type==VAR_TYPE_BOOL && b.content.type==VAR_TYPE_BOOL)
	{
        result.content.var_value.bool_value=a.content.var_value.bool_value==b.content.var_value.bool_value;
	}
    else if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_BOOL)
	{
        result.content.var_value.bool_value=a.content.var_value.int_value==b.content.var_value.bool_value;
	}
    else if(a.content.type==VAR_TYPE_NILL && b.content.type!=VAR_TYPE_NILL)
	{
        result.content.var_value.bool_value=0;
	}
    else if(a.content.type==VAR_TYPE_NILL && b.content.type==VAR_TYPE_NILL)
	{
        result.content.var_value.bool_value=1;
	}
    else if(a.content.type==VAR_TYPE_HANDLE && b.content.type==VAR_TYPE_INT)
	{
        result.content.var_value.bool_value=(a.content.var_value.handle_value==b.content.var_value.int_value);
	}
    else if(a.content.type==VAR_TYPE_HANDLE && b.content.type ==VAR_TYPE_NILL)
	{
        result.content.var_value.bool_value=(a.content.var_value.handle_value==0);
	}
    else if(a.content.type==VAR_TYPE_BOOL && b.content.type==VAR_TYPE_INT)
	{
        result.content.var_value.bool_value=a.content.var_value.bool_value==b.content.var_value.int_value;
	}
	else
	{
		printf("this way of \" equal \" comparing is not supported  yet\n");
		exit(0);
	}
	return result;
}

Var var_not_equal(Var a,Var b)
{
	Var result;

    result.content.type=VAR_TYPE_BOOL;
    if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_INT)
	{
        result.content.var_value.bool_value=a.content.var_value.int_value!=b.content.var_value.int_value;
	}
    else if(a.content.type==VAR_TYPE_BOOL && b.content.type==VAR_TYPE_BOOL)
	{
        result.content.var_value.bool_value=a.content.var_value.bool_value!=b.content.var_value.bool_value;
	}
    else if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_BOOL)
	{
        result.content.var_value.bool_value=a.content.var_value.int_value!=b.content.var_value.bool_value;
	}
    else if(a.content.type==VAR_TYPE_NILL && b.content.type!=VAR_TYPE_NILL)
	{
        result.content.var_value.bool_value=1;
	}
    else if(a.content.type==VAR_TYPE_NILL && b.content.type==VAR_TYPE_NILL)
	{
        result.content.var_value.bool_value=0;
	}
    else if(a.content.type==VAR_TYPE_HANDLE && b.content.type==VAR_TYPE_INT)
	{
        result.content.var_value.bool_value=(a.content.var_value.handle_value!=b.content.var_value.int_value);
	}
    else if(a.content.type==VAR_TYPE_HANDLE && b.content.type ==VAR_TYPE_NILL)
	{
        result.content.var_value.bool_value=(a.content.var_value.handle_value!=0);
	}
    else if(a.content.type==VAR_TYPE_BOOL && b.content.type==VAR_TYPE_INT)
	{
        result.content.var_value.bool_value=a.content.var_value.bool_value!=b.content.var_value.int_value;
	}
	else
	{
        printf("this way of \" not equal \" comparing is not supported yet %d %d \n",a.content.type,b.content.type);
		exit(0);
	}
	return result;
}

Var var_less_or_equal(Var a,Var b)
{
	Var result;

    result.content.type=VAR_TYPE_BOOL;
    if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_INT)
	{
        result.content.var_value.bool_value=a.content.var_value.int_value<=b.content.var_value.int_value;
	}
    else if(a.content.type==VAR_TYPE_BOOL && b.content.type==VAR_TYPE_BOOL)
	{
        result.content.var_value.bool_value=a.content.var_value.bool_value<=b.content.var_value.bool_value;
	}
    else if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_BOOL)
	{
        result.content.var_value.bool_value=a.content.var_value.int_value<=b.content.var_value.bool_value;
	}

    else if(a.content.type==VAR_TYPE_BOOL && b.content.type==VAR_TYPE_INT)
	{
        result.content.var_value.bool_value=a.content.var_value.bool_value<=b.content.var_value.int_value;
	}
	else
	{
		printf("this way of \"less or equal \" comparing is not supported yet \n");
		exit(0);
	}
	return result;
}

Var var_large_or_qual(Var a,Var b)
{

	Var result;

    result.content.type=VAR_TYPE_BOOL;
    if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_INT)
	{
        result.content.var_value.bool_value=a.content.var_value.int_value>=b.content.var_value.int_value;
	}
    else if(a.content.type==VAR_TYPE_BOOL && b.content.type==VAR_TYPE_BOOL)
	{
        result.content.var_value.bool_value=a.content.var_value.bool_value<=b.content.var_value.bool_value;
	}
    else if(a.content.type==VAR_TYPE_INT && b.content.type==VAR_TYPE_BOOL)
	{
        result.content.var_value.bool_value=a.content.var_value.int_value>=b.content.var_value.bool_value;
	}

    else if(a.content.type==VAR_TYPE_BOOL && b.content.type==VAR_TYPE_INT)
	{
        result.content.var_value.bool_value=a.content.var_value.bool_value>=b.content.var_value.int_value;
	}
    else if(a.content.type!=VAR_TYPE_HANDLE && b.content.type!=VAR_TYPE_HANDLE)
	{
        result.content.var_value.bool_value= var_get_value(a)>=var_get_value(b);
	}
	return result;
}

/*获得Var变量的内容*/
double var_GetDouble(Var a)
{
	if(var_GetType(a)==VAR_TYPE_REAL)
	{
        return a.content.var_value.real_value;
	}
	else
	{
		STOP("ERROR  Var_GetDouble : invalid type");
	}

}
/*获得Var变量的内容*/
int var_GetInt(Var a)
{
	if(var_GetType(a)==VAR_TYPE_INT)
	{
        return a.content.var_value.int_value;
	}
	else
	{
        STOP("ERROR  var_GetInt : invalid type");
	}
}

int var_GetBool(Var a)
{
    if(var_GetType(a)==VAR_TYPE_BOOL)
	{
        return a.content.var_value.bool_value;
	}
	else
	{
        STOP("ERROR  var_GetBool : invalid type");
	}
}

char var_GetChar(Var a)
{
    if(var_GetType(a)==VAR_TYPE_CHAR)
    {
        return a.content.var_value.char_value;
    }
    else
    {
        STOP("ERROR  var_GetChar : invalid type");
    }
}

/*获得变量的值类型*/
int var_GetType(Var a)
{
    return a.content.type;
}
void * var_getHandle(Var a)
{
    return a.content.var_value.handle_value;
}

/*设置Var变量的实数值，如果其不为实数，类型将会强制转换*/
void var_SetReal(Var *a,double value)
{
    a->content.var_value.real_value=value;
    a->content.type=VAR_TYPE_REAL;
}

/*设置Var变量的字符值，如果其不为字符，类型将会强制转换*/
void var_SetChar(Var *a,int ch)
{
    a->content.var_value.char_value=ch;
    a->content.type=VAR_TYPE_CHAR;
}

char * var_GetMsg(Var a)
{
    return a.content.var_value.str;
}


/*设置Var变量的整数值，如果其不为整数，类型将会强制转换*/
void var_SetInt(Var *a,int value)
{
    a->content.var_value.int_value=value;
    a->content.type=VAR_TYPE_INT;
}


/*设置Var变量的布尔值，如果其不为布尔，类型将会强制转换*/
void var_SetBool(Var *a,int value)
{
    a->content.var_value.bool_value=value;
    a->content.type=VAR_TYPE_BOOL;
}

/*设置一个指定的Var变量的值类型为Nil*/
void var_SetNil(Var *a)
{
    a->content.type=VAR_TYPE_NILL;
}

/*将指定的Var变量转换成API形式*/
void var_SetAPI(Var *a ,int API_index)
{
    a->content.var_value.func.func_index=API_index;
    a->content.var_value.func.func_type=FUNC_API;
}

/*在stdout里打印Var变量的值，注意不带换行*/
void var_Print(Var a)
{
	switch(var_GetType(a))
	{
	case VAR_TYPE_INT:
		printf("%d",var_GetInt(a));
		break;
	case VAR_TYPE_REAL:
		printf("%g",var_GetDouble(a));
		break;
	case VAR_TYPE_BOOL:
		printf("%d",var_GetBool(a));
		break;
	case VAR_TYPE_NILL:
		printf("nil");
		break;
    case VAR_TYPE_CHAR:
        printf("%c",var_GetChar(a));
        break;
    case VAR_TYPE_MESSAGE:
        printf("%s",a.content.var_value.str);
        break;
	default:
		printf("var_Print : not support yet %d\n",var_GetType(a));
		break;
	}
}















