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
    struct tuple_chunk_t *tmp_next;
    struct tuple_chunk_t *tmp_pre;
} tuple_chunk;
/*临时数组对象*/
static tuple_chunk * TupleTmpPool=NULL;


/*创建数组*/
static tuple_chunk * CreateTuple(int tuple_size)
{
    tuple_chunk * new_array=malloc(sizeof(tuple_chunk));
    new_array->tuple_size=tuple_size;
    new_array->var_handle= malloc(sizeof(Var) * tuple_size);
	new_array->ref_count=0;

    new_array->tmp_next=TupleTmpPool;
	new_array->tmp_pre=NULL;
    TupleTmpPool=new_array;
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
Var tuple_GetValue(Var array_obj,int index )
{
    tuple_chunk * a;
    a=(tuple_chunk *)var_getHandle (array_obj);
    if(index <a->tuple_size+1&& index>0)
	{
        Var * the_tuple =a->var_handle;
        return the_tuple[index-1];
	}
	else
	{
        STOP("tuple getter: index out of bound!");
	}
}

/*为数组的指定元素赋值*/
void tuple_SetValue(Var array_obj,int index ,Var new_value)
{
    tuple_chunk * a;
    a=(tuple_chunk *)var_getHandle (array_obj);
    if(index <a->tuple_size+1 && index>0)
	{
		Var * array =a->var_handle;
        array[index-1]=new_value;
	}
	else
	{
        STOP("tuple setter: index out of bound!");
	}
}

Var the_tuple_creator(int size,Var init_arg[])
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

/*获取数组的长度*/
static void getTupleLength()
{
	Var result;
	result.content.type=VAR_TYPE_INT;
	if(API_GetArgCount()!=1)
	{
        printf("the \" tuple_size \" function only need one args!!\n");
		exit(0);
	}
    Var  the_tuple_var = API_argument_list[0];
    if(the_tuple_var.content.type!=VAR_TYPE_TUPLE)
	{
        printf("tuple_size only support array type!\n");
		exit(0);
	}
    tuple_chunk * a=var_getHandle (the_tuple_var);
    var_SetInt (&result,a->tuple_size);
	Tina_API_SetReturn ( result );
}


static void free_array(tuple_chunk * ptr)
{
	test(
		printf("free array  %d\n",ptr);
	);
	/*数组的成员可能也是对象或数组的引用,
	因此我们要判断出来,并使其指向的对象的引用计数器自减
	*/
{	int i=0;
    for(;i<ptr->tuple_size;i++)
	{
		int t=var_GetType(ptr->var_handle[i]);
        if(t==VAR_TYPE_TUPLE || t==VAR_TYPE_HANDLE ||t==VAR_TYPE_VECTOR)
		{
            RefCountDecrease(t,var_getHandle (ptr->var_handle[i]));
		}
	}
}
	free(ptr);
}


void TupleRefCountIncrease(void * ptr)
{
    tuple_chunk * obj=(tuple_chunk*)ptr;
	if(obj->ref_count==0)
	{
        if(TupleTmpPool==obj)
		{
            TupleTmpPool=obj->tmp_next;
		}
		else
		{
            tuple_chunk*pre=obj->tmp_pre;
            tuple_chunk*next=obj->tmp_next;
			if(pre)
			{
				pre->tmp_next=next;
			}
			if(next)
			{
				next->tmp_pre=pre;
			}
		}
	}
	obj->ref_count++;
}

/*清除数组临时对象池*/
void tuple_CleanTmpPool()
{
    tuple_chunk * node=TupleTmpPool;
	while(node)
	{
        tuple_chunk * next=node->tmp_next;
		free_array(node);
		node=next;
	}
}

void TupleRefCountDecrease(void * ptr)
{
    tuple_chunk * obj=(tuple_chunk*)ptr;
	obj->ref_count--;
	if(obj->ref_count<=0)
	{
		free_array(obj);
	}
}

void script_tuple_init()
{
    Tina_API_Register ( "tuple",the_tuple_creator,0);
    Tina_API_Register ( "tuple_length",getTupleLength,0);
}
