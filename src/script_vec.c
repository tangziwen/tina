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
typedef struct vec_chunk_t
{
    int vec_size;
    int ref_count;
    int content_type;
	Var * var_handle;
    struct vec_chunk_t *tmp_next;
    struct vec_chunk_t *tmp_pre;
} vec_chunk;
/*临时数组对象*/
static vec_chunk * VecTmpPool=NULL;


/*创建数组*/
static vec_chunk * CreateVec(int tuple_size)
{
    vec_chunk * new_array=malloc(sizeof(vec_chunk));
    new_array->vec_size=tuple_size;
    new_array->var_handle= malloc(sizeof(Var) * tuple_size);
	new_array->ref_count=0;
    new_array->content_type=VAR_TYPE_NILL;
    new_array->tmp_next=VecTmpPool;
	new_array->tmp_pre=NULL;
    VecTmpPool=new_array;
	return new_array;
}


/*获得数组指定元素的值*/
Var *vector_GetValue(Var array_obj,int index )
{
    vec_chunk * a;
    a=(vec_chunk *)var_getHandle (array_obj);
    if(index <a->vec_size+1&& index>0)
	{
        Var * the_tuple =a->var_handle;
        return &(the_tuple[index-1]);
	}
	else
	{
        STOP("vec getter: index out of bound!");
	}
}

/*为数组的指定元素赋值*/
void vector_SetValue(Var array_obj,int index ,Var new_value)
{
    vec_chunk * a;
    a=(vec_chunk *)var_getHandle (array_obj);
    if(index <a->vec_size+1 && index>0)
    {
        if(a->content_type!=var_GetType (new_value))
        {
            STOP("differnce type assignment catch by vector setter !!!!");
        }

        Var * array =a->var_handle;
        array[index-1]=new_value;
    }
    else
    {
        STOP("vec setter: index out of bound!");
    }
}

Var the_vector_creator(int size,Var init_arg[])
{
	Var result;
    result.content.type=VAR_TYPE_VECTOR;
   vec_chunk * new_obj = CreateVec(size);
    new_obj->content_type=var_GetType (init_arg[0]);
    result.content.var_value.handle_value=new_obj;
    int i=0;
    for(;i<size;i++)
    {
        if(new_obj->content_type!=var_GetType (init_arg[i]))
        {
            STOP("invalid vector");
        }
        vector_SetValue (result,i+1,init_arg[i]);
    }
    return result;
}

/*获取数组的长度*/
static void getVectorLength()
{
	Var result;
	result.content.type=VAR_TYPE_INT;
	if(API_GetArgCount()!=1)
	{
        printf("the \" vec_size \" function only need one args!!\n");
		exit(0);
	}
    Var  the_tuple_var = API_argument_list[0];
    if(the_tuple_var.content.type!=VAR_TYPE_VECTOR)
	{
        printf("vec_size only support array type!\n");
		exit(0);
	}
    vec_chunk * a=var_getHandle (the_tuple_var);
    var_SetInt (&result,a->vec_size);
	Tina_API_SetReturn ( result );
}


static void free_vector(vec_chunk * ptr)
{
	test(
		printf("free array  %d\n",ptr);
	);
	/*数组的成员可能也是对象或数组的引用,
	因此我们要判断出来,并使其指向的对象的引用计数器自减
	*/
{	int i=0;
    for(;i<ptr->vec_size;i++)
	{
		int t=var_GetType(ptr->var_handle[i]);
        if(t==VAR_TYPE_VECTOR || t==VAR_TYPE_OBJ ||t==VAR_TYPE_TUPLE)
		{
            RefCountDecrease(t,var_getHandle (ptr->var_handle[i]));
		}
	}
}
	free(ptr);
}


