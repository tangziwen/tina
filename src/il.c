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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "il.h"
#include "var.h"
#include "vm.h"
#include "function.h"
#include "api.h"
#include "return.h"
#include "script_tuple.h"
#include "script_vec.h"
/*当前的标签数*/
int label_index=0;
/*储存上一个被使用的临时节点的值*/
Var last_tmp_value;
/*函数的调用*/
Var IL_CallFunc (IL_element var, int index,int mode);
/*自指针*/
Var self_ptr;
Var func_return_value;
IL_list * current_list=NULL;
static void set_tmp(int index,Var var)
{
	current_list->tmp_var_list[index]=var;
}
static Var get_tmp(int index)
{
	return  current_list->tmp_var_list[index];
}
/*创建一个中间语言表达式*/
IL_exp * IL_exp_create ( char op,int tmp_var_index,IL_element var_a,IL_element var_b )
{
	static int exp_index=0;
	IL_exp * exp= ( IL_exp* ) malloc ( sizeof ( IL_exp ) );
	exp->op=op;
	exp->A=var_a;
	exp->B=var_b;
	exp->tmp_index=tmp_var_index;
	exp_index++;
	exp->id=exp_index;
	return exp;
}

IL_node * IL_node_create_return()
{
	IL_node * node = ( IL_node * ) malloc ( sizeof ( IL_node ) );
	node->retrn = ( IL_return* ) malloc ( sizeof ( IL_return ) );
	node->type=IL_NODE_RETURN;
	return node;
}

void IL_ListInsertReturn()
{
	IL_ListInsertNode ( IL_node_create_return() );
}

/*函数的调用*/
Var IL_CallFunc (IL_element var, int index,int mode);
void IL_ListInsertNode ( IL_node *node )
{
	if ( func_get_current()->list.head==NULL )
	{
		func_get_current()->list.head=node;
		node->pre=NULL;
		node->next=NULL;
	}
	else
	{
		IL_node *tmp=func_get_current()->list.head;
		while ( tmp->next!=NULL )
		{
			tmp=tmp->next;
		}
		tmp->next=node;
		node->pre=tmp;
		node->next=NULL;
	}
}
/*向中间语言的执行序列中增加一个表达式*/
void IL_ListInsert ( IL_exp *exp )
{
	if ( func_get_current()->list.head==NULL )
	{
		func_get_current()->list.head=IL_node_create_exp ( exp );
		func_get_current()->list.head->next=NULL;
	}
	else
	{
		IL_node *tmp=func_get_current()->list.head;
		while ( tmp->next!=NULL )
		{
			tmp=tmp->next;
		}
		IL_node * node= IL_node_create_exp ( exp );
		tmp->next= node;
		node->pre=tmp;
		node->next=NULL;
	}
}

void IL_print_exp ( IL_node * tmp )
{
	printf ( "@ %d ",tmp->exp->tmp_index );
	printf ( "=" );
	switch ( tmp->exp->A.type )
	{
	case ELEMENT_VAR:
		printf ( " var-ID %d",tmp->exp->A.index );
		break;
	case ELEMENT_NUM:
		printf ( "%g",var_get_value ( tmp->exp->A.value ) );
		break;
	case ELEMENT_TMP:
		printf ( "@%d",tmp->exp->A.index );
		break;
	case ELEMENT_API:
		printf ( "API %d",tmp->exp->A.index );
		break;
	case ELEMENT_FUNC:
		printf ( "FUNC %d",tmp->exp->A.index );
		break;
	case ELEMENT_CALL_BY_MEMBER:
		printf ( "CALL MEMBER %d",tmp->exp->A.index );
		break;
	case ELEMENT_ARRAY:
		printf("ARRAY %d",tmp->exp->A.index);
		break;
	case ELEMENT_STRUCT:
		printf("STRUCT %s",tmp->exp->A.value.content.str);
		break;
	}
	printf ( " %c ",tmp->exp->op );
	switch ( tmp->exp->B.type )
	{
	case ELEMENT_VAR:
		printf ( " var-ID %d",tmp->exp->B.index );
		break;
	case ELEMENT_NUM:
		if ( tmp->exp->B.value.content.type==VAR_TYPE_NILL )
		{
			printf ( "NULL" );
		}
		else
		{
            if(tmp->exp->B.value.content.type==VAR_TYPE_MESSAGE)
			{
				printf ( "%s",tmp->exp->B.value.content.str );
			}
			else
			{
				printf ( "%g",var_get_value ( tmp->exp->B.value ) );
			}

		}
		break;
	case ELEMENT_TMP:
		printf ( "@%d",tmp->exp->B.index );
		break;
	case ELEMENT_API:
		printf ( "API %d",tmp->exp->B.index );
		break;
	case ELEMENT_FUNC:
		printf ( "FUNC %d",tmp->exp->B.index );
		break;
	case ELEMENT_ARRAY:
		printf("ARRAY %d",tmp->exp->B.index);
		break;
	}
	printf ( "\n" );
}



