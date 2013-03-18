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
#include "const_segment.h"

/*当前运行到节点*/
static IL_node * current_node=NULL;
/*当前的标签数*/
int label_index=0;
/*储存上一个被使用的临时节点的值*/
Var last_tmp_value;
/*函数的调用*/
Var IL_CallFunc (int args, int index, int mode);
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
/*创建一个中间语言表达式节点*/
IL_exp * IL_exp_create ( char op,IL_element var_a,IL_element var_b )
{
	static int exp_index=0;
	IL_exp * exp= ( IL_exp* ) malloc ( sizeof ( IL_exp ) );
	exp->op=op;
	exp->A=var_a;
	exp->B=var_b;
	exp_index++;
	exp->id=exp_index;
	return exp;
}


/*创建返回的节点*/
IL_node * IL_CreateReturnNode()
{
	IL_node * node = ( IL_node * ) malloc ( sizeof ( IL_node ) );
	node->retrn = ( IL_return* ) malloc ( sizeof ( IL_return ) );
	node->type=IL_NODE_RETURN;
	return node;
}

void IL_ListInsertReturn()
{
    IL_ListInsertNode ( IL_CreateReturnNode() );
}

/*创建一个节点，用与调用列表*/
IL_CallNode * IL_CreateCallListNode(int index,int args)
{
    IL_CallNode * node =malloc (sizeof (IL_CallNode));
    node->args=args;
    node->list_id=index;
    return node;
}



/*为列表中添加节点*/
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
IL_ListNode * IL_CreateListNode(int var_index)
{
    IL_ListNode * node=malloc (sizeof(IL_ListNode));
    node->var_index=var_index;
    return node;
}



/*向中间语言的执行序列中增加一个表达式*/
void IL_ListInsertEXP ( IL_exp *exp,int tmp_index )
{
    IL_node * node= IL_CreateNode(tmp_index);
    node->exp=exp;
    node->type=IL_NODE_EXP;
	if ( func_get_current()->list.head==NULL )
	{
        func_get_current()->list.head=node;
		func_get_current()->list.head->next=NULL;
	}
	else
	{
		IL_node *tmp=func_get_current()->list.head;
		while ( tmp->next!=NULL )
		{
			tmp=tmp->next;
		}
		tmp->next= node;
		node->pre=tmp;
		node->next=NULL;
	}
}


/*向节点中添加*/
IL_node * IL_CreateNode(int tmp_index)
{
    IL_node * node=malloc(sizeof(IL_node));
    node->tmp_index=tmp_index;
    node->next=NULL;
    node->pre=IL_NODE_NILL;
    return node;
}


IL_ListCreatorNode *  IL_CreateCreator(int init_args)
{
    IL_ListCreatorNode * node =malloc (sizeof(IL_ListCreatorNode));
    node->init_args=init_args;
    return node;
}
/*像中间代码添加列表构造器中间代码*/
void IL_ListInsertListCreator(int type,int tmp_index,int init_args)
{
    IL_node * node=IL_CreateNode (tmp_index);
     node->list_creator=IL_CreateCreator(init_args);
     switch(type)
     {
     case ELEMENT_VECTOR_CREATE:
         node->type=IL_NODE_VECTOR_CREATOR;
         break;
    case ELEMENT_TUPLE_CREATE:
         node->type=IL_NODE_TUPLE_CREATOR;
         break;
        default:
         STOP("IL_ListInsertListCreator error!");
         break;
     }
    if ( func_get_current()->list.head==NULL )
    {
        func_get_current()->list.head=node;
        func_get_current()->list.head->next=NULL;
    }
    else
    {
        IL_node *tmp=func_get_current()->list.head;
        while ( tmp->next!=NULL )
        {
            tmp=tmp->next;
        }
        tmp->next= node;
        node->pre=tmp;
        node->next=NULL;
    }
}

/*像中间代码添加带列表函数调用代码*/
void IL_ListInsertCall(int type,int tmp_index,int args,int function_index)
{
    IL_node * node=IL_CreateNode (tmp_index);
     node->call=IL_CreateCallListNode (function_index,args);
     switch(type)
     {
     case FUNC_NORMAL:
         node->type=IL_NODE_CALL_LIST;
         break;
    case FUNC_API:
         node->type=IL_NODE_CALL_API;
         break;
    case FUNC_DYNAMIC:
         node->type=IL_NODE_CALL_DYNAMIC;
         break;
        default:
         STOP("IL_ListInsertCall error!");
         break;
     }
    if ( func_get_current()->list.head==NULL )
    {
        func_get_current()->list.head=node;
        func_get_current()->list.head->next=NULL;
    }
    else
    {
        IL_node *tmp=func_get_current()->list.head;
        while ( tmp->next!=NULL )
        {
            tmp=tmp->next;
        }
        tmp->next= node;
        node->pre=tmp;
        node->next=NULL;
    }
}

