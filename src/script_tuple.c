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
#include "script_tuple.h"
#include "var.h"
#include "api.h"
#include<assert.h>
#include <stdlib.h>
#define NDEBUG
#include "debug.h"
typedef struct tuple_chunk_t
{
    int tuple_size;
	int ref_count;
	Var * var_handle;
} tuple_chunk;





/*创建数组*/
static tuple_chunk * CreateTuple(int tuple_size)
{
    tuple_chunk * new_array=malloc(sizeof(tuple_chunk));
    new_array->tuple_size=tuple_size;
    new_array->var_handle= malloc(sizeof(Var) * tuple_size);
	new_array->ref_count=0;
	Var * array= ( Var * ) new_array->var_handle;
	{
		int i;
		/*初始化*/
        for ( i=0; i<tuple_size; i++ )
		{
            var_SetInt (array+i,0);
		}
	}
	return new_array;
}






/*获得数组指定元素的值*/
Var *tuple_GetValue(Var array_obj,int index )
{
    tuple_chunk * a;
    a=(tuple_chunk *)var_getHandle (array_obj);
    if(index <a->tuple_size+1&& index>0)
	{
        Var * the_tuple =a->var_handle;
        return  & (the_tuple[index-1]);
	}
	else
	{
        if(abs(index)<a->tuple_size+1&& index<0)
        {
            Var * the_tuple =a->var_handle;
            return  & (the_tuple[a->tuple_size+index]);
        }
        else
        {
            STOP("tuple getter: index out of bound!");
        }
	}
}

/*为数组的指定元素赋值*/
void tuple_SetValue(Var array_obj,int index ,Var new_value)
{
    tuple_chunk * a;
    a=(tuple_chunk *)var_getHandle (array_obj);
    if(index <a->tuple_size+1 && index>0)
	{
        Var * the_tuple =a->var_handle;
        the_tuple[index-1]=new_value;
	}
	else
	{
        if(abs(index)<a->tuple_size+1&& index<0)
        {
            Var * the_tuple =a->var_handle;
            the_tuple[a->tuple_size+index]=new_value;
        }
        else
        {
            STOP("tuple getter: index out of bound!");
        }
	}
}

Var tuple_CreateByList(int size,Var init_arg[])
{
        Var result;
        result.content.type=VAR_TYPE_TUPLE;
        tuple_chunk * new_obj = CreateTuple(size);
        result.content.var_value.handle_value=new_obj;
        int i=0;
        for(;i<size;i++)
        {
            tuple_SetValue (result,i+1,init_arg[i]);
        }
        return result;
}

Var tuple_CreateBySize(int size)
{
    Var result;
    result.content.type=VAR_TYPE_TUPLE;
   tuple_chunk * new_obj = CreateTuple(size);
    result.content.var_value.handle_value=new_obj;
    Var a;
    var_SetInt (&a,0);
    int i=0;
    for(;i<size;i++)
    {
        tuple_SetValue (result,i+1,a);
    }
    return result;
}

void script_tuple_init()
{


}

/*以指定字符串作为资源，创建一个向量，该向量的各个维度为字符串中的各字符，成员包括末尾的终结符*/
Var  tuple_CreateByString(const char * str)
{
    int str_size=strlen(str)+1;
    tuple_chunk * new_tuple=CreateTuple (str_size);
    Var obj;
    obj.content.var_value.handle_value=new_tuple;
    obj.content.type=VAR_TYPE_TUPLE;
    int i=0;
    for(;i<str_size;i++)
    {
        Var ch;
        var_SetChar(&ch,str[i]);
        tuple_SetValue(obj,i+1,ch);
    }
    return obj;
}


/*获取容器的基数*/
int tuple_GetCard(Var tuple)
{
    if(var_GetType (tuple)!=VAR_TYPE_TUPLE)
    {
        STOP("the 'card' only use for the tuple value");
    }
return ((tuple_chunk *)tuple.content.var_value.handle_value)->tuple_size;
}



/*打印一个元组的所有成员*/
void tuple_print(Var vec)
{
    if(var_GetType (vec)!=VAR_TYPE_TUPLE)
    {
        STOP("the obj is not a tuple");
    }
    tuple_chunk *the_vec= var_getHandle (vec);
    int i=1;
    int vec_size=the_vec->tuple_size;
    for(;i<vec_size+1;i++)
    {
        var_Print (*tuple_GetValue (vec,i));
        printf(" ");
    }
}