void IL_print_jmp ( IL_node *node )
{
	printf ( "JMP TO %d\n",node->jmp->label );
}
void IL_print_je ( IL_node * node )
{
	printf ( "JE TO %d\n",node->jmp->label );
}
void IL_print_jne ( IL_node *node )
{
	printf ( "JNE to %d\n",node->jmp->label );
}
void IL_print_lab ( IL_node * node )
{
	printf ( "LABEL : %d \n",node->jmp->label );
}

void IL_printf_prnt ( IL_node *node )
{
	printf ( "PRNT :\n" );
}
/*打印中间代码执行序列*/
void IL_list_print ( Function * func )
{
	IL_node * tmp=func->list.head;
	while ( tmp!=NULL )
	{
		switch ( tmp->type )
		{
		case IL_NODE_EXP:
			IL_print_exp ( tmp );
			break;
		case IL_NODE_JE:
			IL_print_je ( tmp );
			break;
		case IL_NODE_JMP:
			IL_print_jmp ( tmp );
			break;
		case IL_NODE_JNE:
			IL_print_jne ( tmp );
			break;
		case IL_NODE_LAB:
			IL_print_lab ( tmp );
			break;
		case IL_NODE_PRNT:
			IL_printf_prnt ( tmp );
			break;
		case IL_NODE_RETURN:
			printf ( "return node \n" );
			break;
		}
		tmp=tmp->next;
	}
}

/*根据函数名称打印中间代码*/
void Tina_PrintIL ( const char * func_name )
{
	IL_list_print ( func_get_by_index ( func_get_index_by_name ( func_name ) ) );

}

/*创建表达式型节点*/
IL_node * IL_node_create_exp ( IL_exp * exp )
{
	IL_node * node= ( IL_node* ) malloc ( sizeof ( IL_node ) );
	node->exp=exp;
	node->next=NULL;
	node->type=IL_NODE_EXP;
	return node;
}


IL_node * IL_node_create_jmp ( int label ,int mode )
{
	IL_node * node = ( IL_node * ) malloc ( sizeof ( IL_node ) );
	node->jmp = ( IL_jmp* ) malloc ( sizeof ( IL_jmp ) );
	node->jmp->label=label;
	node->type=mode;
	return node;
}

IL_node * IL_node_create_prnt ( )
{
	IL_node * node = ( IL_node * ) malloc ( sizeof ( IL_node ) );
	node->prnt= ( IL_print * ) malloc ( sizeof ( IL_print ) );
	node->type=IL_NODE_PRNT;
	return node;
}