/*打印表达式节点*/
void IL_PrintExp ( IL_node * tmp )
{
    printf("EXP:@%d",tmp->tmp_index);
	printf ( "=" );
	switch ( tmp->exp->A.type )
	{
	case ELEMENT_VAR:
        printf ( "$%d",tmp->exp->A.index );
		break;
    case ELEMENT_LITERAL:
        printf("#%d ",tmp->exp->A.index);
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
        printf("ARRAY <$%d,@%d>",tmp->exp->A.index,tmp->exp->A.info.array_index_tmp);
		break;
	}
	printf ( " %c ",tmp->exp->op );
	switch ( tmp->exp->B.type )
	{
	case ELEMENT_VAR:
        printf ( "$%d",tmp->exp->B.index );
		break;
    case ELEMENT_LITERAL:
        printf("#%d ",tmp->exp->B.index);
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
 printf("ARRAY <$%d,@%d>",tmp->exp->B.index,tmp->exp->B.info.array_index_tmp);
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

void IL_printf_call_list ( IL_node *node )
{
    printf ( "LIST:@%d=[id:%d,args:%d]\n",node->tmp_index,node->call->list_id,node->call->args );
}

void IL_printf_call_API ( IL_node *node )
{
     printf ( "API:@%d=[id:%d,args:%d]\n",node->tmp_index,node->call->list_id,node->call->args );
}

void IL_printf_call_dynamic ( IL_node *node )
{
     printf ( "DYNAMIC:@%d=[args:%d]\n",node->tmp_index,node->call->args );
}

void IL_printf_vector_creator(IL_node *node)
{
    printf("VECTOR:@%d=[args:%d]\n",node->tmp_index,node->list_creator->init_args);
}
void IL_printf_tuple_creator(IL_node *node)
{
    printf("TUPLE:@%d=[args:%d]\n",node->tmp_index,node->list_creator->init_args);
}
/*打印中间代码执行序列*/
void IL_list_print ( Function * func )
{
    printf("IL BEGIN :\n");
	IL_node * tmp=func->list.head;
	while ( tmp!=NULL )
	{
		switch ( tmp->type )
		{
		case IL_NODE_EXP:
            IL_PrintExp ( tmp );
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
        case IL_NODE_CALL_API:
            IL_printf_call_API(tmp);
            break;
        case IL_NODE_CALL_LIST:
            IL_printf_call_list(tmp);
            break;
        case IL_NODE_CALL_DYNAMIC:
            IL_printf_call_dynamic(tmp);
            break;
		case IL_NODE_RETURN:
			printf ( "return node \n" );
			break;
        case IL_NODE_VECTOR_CREATOR:
            IL_printf_vector_creator (tmp);
            break;
        case IL_NODE_TUPLE_CREATOR:
            IL_printf_tuple_creator (tmp);
            break;
		}
		tmp=tmp->next;
	}
    printf("IL  END:\n");
}

/*根据函数名称打印中间代码*/
void Tina_PrintIL ( const char * func_name )
{
	IL_list_print ( func_get_by_index ( func_get_index_by_name ( func_name ) ) );

}

/*创建表达式型节点*/
IL_node * IL_node_create_exp ( IL_exp * exp,int tmp_index )
{
	IL_node * node= ( IL_node* ) malloc ( sizeof ( IL_node ) );
    node->exp=exp;
	node->next=NULL;
    node->tmp_index=tmp_index;
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



/*求值器，用于将中间代码节点中的内容，求解成真正的变量*/
static Var IL_rt_Evaluate ( IL_element element )
{
	Var result ;
	switch ( element.type )
	{
	case ELEMENT_VAR:
		result= vm_rt_stack_var_get ( element.index );
		break;

    case ELEMENT_LITERAL:/*字面量*/
    {
        /*从常量段寻找*/
        Var src=ConstSegmentGetVar (element.index);
        result=src ;
        if(var_GetType (src)==VAR_TYPE_VECTOR)/*如果是向量字面量，则拷贝一个副本*/
        {
            result=vector_Clone (src);
        }
    }
		break;
	case ELEMENT_TMP:
		result= get_tmp(element.index);
		break;
		/*自指向*/
	case ELEMENT_SELF:
        result=self_ptr;
        break;
    case ELEMENT_ARRAY:
    {
        Var a=vm_rt_stack_var_get ( element.index );
        if ( a.content.type!=VAR_TYPE_TUPLE && a.content.type!=VAR_TYPE_VECTOR )
        {
            STOP( "error !!  only list can use '['  ']' to access!\n ");
        }
        else
        {
            int list_index=var_GetInt (get_tmp (element.info.array_index_tmp));
            /*获得下标*/
            if(a.content.type==VAR_TYPE_TUPLE)
            {
                result=tuple_GetValue(a,list_index);
            }
            else
            {
                result=vector_GetValue (a,list_index);
            }
		}
	}
	break;
	default :
		printf ( "can't resolve this element type : %d\n",element.type );
		exit ( 0 );
		return result;
	}
	return result;
}



/*在运行时解析节点的加法运算*/
static void rt_eval_plus ( IL_node *tmp )
{
	Var value_a,value_b;
	/*根据栈中的函数调用情况获取栈中绝对定位下的变量*/
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
	last_tmp_value=var_add ( value_a,value_b );
    set_tmp(tmp->tmp_index,last_tmp_value);
}

/*在运行时解析对象成员指向运算*/
static void rt_eval_point_to(IL_node *tmp)
{
    if(tmp->exp->A.type!=ELEMENT_VAR && tmp->exp->A.type!=ELEMENT_API && tmp->exp->A.type!=ELEMENT_SELF
            &&  tmp->exp->A.type!=ELEMENT_TMP
            &&tmp->exp->A.type!=ELEMENT_LITERAL)
	{
        printf("doesn't support this kind of point operate,A : index %d    type %d \n",tmp->tmp_index,tmp->exp->A.type);
		exit(0);
	}
	Var value_a,value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
	if(value_a.content.type==VAR_TYPE_STRUCT_NAME) /*类成员*/
	{
		last_tmp_value=var_point_to ( value_a,value_b );
        last_tmp_value.address=GetPrototypeMemberAddress(value_a.content.var_value.str,var_GetMsg(value_b));
		/*类成员，self指针失效，同时环境索引变为全局*/
		self_ptr.class_id=env_index;
		self_ptr.content.type=VAR_TYPE_NILL;
	}
	else /*对象成员*/
	{

		/*判断访问性*/
        int acc=get_accessbility_of_member(value_a.class_id,get_index_of_member(value_a.class_id,var_GetMsg(value_b)));
		if(acc!=STRUCT_PUBLIC && env_index != value_a.class_id)/*非公有且在外部*/
		{
			printf("private or sealed member cannot be assinged outside\n");
			exit(0);
		}
		last_tmp_value=var_point_to ( value_a,value_b );
		/*设置self指针*/
		self_ptr= value_a;
		/*指向成员的运算的结果为其成员的地址*/
        last_tmp_value.address=GetObjectMemberAddress(var_getHandle (value_a),get_index_of_member(value_a.class_id,var_GetMsg (value_b)));
	}
    set_tmp(tmp->tmp_index,last_tmp_value);
}

/*在运行时解析节点的乘法运算*/
static void rt_eval_mutiple ( IL_node *tmp )
{
	Var value_a,value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
	last_tmp_value=var_multiple ( value_a,value_b );
    set_tmp(tmp->tmp_index,last_tmp_value);
}



/*在运行时解析节点的减法运算*/
static void rt_eval_minus ( IL_node *tmp )
{
	Var value_a,value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
	last_tmp_value=var_minus ( value_a,value_b );
    set_tmp(tmp->tmp_index,last_tmp_value);

}
/*在运行时解析节点的除法运算*/
static void rt_eval_divide ( IL_node *tmp )
{
	Var value_a,value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
	last_tmp_value=var_divide ( value_a,value_b );
    set_tmp(tmp->tmp_index,last_tmp_value);
}



static void rt_eval_large ( IL_node *tmp )
{
	Var value_a,value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
	last_tmp_value=var_large ( value_a,value_b );
    set_tmp(tmp->tmp_index,last_tmp_value);
}


static void rt_eval_less ( IL_node *tmp )
{
	Var value_a,value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
	last_tmp_value=var_less ( value_a,value_b );
    set_tmp(tmp->tmp_index,last_tmp_value);
}


static void rt_eval_less_or_equal ( IL_node *tmp )
{
	Var value_a,value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
	last_tmp_value=var_less_or_equal ( value_a,value_b );
    set_tmp(tmp->tmp_index,last_tmp_value);
}



static void rt_eval_large_or_equal ( IL_node *tmp )
{
	Var value_a,value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B);
	last_tmp_value=var_large_or_qual ( value_a,value_b );
    set_tmp(tmp->tmp_index,last_tmp_value);
}



static void rt_eval_equal ( IL_node *tmp )
{
	Var value_a,value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
	last_tmp_value=var_equal ( value_a,value_b );
    set_tmp(tmp->tmp_index,last_tmp_value);
}



static void rt_eval_not_equal ( IL_node *tmp )
{
	Var value_a,value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
	last_tmp_value=var_not_equal ( value_a,value_b );
    set_tmp(tmp->tmp_index,last_tmp_value);
}


static void rt_eval_and ( IL_node *tmp )
{
	Var value_a,value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
	last_tmp_value=var_and ( value_a,value_b );
    set_tmp(tmp->tmp_index,last_tmp_value);
}


static void rt_eval_or ( IL_node *tmp )
{
	Var value_a,value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
	last_tmp_value=var_or ( value_a,value_b );
    set_tmp(tmp->tmp_index,last_tmp_value);
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
/*获得表中成员*/
static Var GetListElement(Var obj,int index)
{
    if(var_GetType (obj)==VAR_TYPE_VECTOR)
    {
       return vector_GetValue (obj,index);
    }else
        if(var_GetType (obj)==VAR_TYPE_VECTOR)
        {
            return tuple_GetValue (obj,index);
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
    value_b=IL_rt_Evaluate ( tmp->exp->B );

	switch (tmp->exp->A.type)
	{
		/*为数组元素赋值*/
	case ELEMENT_ARRAY:
    {
        Var var_array=vm_rt_stack_var_get( tmp->exp->A.index);
        int list_index= var_GetInt (get_tmp(tmp->exp->A.info.array_index_tmp));
        int array_type=var_GetType (var_array);
        int l_t=var_GetType(var_array);
        /*进行赋值运算*/
        Var r_value =assign_get(GetListElement(var_array,list_index),value_b,mode);
		int r_t=var_GetType(r_value);
        if(r_t==VAR_TYPE_HANDLE || r_t==VAR_TYPE_TUPLE ||r_t==VAR_TYPE_VECTOR)
		{
            RefCountIncrease(r_t,r_value.content.var_value.handle_value);
		}
        if(l_t==VAR_TYPE_HANDLE || r_t==VAR_TYPE_TUPLE ||r_t==VAR_TYPE_VECTOR)
		{
            RefCountDecrease(l_t,var_array.content.var_value.handle_value);
		}
        if(array_type==VAR_TYPE_TUPLE)
        {
         tuple_SetValue(var_array,list_index,r_value);
        }
        else if(array_type==VAR_TYPE_VECTOR )
        {
        vector_SetValue(var_array,list_index,r_value);
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
            RefCountIncrease(r_t,var_getHandle (r_value));
		}
        if(l_t==VAR_TYPE_HANDLE || l_t==VAR_TYPE_TUPLE || l_t==VAR_TYPE_VECTOR)
		{
            RefCountDecrease(l_t,var_getHandle (l_value));
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
            RefCountIncrease(r_t,var_getHandle (r_value));
		}
        if(l_t==VAR_TYPE_HANDLE || l_t==VAR_TYPE_TUPLE ||l_t==VAR_TYPE_VECTOR )
		{
            RefCountDecrease(l_t,var_getHandle (l_value));
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
    set_tmp(tmp->tmp_index,last_tmp_value);

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
        printf ( "%d\n",var_GetInt (the_var));
		break;
	case VAR_TYPE_REAL:
        printf ( "%g\n",var_GetDouble (the_var));
		break;
	case VAR_TYPE_BOOL:
        if ( var_GetBool (the_var)!=0 )
		{
			printf ( "true\n" );
		}
		else
		{
			printf ( "false\n" );
		}
		break;
    case VAR_TYPE_MESSAGE:
        printf ( "%s\n",var_GetMsg (the_var));
		break;
	case VAR_TYPE_REF:
        printf ( "ref to %d  type :ref \n",var_getHandle (the_var));
		break;
	case VAR_TYPE_HANDLE:
        printf ( "handle %d \n",var_getHandle (the_var));
		break;
	case VAR_TYPE_NILL:
		printf("NILL\n");
		break;
    case VAR_TYPE_CHAR:
        printf("%c\n",var_GetChar (the_var));
        break;
    case VAR_TYPE_VECTOR:
        vector_Print(the_var);
        break;
    case VAR_TYPE_TUPLE:
        printf("tuple %d\n",var_getHandle (the_var));
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
        current_node=tmp;
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
        case IL_NODE_NILL:
            break;
        case IL_NODE_VECTOR_CREATOR:
        {
            IL_node * i=tmp->pre;
            int c=tmp->list_creator->init_args;
            Var init_var[c];
            for(;c>0;c--)
            {
                init_var[c-1]=get_tmp (i->tmp_index);
                i=i->pre;
            }
        last_tmp_value= the_vector_creator(tmp->list_creator->init_args,init_var);
        set_tmp (tmp->tmp_index,last_tmp_value);
        }
            break;
        case IL_NODE_TUPLE_CREATOR:

        {
            IL_node * i=tmp->pre;
            int c=tmp->list_creator->init_args;
            Var init_var[c];
            for(;c>0;c--)
            {
                init_var[c-1]=get_tmp (i->tmp_index);
                i=i->pre;
            }
        last_tmp_value= the_tuple_creator (tmp->list_creator->init_args,init_var);
        set_tmp (tmp->tmp_index,last_tmp_value);
        }
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
            if(var_GetType (last_tmp_value)==VAR_TYPE_BOOL)
            {
                if(var_GetBool (last_tmp_value)==0)
                {
                    can_jmp=1;
                }
            }
            else
            {
                STOP(" illigal je");
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
        case IL_NODE_CALL_LIST:
        {
            last_tmp_value =IL_CallFunc (tmp->call->args,tmp->call->list_id,FUNC_NORMAL);
            set_tmp (tmp->tmp_index,last_tmp_value);
        }
            break;
        case IL_NODE_CALL_API:
        {
            last_tmp_value=IL_CallFunc (tmp->call->args,tmp->call->list_id,FUNC_API);
            set_tmp (tmp->tmp_index,last_tmp_value);
        }
            break;
        case IL_NODE_CALL_DYNAMIC:
        {
            last_tmp_value =IL_CallFunc (tmp->call->args,tmp->call->list_id,FUNC_DYNAMIC);
            set_tmp (tmp->tmp_index,last_tmp_value);
        }
            break;
        case IL_NODE_JNE:
        {
            int can_jmp=0;
            if(var_GetType (last_tmp_value)==VAR_TYPE_BOOL)
            {
                if(var_GetBool (last_tmp_value)!=0)
                {
                    can_jmp=1;
                }
            }
            else
            {
                STOP(" illigal jne");
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
        var_SetInt (&result,0);
		return result;
	}
}


/*函数的调用*/
Var IL_CallFunc (int args, int f_index,int mode)
{

	Var result;
    Var arg_list[args];


    /*回溯添加参数,并把当前层的开头的相应的*/
    /*局部变量赋值为此*/
    {
        IL_node * tmp=current_node->pre;
        int i=0;
        for ( i=0;
                i<args;
                i++)
        {
            Var source=get_tmp(tmp->tmp_index);
            tmp=tmp->pre;
            arg_list[args-i-1]=source;
        }
        if(mode==FUNC_DYNAMIC)/*说延迟绑定函数索引，这是需要在开头寻找*/
        {
            f_index=get_tmp(tmp->tmp_index).content.var_value.func.func_index;
            mode=get_tmp(tmp->tmp_index).content.var_value.func.func_type;
        }

    }


	if(mode==FUNC_NORMAL)/*普通的脚本函数调用*/
	{
		Function * f =func_get_by_index ( f_index);
		/*检查执行时刻的参数是否与所对应的函数的参数数目相符*/
        if(f->arg_counts!=args)
		{
            printf("error,%s  the args count is not match 形式：%d 实际：%d\n",f->name,f->arg_counts,args);
			exit(0);
		}
		int i=0;
		vm_RTstackPush();
		for ( i=0;
                i<args;
				i++)
		{
            Var source=arg_list[i];
			int r_t=var_GetType(source);
			/*为函数的参数赋值*/
            if(r_t==VAR_TYPE_TUPLE  || r_t==VAR_TYPE_VECTOR|| r_t==VAR_TYPE_HANDLE)/*若右值为引用型引用计数自增*/
			{
                RefCountIncrease(r_t,var_getHandle (source));
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
        for ( i=0; i<args; i++ )
		{
            API_argument_list[i]=arg_list[i];
		}
        API_SetArgCount(args);
		Var return_value=API_InovkeByIndex ( f_index);
		return return_value;
	}
	return result;
}