void VectorRefCountIncrease(void * ptr)
{
    vec_chunk * obj=(vec_chunk*)ptr;
	if(obj->ref_count==0)
	{
        if(VecTmpPool==obj)
		{
            VecTmpPool=obj->tmp_next;
		}
		else
		{
            vec_chunk*pre=obj->tmp_pre;
            vec_chunk*next=obj->tmp_next;
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
void vector_CleanTmpPool()
{
    vec_chunk * node=VecTmpPool;
	while(node)
	{
        vec_chunk * next=node->tmp_next;
        free_vector(node);
		node=next;
	}
}

/*以指定字符串作为资源，创建一个向量，该向量的各个维度为字符串中的各字符，成员包括末尾的终结符*/
void * vector_CreateByString(const char * str)
{
    int str_size=strlen(str)+1;
    vec_chunk * new_vec=CreateVec (str_size);
    new_vec->content_type=VAR_TYPE_CHAR;
    Var obj;
    obj.content.var_value.handle_value=new_vec;
    obj.content.type=VAR_TYPE_VECTOR;
    int i=0;
    for(;i<str_size;i++)
    {
        Var ch;
        var_SetChar(&ch,str[i]);
        vector_SetValue (obj,i+1,ch);
    }
    return var_getHandle (obj);
}

void VectorRefCountDecrease(void * ptr)
{
    vec_chunk * obj=(vec_chunk*)ptr;
	obj->ref_count--;
	if(obj->ref_count<=0)
	{
        free_vector(obj);
	}
}

void script_vector_init()
{
}

/*打印一个向量的所有成员*/
void vector_Print(Var vec)
{
    if(var_GetType (vec)!=VAR_TYPE_VECTOR)
    {
        STOP("the obj is not a vector");
    }
    vec_chunk *the_vec= var_getHandle (vec);
    int i=1;
    int vec_size=the_vec->vec_size;
    for(;i<vec_size+1;i++)
    {
        var_Print (*vector_GetValue (vec,i));
    }
    printf("\n");
}


/*从原位置克隆一个向量*/
Var * vector_Clone(Var src)
{
    if(var_GetType (src)!=VAR_TYPE_VECTOR)
    {
        STOP("vector_Clone invalid src ");
    }

    vec_chunk * src_vec, *new_vec;
    src_vec=var_getHandle (src);
    new_vec= CreateVec (src_vec->vec_size);
    new_vec=src_vec;/*拷贝所有属性*/
    /*拷贝成员*/
    int i=1;
    for(;i<=src_vec->vec_size;i++)
    {
        new_vec->var_handle[i]=src_vec->var_handle[i];
    }
    /*装箱成Var 类型*/
    Var  *new_obj=malloc(sizeof (Var));
    new_obj->content.type=VAR_TYPE_VECTOR;
    new_obj->content.var_value.handle_value=new_vec;
    return new_obj;
}

/*获得指定向量维护的类型*/
int vector_GetType(Var vec)
{
    if(vec.content.type!=VAR_TYPE_VECTOR)
    {
        STOP("vector_GetType ,not a vector value");
    }
    vec_chunk * a= var_getHandle (vec);
    return a->content_type;
}

/*获得指定向量所存在的成员个数*/
int vector_GetSize(Var vec)
{

    if(vec.content.type!=VAR_TYPE_VECTOR)
    {
        STOP("vector_GetSize ,not a vector value");
    }
    vec_chunk * a= var_getHandle (vec);
    return a->vec_size;
}

/*将一个向量的元素，若其维护的值为VAR_TYPE_CHAR的话，转移至一个char型数组里。该空间有使用者回收*/
char * vector_ToString(Var vec)
{
    if(vec.content.type!=VAR_TYPE_VECTOR)
{
        STOP("vector_ToString invalid obj");
    }
    vec_chunk * obj=var_getHandle (vec);
    if(obj->content_type!=VAR_TYPE_CHAR)
    {
        STOP("vector_ToString invalid obj");
    }
    char * str=malloc(sizeof(obj->vec_size));
    int i=0;
    for(;i<obj->vec_size;i++)
    {
        //str[i]=var_GetChar (vector_GetValue (vec,i+1));
    }
    return str;
}