Var  IL_rt_element_to_array_ptr ( IL_element var, IL_node * tmp )
{
	if(var.type==ELEMENT_ARRAY)
	{
		Var a=vm_rt_stack_var_get ( var.index );
        if ( a.content.type!=VAR_TYPE_TUPLE && a.content.type!=VAR_TYPE_VECTOR  )
		{
			printf ( "error !!  none array var can use '['  ']' to access!\n " );
			exit ( 0 );
		}
		else
		{
			/*前一个临时变量即为下标*/
			if(get_tmp(var.array_index).content.type==VAR_TYPE_INT)
			{
                Var result;
                if(a.content.type==VAR_TYPE_TUPLE)
                {
                  result =  tuple_GetValue(a,get_tmp(var.array_index).content.int_value);
                }
                else
                {
                    result =  vector_GetValue(a,get_tmp(var.array_index).content.int_value);
                }
				return result;
			}
			else
			{
				printf("error !the array subscript must be integer!\n");
				exit(0);
			}
		}
	}
}
/*在运行时将节点根据其指向的类别,转换成适当的Var型变量,如果是函数则为其返回值*/
static Var IL_rt_element_to_var ( IL_element element, IL_node * tmp )
{
	Var result ;
	result.content.real_value=0;
	result.content.type=VAR_TYPE_REAL;
	result.index=element.index;/*变量也要储存其自身所在的索引,因为在引用型变量中会使用到*/
	switch ( element.type )
	{
	case ELEMENT_VAR:
		result= vm_rt_stack_var_get ( element.index );
		break;

	case ELEMENT_NUM:
		result= element.value;
		break;

	case ELEMENT_TMP:
		result= get_tmp(element.index);
		break;
		/*自指向*/
	case ELEMENT_SELF:
		result=self_ptr;
		break;
	case ELEMENT_API:
	{

		int old_index =env_index;
		env_index =ENV_GLOBAL;
		result = IL_CallFunc (element, element.index,FUNC_API );
		env_index=old_index;
		return result;

	}
	break;
	case ELEMENT_STRUCT:
	{
		result.content.str=element.value.content.str;
		result.content.type=VAR_TYPE_STRUCT_NAME;
		return result;
	}
	break;
	case ELEMENT_ARRAY:
	{
		Var a=vm_rt_stack_var_get ( element.index );
        if ( a.content.type!=VAR_TYPE_TUPLE && a.content.type!=VAR_TYPE_VECTOR )
		{
			printf ( "error !!  none array element can use '['  ']' to access!\n " );
			exit ( 0 );
		}
		else
		{
			/*获得下标*/
			if(get_tmp(element.array_index).content.type==VAR_TYPE_INT)
			{
                if(a.content.type==VAR_TYPE_TUPLE)
                {
result=tuple_GetValue(a,get_tmp(element.array_index).content.int_value);
                }
                else
                {
result=vector_GetValue (a,get_tmp(element.array_index).content.int_value);
                }

			}
			else
			{
				printf("error !the array subscript must be integer!\n");
				exit(0);
			}
		}
	}
	break;
	case ELEMENT_FUNC:
	{
		int old_index =env_index;
		env_index =ENV_GLOBAL;
		result = IL_CallFunc (element, element.index ,FUNC_NORMAL);
		env_index =old_index;
		return result;
	}
	break;
	case ELEMENT_CALL_BY_PTR:
	{
		int old_index =env_index;
		env_index =ENV_GLOBAL;
		result = IL_CallFunc (element, vm_rt_stack_var_get(element.index).content.func.func_index, vm_rt_stack_var_get(element.index).content.func.func_type);
		env_index =old_index;
		return result;
	}
	break;
	case ELEMENT_CALL_BY_MEMBER:
	{

		int old_index =env_index;

		env_index=self_ptr.class_id;
		result = IL_CallFunc (element,  get_tmp(element.index).content.func.func_index ,get_tmp(element.index).content.func.func_type);
		env_index=old_index;
		return result;
	}
	break;
	default :
		printf ( "can't resolve this element type : %d\n",element.type );
		exit ( 0 );
		return result;
	}
	result.index=element.index;
	return result;
}



/*在运行时解析节点的加法运算*/
static void rt_eval_plus ( IL_node *tmp )
{
	Var value_a,value_b;
	/*根据栈中的函数调用情况获取栈中绝对定位下的变量*/
	value_a=IL_rt_element_to_var ( tmp->exp->A,tmp );

	value_b=IL_rt_element_to_var ( tmp->exp->B,tmp );
	last_tmp_value=var_add ( value_a,value_b );
	set_tmp(tmp->exp->tmp_index,last_tmp_value);
}

/*在运行时解析对象成员指向运算*/
static void rt_eval_point_to(IL_node *tmp)
{
	if(tmp->exp->A.type!=ELEMENT_VAR && tmp->exp->A.type!=ELEMENT_STRUCT && tmp->exp->A.type!=ELEMENT_API && tmp->exp->A.type!=ELEMENT_SELF &&  tmp->exp->A.type!=ELEMENT_TMP)
	{
		printf("doesn't support this kind of point operate,A : index %d    type %d \n",tmp->exp->tmp_index,tmp->exp->A.type);
		exit(0);
	}
	Var value_a,value_b;
	value_a=IL_rt_element_to_var ( tmp->exp->A,tmp );
	value_b=IL_rt_element_to_var ( tmp->exp->B,tmp );
	if(value_a.content.type==VAR_TYPE_STRUCT_NAME) /*类成员*/
	{
		last_tmp_value=var_point_to ( value_a,value_b );
		last_tmp_value.address=GetPrototypeMemberAddress(value_a.content.str,value_b.content.str);
		/*类成员，self指针失效，同时环境索引变为全局*/
		self_ptr.class_id=env_index;
		self_ptr.content.type=VAR_TYPE_NILL;
	}
	else /*对象成员*/
	{

		/*判断访问性*/
		int acc=get_accessbility_of_member(value_a.class_id,get_index_of_member(value_a.class_id,value_b.content.str));
		if(acc!=STRUCT_PUBLIC && env_index != value_a.class_id)/*非公有且在外部*/
		{
			printf("private or sealed member cannot be assinged outside\n");
			exit(0);
		}
		last_tmp_value=var_point_to ( value_a,value_b );
		/*设置self指针*/
		self_ptr= value_a;
		/*指向成员的运算的结果为其成员的地址*/
		last_tmp_value.address=GetObjectMemberAddress(value_a.content.handle_value,get_index_of_member(value_a.class_id,value_b.content.str));
	}
	set_tmp(tmp->exp->tmp_index,last_tmp_value);
}

/*在运行时解析节点的乘法运算*/
static void rt_eval_mutiple ( IL_node *tmp )
{
	Var value_a,value_b;
	value_a=IL_rt_element_to_var ( tmp->exp->A,tmp );
	value_b=IL_rt_element_to_var ( tmp->exp->B,tmp );
	last_tmp_value=var_multiple ( value_a,value_b );
	set_tmp(tmp->exp->tmp_index,last_tmp_value);
}



/*在运行时解析节点的减法运算*/
static void rt_eval_minus ( IL_node *tmp )
{
	Var value_a,value_b;
	value_a=IL_rt_element_to_var ( tmp->exp->A,tmp );
	value_b=IL_rt_element_to_var ( tmp->exp->B,tmp );
	last_tmp_value=var_minus ( value_a,value_b );
	set_tmp(tmp->exp->tmp_index,last_tmp_value);

}
/*在运行时解析节点的除法运算*/
static void rt_eval_divide ( IL_node *tmp )
{
	Var value_a,value_b;
	value_a=IL_rt_element_to_var ( tmp->exp->A,tmp );
	value_b=IL_rt_element_to_var ( tmp->exp->B,tmp );
	last_tmp_value=var_divide ( value_a,value_b );
	set_tmp(tmp->exp->tmp_index,last_tmp_value);
}

/*在运行时解析节点的引用运算*/
static void rt_eval_ref ( IL_node *tmp )
{
	Var value_a;
	value_a=IL_rt_element_to_var ( tmp->exp->A,tmp );
	Var result;
	result.content.type=VAR_TYPE_REF;
	result.content.handle_value=vm_GetAbs ( value_a.index )  ;
	set_tmp(tmp->exp->tmp_index,result);
}

static void rt_eval_large ( IL_node *tmp )
{
	Var value_a,value_b;
	value_a=IL_rt_element_to_var ( tmp->exp->A,tmp );
	value_b=IL_rt_element_to_var ( tmp->exp->B,tmp );
	last_tmp_value=var_large ( value_a,value_b );
	set_tmp(tmp->exp->tmp_index,last_tmp_value);
}


static void rt_eval_less ( IL_node *tmp )
{
	Var value_a,value_b;
	value_a=IL_rt_element_to_var ( tmp->exp->A,tmp );
	value_b=IL_rt_element_to_var ( tmp->exp->B,tmp );
	last_tmp_value=var_less ( value_a,value_b );
	set_tmp(tmp->exp->tmp_index,last_tmp_value);
}


static void rt_eval_less_or_equal ( IL_node *tmp )
{
	Var value_a,value_b;
	value_a=IL_rt_element_to_var ( tmp->exp->A,tmp );
	value_b=IL_rt_element_to_var ( tmp->exp->B,tmp );
	last_tmp_value=var_less_or_equal ( value_a,value_b );
	set_tmp(tmp->exp->tmp_index,last_tmp_value);
}



static void rt_eval_large_or_equal ( IL_node *tmp )
{
	Var value_a,value_b;
	value_a=IL_rt_element_to_var ( tmp->exp->A,tmp );
	value_b=IL_rt_element_to_var ( tmp->exp->B,tmp );
	last_tmp_value=var_large_or_qual ( value_a,value_b );
	set_tmp(tmp->exp->tmp_index,last_tmp_value);
}



static void rt_eval_equal ( IL_node *tmp )
{
	Var value_a,value_b;
	value_a=IL_rt_element_to_var ( tmp->exp->A,tmp );
	value_b=IL_rt_element_to_var ( tmp->exp->B,tmp );
	last_tmp_value=var_equal ( value_a,value_b );
	set_tmp(tmp->exp->tmp_index,last_tmp_value);
}



static void rt_eval_not_equal ( IL_node *tmp )
{
	Var value_a,value_b;
	value_a=IL_rt_element_to_var ( tmp->exp->A,tmp );
	value_b=IL_rt_element_to_var ( tmp->exp->B,tmp );
	last_tmp_value=var_not_equal ( value_a,value_b );
	set_tmp(tmp->exp->tmp_index,last_tmp_value);
}


static void rt_eval_and ( IL_node *tmp )
{
	Var value_a,value_b;
	value_a=IL_rt_element_to_var ( tmp->exp->A,tmp );
	value_b=IL_rt_element_to_var ( tmp->exp->B,tmp );
	last_tmp_value=var_and ( value_a,value_b );
	set_tmp(tmp->exp->tmp_index,last_tmp_value);
}


static void rt_eval_or ( IL_node *tmp )
{
	Var value_a,value_b;
	value_a=IL_rt_element_to_var ( tmp->exp->A,tmp );
	value_b=IL_rt_element_to_var ( tmp->exp->B,tmp );
	last_tmp_value=var_or ( value_a,value_b );
	set_tmp(tmp->exp->tmp_index,last_tmp_value);
}

static Var assign_get(Var a,Var b,int mode)
{
	switch(mode)
	{
	case '=':
		return b;
		break;
	case OP_ASSIGN_PLUS:
		return var_add(a,b);
		break;
	case OP_ASSIGN_DIVIDE:
		return var_divide(a,b);
		break;
	case OP_ASSIGN_MINUS:
		return  var_minus(a,b);
		break;
	case OP_ASSIGN_MULTIPLE:
		return var_multiple(a,b);
		break;
	}
}

static void rt_eval_assign ( IL_node *tmp ,int mode)
{

	if ( tmp->exp->A.type==ELEMENT_VAR /*局部变量*/
			||tmp->exp->A.type==ELEMENT_ARRAY /*数组变量*/
			||(tmp->exp->A.type==ELEMENT_TMP && get_tmp(tmp->exp->A.index).address)) /*对象成员*/
	{
	}
	else
	{
		printf ( "only var can BE l-value!!%d \n ",tmp->exp->A.type );
		exit ( 0 );
	}
	Var	value_b;
	value_b=IL_rt_element_to_var ( tmp->exp->B,tmp );

	switch (tmp->exp->A.type)
	{
		/*为数组元素赋值*/
	case ELEMENT_ARRAY:
	{
        Var var_array=vm_rt_stack_var_get( tmp->exp->A.index);
        int array_type=var_GetType (var_array);
        Var l_value;
        if(array_type==VAR_TYPE_TUPLE)
        {
         l_value=tuple_GetValue(var_array,get_tmp(tmp->exp->A.array_index).content.int_value);
        }
        else if(array_type==VAR_TYPE_VECTOR )
        {
            l_value=vector_GetValue(var_array,get_tmp(tmp->exp->A.array_index).content.int_value);
        }
		int l_t=var_GetType(l_value);
		Var r_value =assign_get(IL_rt_element_to_array_ptr(tmp->exp->A,tmp),value_b,mode);
		int r_t=var_GetType(r_value);

        if(r_t==VAR_TYPE_HANDLE || r_t==VAR_TYPE_TUPLE ||r_t==VAR_TYPE_VECTOR)
		{
			RefCountIncrease(r_t,r_value.content.handle_value);
		}
        if(l_t==VAR_TYPE_HANDLE || r_t==VAR_TYPE_TUPLE ||r_t==VAR_TYPE_VECTOR)
		{
			RefCountDecrease(l_t,l_value.content.handle_value);
		}
        if(array_type==VAR_TYPE_TUPLE)
        {
         tuple_SetValue(var_array,get_tmp(tmp->exp->A.array_index).content.int_value,r_value);
        }
        else if(array_type==VAR_TYPE_VECTOR )
        {
            vector_SetValue (var_array,get_tmp(tmp->exp->A.array_index).content.int_value,r_value);
        }

	}
	break;
	/*为变量赋值*/
	case ELEMENT_VAR:
	{
		Var r_value=assign_get(vm_rt_stack_var_get(tmp->exp->A.index),value_b,mode);
		Var l_value=vm_rt_stack_var_get(tmp->exp->A.index);
		int l_t=var_GetType(l_value);
		int r_t=var_GetType(r_value);

        if(r_t==VAR_TYPE_HANDLE || r_t==VAR_TYPE_TUPLE || r_t==VAR_TYPE_VECTOR)
		{
			RefCountIncrease(r_t,r_value.content.handle_value);
		}
        if(l_t==VAR_TYPE_HANDLE || l_t==VAR_TYPE_TUPLE || l_t==VAR_TYPE_VECTOR)
		{
			RefCountDecrease(l_t,l_value.content.handle_value);
		}
		vm_rt_stack_var_set (tmp->exp->A.index ,r_value);
	}
	break;
	case ELEMENT_TMP:  /*为对象的成员赋值*/
	{
		Var* member=(Var *) (get_tmp(tmp->exp->A.index).address);
		Var l_value= (*member);
		Var r_value=assign_get((*member),value_b,mode);
		int l_t=var_GetType(l_value);
		int r_t=var_GetType(r_value);

        if(r_t==VAR_TYPE_HANDLE || r_t==VAR_TYPE_TUPLE ||r_t==VAR_TYPE_VECTOR )
		{
			RefCountIncrease(r_t,r_value.content.handle_value);
		}
        if(l_t==VAR_TYPE_HANDLE || l_t==VAR_TYPE_TUPLE ||l_t==VAR_TYPE_VECTOR )
		{
			RefCountDecrease(l_t,l_value.content.handle_value);
		}
		(*member)=r_value;
		Var  a= get_tmp(tmp->exp->A.index);
		/*用完之后,马上把临时变量表的地址给抹去,防止把其他变量当成对象*/
		a.address=NULL;
		set_tmp(tmp->exp->A.index,a);
	}
	break;
	}
	last_tmp_value=value_b;
	set_tmp(tmp->exp->tmp_index,last_tmp_value);

}


static void handle_exp ( IL_node *tmp )
{
	switch ( tmp->exp->op )
	{
	case '+':

		rt_eval_plus ( tmp );
		break;
	case '-':
		rt_eval_minus ( tmp );
		break;
	case '*':
		rt_eval_mutiple ( tmp );
		break;
	case '/':
		rt_eval_divide ( tmp );
		break;
	case '=':
		rt_eval_assign ( tmp,tmp->exp->op);
		break;
	case '<':
		rt_eval_less ( tmp );
		break;
	case '@':
		rt_eval_ref ( tmp );
		break;
	case '>':
		rt_eval_large ( tmp );
		break;

	case OP_EQUAL:
		rt_eval_equal ( tmp );
		break;
	case OP_NOT_EQUAL:
		rt_eval_not_equal ( tmp );
		break;
	case OP_LARGE_OR_EQUAL:
		rt_eval_large_or_equal ( tmp );
		break;
	case OP_LESS_OR_EQUAL:
		rt_eval_less_or_equal ( tmp );
		break;
	case OP_ASSIGN_PLUS:
		rt_eval_assign ( tmp,tmp->exp->op);
		break;
	case OP_ASSIGN_MINUS:
		rt_eval_assign ( tmp,tmp->exp->op);
		break;
	case OP_ASSIGN_MULTIPLE:
		rt_eval_assign ( tmp,tmp->exp->op);
		break;
	case OP_ASSIGN_DIVIDE:
		rt_eval_assign ( tmp,tmp->exp->op);
		break;
	case OP_AND:
		rt_eval_and ( tmp );
		break;
	case OP_OR:
		rt_eval_or ( tmp );
		break;
	case '.':
		rt_eval_point_to(tmp);
		break;
	}

}
/*执行打印节点*/
void ExecPrintNode(Var the_var)
{
	switch ( the_var.content.type )
	{
	case VAR_TYPE_INT:
		printf ( "%d\n",the_var.content.int_value );
		break;
	case VAR_TYPE_REAL:
		printf ( "%g\n",the_var.content.real_value );
		break;
	case VAR_TYPE_BOOL:
		if ( the_var.content.bool_value!=0 )
		{
			printf ( "true\n" );
		}
		else
		{
			printf ( "false\n" );
		}
		break;
    case VAR_TYPE_MESSAGE:
		printf ( "%s\n",the_var.content.str );
		break;
	case VAR_TYPE_REF:
		printf ( "ref to %d  type :ref \n",the_var.content.handle_value );
		break;
	case VAR_TYPE_HANDLE:
		printf ( "handle %d \n",the_var.content.handle_value);
		break;
	case VAR_TYPE_NILL:
		printf("NILL\n");
		break;
    case VAR_TYPE_CHAR:
        printf("%c\n",the_var.content.char_value);
        break;
    case VAR_TYPE_VECTOR:
        vector_Print(the_var);
        break;
    case VAR_TYPE_TUPLE:
        printf("tuple %d\n",the_var.content.handle_value);
        break;
	default:
		printf("print doesn't support,type: %d \n",var_GetType(the_var));
		break;
	}
}

/*执行中间代码,输入一张中间代码表*/
Var IL_exec ( Function *func )
{
	/*记录当前函数所需的变量数量,为计算其嵌套调用函数时,虚拟机变量的偏移量*/

	vm_SetLayerVarAmount(vm_GetCurrentLayer() ,func->var_counts);
	IL_node * tmp;
	tmp=func->list.head;
	current_list=&func->list;
	while ( tmp!=NULL )
	{
		switch ( tmp->type )
		{
		case IL_NODE_PRNT:
			ExecPrintNode(last_tmp_value);
			break;
		case IL_NODE_EXP:
			handle_exp ( tmp );
			break;
		case IL_NODE_RETURN:
			return last_tmp_value;
			break;
		case IL_NODE_JMP:
		{
			int label=tmp->jmp->label;
			IL_node * node = tmp->next;
			while ( 1 )
			{
				if ( (node->type==IL_NODE_LAB) && (node->jmp->label==label ) )
				{
					break;
				}
				node=node->next;
			}
			tmp =node;
		}
		break;
		case IL_NODE_JE:
		{
			int can_jmp=0;
			switch ( last_tmp_value.content.type )
			{
			case VAR_TYPE_INT:
				if ( last_tmp_value.content.int_value==0 )
					can_jmp=1;
				break;
			case VAR_TYPE_REAL:
				if ( last_tmp_value.content.real_value==0 )
					can_jmp=1;
				break;
			case VAR_TYPE_BOOL:
				if ( last_tmp_value.content.bool_value==0 )
					can_jmp=1;
				break;
			}

			if ( can_jmp==1 )
			{
				int label =tmp->jmp->label;
				IL_node * node= tmp->next;
				while ( 1 )
				{
					if ( (node->type==IL_NODE_LAB )&& (node->jmp->label==label)  )
					{
						break;
					}
					node=node->next;
				}
				tmp =node;
			}
		}
		break;
		case IL_NODE_JNE:
		{
			int can_jmp=0;
			switch ( last_tmp_value.content.type )
			{
			case VAR_TYPE_INT:
				if ( last_tmp_value.content.int_value!=0 )
					can_jmp=1;
				break;
			case VAR_TYPE_REAL:
				if ( last_tmp_value.content.real_value!=0 )
					can_jmp=1;
				break;
			case VAR_TYPE_BOOL:
				if ( last_tmp_value.content.bool_value!=0 )
					can_jmp=1;
				break;
			}
			if ( can_jmp==1 )
			{
				int label = tmp->jmp->label;
				IL_node * node=tmp->pre;
				while ( 1 )
				{
					if ( (node->type==IL_NODE_LAB) &&(node->jmp->label==label))
					{
						break;
					}
					node=node->pre;
				}
				tmp=node;
			}

		}
		break;
		}
		tmp=tmp->next;
	}

	{
		Var result;
		result.content.type=VAR_TYPE_INT;
		result.content.int_value=0;
		return result;
	}
}


/*函数的调用*/
Var IL_CallFunc (IL_element element, int f_index,int mode)
{

	Var result;
	if(mode==FUNC_NORMAL)/*普通的脚本函数调用*/
	{
		Function * f =func_get_by_index ( f_index);
		/*检查执行时刻的参数是否与所对应的函数的参数数目相符*/
		if(f->arg_counts!=element.value.content.func.args)
		{
			printf("error,%s  the args count is not match 形式：%d 实际：%d\n",f->name,f->arg_counts,element.value.content.func.args);
			exit(0);
		}
		int i=0;
		i=element.value.content.func.args;
		vm_RTstackPush();
		/*回溯添加参数,并把当前层的开头的相应的*/
		/*局部变量赋值为此*/
		for ( i=0;
				i<element.value.content.func.args;
				i++)
		{
			Var source=get_tmp(element.function_indexs[i]);
			int r_t=var_GetType(source);
			/*为函数的参数赋值*/
            if(r_t==VAR_TYPE_TUPLE  || r_t==VAR_TYPE_VECTOR|| r_t==VAR_TYPE_HANDLE)/*若右值为引用型引用计数自增*/
			{
				RefCountIncrease(r_t,source.content.handle_value);
			}
			vm_rt_stack_var_set ( i,source);
		}
		/*保存当前的表环境*/
		IL_list * old_list =current_list;
		result =func_invoke ( f_index);
		current_list=old_list;
		/*函数的栈弹出*/
		vm_RTstackPop();
	}
	else if(mode==FUNC_API)/*API函数调用情况*/
	{

		int i=0;
		for ( i=0; i<element.value.content.func.args; i++ )
		{
			API_argument_list[i]=get_tmp(element.function_indexs[i]);
		}
		API_SetArgCount(element.value.content.func.args);
		Var return_value=API_InovkeByIndex ( f_index);

		return return_value;
	}
	return result;
}
