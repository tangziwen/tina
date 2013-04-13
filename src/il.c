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
#include "const_segment.h"
#include "script_struct.h"
#include "build.h"
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
static void set_tmp(int index,Var * var,int type)
{
    if(current_list->tmp_var_list[index].tmp_type==TMP_ARITH)/*如果前一个是算术类型的，那么释放它的内存*/
    {
        free(current_list->tmp_var_list[index].result);
    }


    if(type==TMP_ARITH)/*如果当前的结果是算术类型，把值给拷下来*/
    {
        current_list->tmp_var_list[index].result=malloc(sizeof(Var));
        memcpy (current_list->tmp_var_list[index].result,var,sizeof(Var));
    }
    else if(type==TMP_DEREFER)/*解引用运算，直接赋值地址*/
    {
        current_list->tmp_var_list[index].result=var;
    }
    //保存当前临时结果类型
    current_list->tmp_var_list[index].tmp_type=type;
}
static Var * get_tmp(int index)
{
    return  current_list->tmp_var_list[index].result;
}


/*获得表中成员*/
static Var  * GetListElement(Var obj,int index)
{
        if(var_GetType (obj)==VAR_TYPE_TUPLE)
        {
            return tuple_GetValue (obj,index);
        }
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


IL_ListCreatorNode *  IL_CreateListCreator(int init_args)
{
    IL_ListCreatorNode * node =malloc (sizeof(IL_ListCreatorNode));
    node->init_args=init_args;
    return node;
}
IL_StructCreatorNode *  IL_CreateStructCreator(int id,int init_args)
{
    IL_StructCreatorNode * node =malloc (sizeof(IL_StructCreatorNode));
    node->id=id;
    node->init_args=init_args;
    return node;
}

/*往中间代码中插入求势节点*/
void IL_ListInsertCard(int tmp_index)
{
    IL_node * node=IL_CreateNode (tmp_index);
    node->type=IL_NODE_CARD;
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

/*往中间代码中插入求类型节点*/
void IL_ListInsertTypeof(int tmp_index)
{
    IL_node * node=IL_CreateNode (tmp_index);
    node->type=IL_NODE_TYPE_OF;
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

/*像中间代码添加列表构造器中间代码*/
IL_node * IL_ListInsertStructCreator(int id,int tmp_index,int init_args)
{
    IL_node * node=IL_CreateNode (tmp_index);
    node->struct_creator=IL_CreateStructCreator(id,init_args);
    node->type=IL_NODE_STRUCT_CREATOR;
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
    return node;
}




/*像中间代码添加列表构造器中间代码*/
void IL_ListInsertListCreator(int type,int tmp_index,int init_args)
{
    if(type!=ELEMENT_TUPLE_CREATOR)
    {
         STOP("IL_ListInsertListCreator error!");
    }
    IL_node * node=IL_CreateNode (tmp_index);
     node->list_creator=IL_CreateListCreator(init_args);
     node->type=IL_NODE_TUPLE_CREATOR;
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
    case FUNC_METHOD:
         node->type=IL_NODE_CALL_METHOD;
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
void IL_PrintExp (FILE *f, IL_node * tmp )
{
    fprintf(f,"E %d ",tmp->tmp_index);
	switch ( tmp->exp->A.type )
	{
	case ELEMENT_VAR:
        fprintf (f, "$%d",tmp->exp->A.index );
		break;
    case ELEMENT_LITERAL:
        fprintf(f,"#%d ",tmp->exp->A.index);
		break;
	case ELEMENT_TMP:
        fprintf (f,"@%d",tmp->exp->A.index );
		break;
	case ELEMENT_API:
        fprintf (f,"API %d",tmp->exp->A.index );
		break;
	case ELEMENT_FUNC:
        fprintf (f,"FUNC %d",tmp->exp->A.index );
		break;
	case ELEMENT_CALL_BY_MEMBER:
        fprintf (f,"CALL MEMBER %d",tmp->exp->A.index );
		break;
	}
    fprintf (f," %d ",tmp->exp->op );
	switch ( tmp->exp->B.type )
	{
	case ELEMENT_VAR:
        fprintf (f,"$%d",tmp->exp->B.index );
		break;
    case ELEMENT_LITERAL:
        fprintf(f,"#%d ",tmp->exp->B.index);
        break;
	case ELEMENT_TMP:
        fprintf (f,"@%d",tmp->exp->B.index );
		break;
	case ELEMENT_API:
        fprintf (f,"API %d",tmp->exp->B.index );
		break;
	case ELEMENT_FUNC:
        fprintf (f,"FUNC %d",tmp->exp->B.index );
		break;
	}
    fprintf (f,"\n" );
}



void IL_PrintGoto (FILE *f,IL_node *node )
{
    fprintf (f,"G %d\n",node->jmp->label );
}
void IL_PrintJe (FILE *f,IL_node *node )
{
    fprintf (f,"JE %d\n",node->jmp->label );
}
void IL_PrintJne (FILE *f,IL_node *node )
{
    fprintf (f,"JNE %d\n",node->jmp->label );
}
void IL_PrintLab (FILE *f,IL_node *node )
{
    fprintf (f,"L %d \n",node->jmp->label );
}

void IL_PrintPrnt (FILE *f,IL_node *node )
{
    fprintf (f,"P \n" );
}

void IL_PrintCallList (FILE *f,IL_node *node )
{
    fprintf (f,"i %d %d %d\n",node->tmp_index,node->call->list_id,node->call->args );
}

void IL_PrintCallAPI (FILE *f,IL_node *node )
{
     fprintf (f,"CALLA %d %d %d\n",node->tmp_index,node->call->list_id,node->call->args );
}

void IL_PrintCallDynamic (FILE *f,IL_node *node )
{
     fprintf (f,"D %d %d %d\n",node->tmp_index,node->call->list_id,node->call->args );
}

void IL_PrintVecCreator(FILE *f,IL_node *node )
{
    fprintf(f,"V %d %d\n",node->tmp_index,node->list_creator->init_args);
}
void IL_PrintTupleCreator(FILE *f,IL_node *node )
{
    fprintf(f,"T %d %d\n",node->tmp_index,node->list_creator->init_args);
}
void IL_PrintStructCreator(FILE *f,IL_node *node )
{
 fprintf(f,"c %d %d %d\n",node->tmp_index,node->struct_creator->id,node->struct_creator->init_args);
}
void IL_PrintCallMethod(FILE *f,IL_node *node )
{
 fprintf(f,"M %d %d\n",node->tmp_index, node->call->args);
}

void IL_PrintCard(FILE *f,IL_node *node )
{
 fprintf(f,"CA %d\n",node->tmp_index);
}

void IL_PrintTypeOf(FILE *f,IL_node *node)
{
 fprintf(f,"t %d\n",node->tmp_index);
}

/*打印中间代码执行序列*/
void IL_list_print (FILE *f, Function * func )
{
	IL_node * tmp=func->list.head;
	while ( tmp!=NULL )
	{
		switch ( tmp->type )
		{
		case IL_NODE_EXP:
            IL_PrintExp (f, tmp );
			break;
		case IL_NODE_JE:
            IL_PrintJe (f, tmp );
			break;
        case IL_NODE_GOTO:
            IL_PrintGoto (f, tmp );
			break;
		case IL_NODE_JNE:
            IL_PrintJne (f, tmp );
			break;
		case IL_NODE_LAB:
            IL_PrintLab (f, tmp );
			break;
		case IL_NODE_PRNT:
            IL_PrintPrnt (f, tmp );
			break;
        case IL_NODE_CALL_API:
            IL_PrintCallAPI(f,tmp);
            break;
        case IL_NODE_CALL_LIST:
            IL_PrintCallList(f,tmp);
            break;
        case IL_NODE_CALL_DYNAMIC:
            IL_PrintCallDynamic(f,tmp);
            break;
		case IL_NODE_RETURN:
            fprintf ( f,"R\n" );
			break;
        case IL_NODE_VECTOR_CREATOR:
            IL_PrintVecCreator (f,tmp);
            break;
        case IL_NODE_TUPLE_CREATOR:
            IL_PrintTupleCreator (f,tmp);
            break;
        case IL_NODE_STRUCT_CREATOR:
            IL_PrintStructCreator(f,tmp);
            break;
        case IL_NODE_CALL_METHOD:
            IL_PrintCallMethod(f,tmp);
            break;
        case IL_NODE_CARD:
            IL_PrintCard(f,tmp);
            break;
        case IL_NODE_TYPE_OF:
            IL_PrintTypeOf(f,tmp);
            break;
		}
		tmp=tmp->next;
	}
}

/*根据函数名称打印中间代码*/
void Tina_PrintIL (FILE* f, const char * func_name )
{
    IL_list_print (f, func_get_by_index ( func_GetIndexByName ( func_name ) ) );

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


IL_node * IL_node_create_goto ( int label ,int mode )
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



/*统一求值器，输入原始用于将中间代码节点中的内容，求解成真正的变量*/
static Var * IL_rt_Evaluate ( IL_element element )
{
    Var * result ;
    switch ( element.type )
    {
    case ELEMENT_VAR:
        result= vm_rt_stack_var_get_ptr( element.index );
        break;

    case ELEMENT_LITERAL:/*字面量*/
    {
        /*从常量段寻找*/
        Var* src=ConstSegmentGetVar (element.index);
        result=src ;
        if(var_GetType (*src)==VAR_TYPE_STR)/*如果是字符串字面量，则拷贝一个副本*/
        {
          (*result)=  tuple_CreateByString (src->content.var_value.str);
        }
    }
        break;
    case ELEMENT_TMP:
        result= get_tmp(element.index);
        break;
        /*自指向*/
    case ELEMENT_SELF:
        result=&self_ptr;
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
    Var  *value_a, * value_b;
	/*根据栈中的函数调用情况获取栈中绝对定位下的变量*/
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
    last_tmp_value=var_add ( *value_a,*value_b );
    set_tmp(tmp->tmp_index,&last_tmp_value,TMP_ARITH);
}

/*在运行时解析下标运算*/
static void rt_eval_subscript(IL_node *tmp)
{
    Var *value_a, *value_b,*result;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
    result=GetListElement (*value_a,var_GetInt (*value_b));
    set_tmp(tmp->tmp_index,result,TMP_DEREFER);
    last_tmp_value=*result;
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
    Var *value_a, *value_b,*result;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
    if(value_a->content.type==VAR_TYPE_STRUCT_NAME) /*类成员*/
	{
        result =var_point_to (* value_a,*value_b);
		/*类成员，self指针失效，同时环境索引变为全局*/
		self_ptr.class_id=env_index;
		self_ptr.content.type=VAR_TYPE_NILL;
	}
	else /*对象成员*/
	{

		/*判断访问性*/
        int acc=get_accessbility_of_member(value_a->class_id,get_index_of_member(value_a->class_id,var_GetMsg(*value_b)));
        if(acc!=STRUCT_PUBLIC && env_index != value_a->class_id)/*非公有且在外部*/
		{
			printf("private or sealed member cannot be assinged outside\n");
			exit(0);
		}
        result=var_point_to ( *value_a,*value_b );
		/*设置self指针*/
        self_ptr=* value_a;
	}
    set_tmp(tmp->tmp_index,result,TMP_DEREFER);
    last_tmp_value=(*result);
}

/*在运行时解析节点的乘法运算*/
static void rt_eval_mutiple ( IL_node *tmp )
{
    Var * value_a, *value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
    last_tmp_value=var_multiple ( *value_a,*value_b );
    set_tmp(tmp->tmp_index,&last_tmp_value,TMP_ARITH);
}



/*在运行时解析节点的减法运算*/
static void rt_eval_minus ( IL_node *tmp )
{
    Var *value_a, *value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
    last_tmp_value=var_minus ( *value_a,*value_b );
    set_tmp(tmp->tmp_index,&last_tmp_value,TMP_ARITH);

}
/*在运行时解析节点的除法运算*/
static void rt_eval_divide ( IL_node *tmp )
{
    Var * value_a, *value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
    last_tmp_value=var_divide ( *value_a,*value_b );
    set_tmp(tmp->tmp_index,&last_tmp_value,TMP_ARITH);
}



static void rt_eval_large ( IL_node *tmp )
{
    Var * value_a,*value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
    last_tmp_value=var_large ( *value_a, *value_b );
    set_tmp(tmp->tmp_index,&last_tmp_value,TMP_ARITH);
}


static void rt_eval_less ( IL_node *tmp )
{
    Var *value_a,*value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
    last_tmp_value=var_less (*value_a,*value_b );
    set_tmp(tmp->tmp_index,&last_tmp_value,TMP_ARITH);
}


static void rt_eval_less_or_equal ( IL_node *tmp )
{
    Var *value_a,*value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
    last_tmp_value=var_less_or_equal ( *value_a,*value_b );
    set_tmp(tmp->tmp_index,&last_tmp_value,TMP_DEREFER);
}



static void rt_eval_large_or_equal ( IL_node *tmp )
{
    Var * value_a,*value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B);
    last_tmp_value=var_large_or_qual (*value_a,*value_b );
    set_tmp(tmp->tmp_index,&last_tmp_value,TMP_ARITH);
}



static void rt_eval_equal ( IL_node *tmp )
{
    Var  *value_a, *value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
    last_tmp_value=var_equal ( *value_a,*value_b );
    set_tmp(tmp->tmp_index,&last_tmp_value,TMP_ARITH);
}



static void rt_eval_not_equal ( IL_node *tmp )
{
    Var *value_a, *value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
    last_tmp_value=var_not_equal ( *value_a,*value_b );
    set_tmp(tmp->tmp_index,&last_tmp_value,TMP_ARITH);
}


static void rt_eval_and ( IL_node *tmp )
{
    Var *value_a,*value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
    last_tmp_value=var_and ( *value_a,*value_b );
    set_tmp(tmp->tmp_index,&last_tmp_value,TMP_ARITH);
}


static void rt_eval_or ( IL_node *tmp )
{
    Var *value_a,*value_b;
    value_a=IL_rt_Evaluate ( tmp->exp->A );
    value_b=IL_rt_Evaluate ( tmp->exp->B );
    last_tmp_value=var_or ( *value_a, *value_b );
    set_tmp(tmp->tmp_index,&last_tmp_value,TMP_ARITH);
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
            ||(tmp->exp->A.type==ELEMENT_TMP)) /*对象成员*/
	{
	}
	else
	{
		printf ( "only var can BE l-value!!%d \n ",tmp->exp->A.type );
		exit ( 0 );
	}
    Var * l_value=IL_rt_Evaluate (tmp->exp->A);
    Var * r_value=IL_rt_Evaluate (tmp->exp->B);
    int l_t=var_GetType(*l_value);
    int r_t=var_GetType(*r_value);

    (*l_value)= assign_get(*l_value,*r_value,mode);
    set_tmp(tmp->tmp_index,l_value,TMP_ARITH);
    last_tmp_value=(*l_value);
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
    case OP_SUBSCRIPT:
        rt_eval_subscript(tmp);
        break;
	}

}
/*执行打印节点*/
void ExecPrintNode(Var the_var)
{
	switch ( the_var.content.type )
	{
	case VAR_TYPE_INT:
        printf ( "%d",var_GetInt (the_var));
		break;
	case VAR_TYPE_REAL:
        printf ( "%g",var_GetDouble (the_var));
		break;
	case VAR_TYPE_BOOL:
        if ( var_GetBool (the_var)!=0 )
		{
            printf ( "true" );
		}
		else
		{
            printf ( "false" );
		}
		break;
    case VAR_TYPE_MESSAGE:
        printf ( "%s",var_GetMsg (the_var));
		break;
    case VAR_TYPE_OBJ:
        printf ( "handle %d ",var_getHandle (the_var));
		break;
	case VAR_TYPE_NILL:
        printf("NILL");
		break;
    case VAR_TYPE_CHAR:
        printf("%c",var_GetChar (the_var));
        break;
    case VAR_TYPE_TUPLE:
        /*打印一个元组的所有成员*/
        tuple_print(the_var);
        break;
	default:
        printf("print doesn't support,type: %d",var_GetType(the_var));
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

        case IL_NODE_STRUCT_CREATOR:
        {
            IL_node * i=tmp->pre;
            int c=tmp->struct_creator->init_args;
            Var init_var[c];
            for(;c>0;c--)
            {
                init_var[c-1]= (*get_tmp (i->tmp_index));
                i=i->pre;
            }
            last_tmp_value =struct_Create(tmp->struct_creator->id,init_var,tmp->struct_creator->init_args);
            set_tmp (tmp->tmp_index,&last_tmp_value,TMP_DEREFER);
        }
            break;
        case IL_NODE_TUPLE_CREATOR:

        {
            IL_node * i=tmp->pre;
            int c=tmp->list_creator->init_args;
            Var init_var[c];
            for(;c>0;c--)
            {
                init_var[c-1]= (*get_tmp (i->tmp_index));
                i=i->pre;
            }
        last_tmp_value= the_tuple_creator (tmp->list_creator->init_args,init_var);
        set_tmp (tmp->tmp_index,&last_tmp_value,TMP_DEREFER);
        }
            break;
        case IL_NODE_GOTO:
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
                STOP(" illegal je");
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
            set_tmp (tmp->tmp_index,&last_tmp_value,TMP_ARITH);
        }
            break;
        case IL_NODE_CALL_API:
        {
            last_tmp_value=IL_CallFunc (tmp->call->args,tmp->call->list_id,FUNC_API);
            set_tmp (tmp->tmp_index,&last_tmp_value,TMP_ARITH);
        }
            break;
        case IL_NODE_CALL_DYNAMIC:
        {
            last_tmp_value =IL_CallFunc (tmp->call->args,tmp->call->list_id,FUNC_DYNAMIC);
            set_tmp (tmp->tmp_index,&last_tmp_value,TMP_ARITH);
        }
            break;
        case IL_NODE_CALL_METHOD:
        {
            last_tmp_value =IL_CallFunc (tmp->call->args,tmp->call->list_id,FUNC_METHOD);
            set_tmp (tmp->tmp_index,&last_tmp_value,TMP_ARITH);
        }
            break;
        case IL_NODE_CARD:
        {
            int card=tuple_GetCard(last_tmp_value);
            var_SetInt (&last_tmp_value,card);
            set_tmp (tmp->tmp_index,&last_tmp_value,TMP_ARITH);
        }
            break;
        case IL_NODE_TYPE_OF:
        {
            int type_of_var=var_GetType (last_tmp_value);
            var_SetInt (&last_tmp_value,type_of_var);
            set_tmp (tmp->tmp_index,&last_tmp_value,TMP_ARITH);
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
                STOP("illegal jne");
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
            Var source= (*get_tmp(tmp->tmp_index));
            tmp=tmp->pre;
            arg_list[args-i-1]=source;
        }
        if(mode==FUNC_DYNAMIC)/*说延迟绑定函数索引，这是需要在开头寻找*/
        {
            f_index=vm_RTstackVarGetAbs (f_index)->content.var_value.func.func_index;
            mode=vm_RTstackVarGetAbs(f_index)->content.var_value.func.func_type;
        }
        if(mode==FUNC_METHOD)
        {
            f_index=get_tmp(tmp->tmp_index)->content.var_value.func.func_index;
            mode=get_tmp(tmp->tmp_index)->content.var_value.func.func_type;
        }
    }


    if(mode==FUNC_NORMAL)/*普通的脚本函数调用*/
    {
        Function * f =func_get_by_index ( f_index);
        /*检查执行时刻的参数是否与所对应的函数的参数数目相符*/
        if(f->arg_counts!=args)
        {
            printf("error,the index %d %s  the args count is not match p: %d a:%d\n",f_index,f->name,f->arg_counts,args);
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

/*将函数所在中间代码节点编译到指定字节码文件中*/
void IL_ListCompile(FILE *f,int func_index)
{
    IL_list_print(f,func_get_by_index (func_index));
}

/*从字节码文件中读入表达式节点*/
void IL_ExpLoad(char * str)
{
    IL_element A,B;
    char exp_str[5][32];
    sscanf (str,"%s %s %s %s %s",exp_str[0],exp_str[1],exp_str[2],exp_str[3],exp_str[4]);
    int tmp_result_index=atoi (exp_str[1]);
    switch(exp_str[2][0])
    {
    case '$':
        A.type=ELEMENT_VAR;
        A.index=atoi(exp_str[2]+1);
        break;
    case '#':
        A.type=ELEMENT_LITERAL;
        A.index=atoi(exp_str[2]+1)+build_GetConstOffset();
        break;
    case '@':
        A.type=ELEMENT_TMP;
        A.index=atoi(exp_str[2]+1);
        break;
    default:
        STOP("THIS EXP NOT SUPPORT YET");
        break;
    }
    int op=atoi(exp_str[3]);
    switch(exp_str[4][0])
    {
    case '$':
        B.type=ELEMENT_VAR;
        B.index=atoi(exp_str[4]+1);
        break;
    case '#':
        B.type=ELEMENT_LITERAL;
        B.index=atoi(exp_str[4]+1)+build_GetConstOffset();
        break;
    case '@':
        B.type=ELEMENT_TMP;
        B.index=atoi(exp_str[4]+1);
        break;
    default:
        STOP("THIS EXP NOT SUPPORT YET");
        break;
    }
    IL_ListInsertEXP (IL_exp_create( op,A,B ),tmp_result_index);
}



/*从字节码文件中读入打印节点*/
void IL_PRNTLoad()
{
    IL_ListInsertNode( IL_node_create_prnt());
}

/*从字节码文件中读入返回节点*/
void IL_RETRNLoad()
{
    IL_ListInsertReturn();
}


/*从字节码文件中读入列表调用节点*/
void IL_CallFuncLoad(char * str)
{
    char func_str[4][32];
    sscanf (str,"%s %s %s %s",func_str[0],func_str[1],func_str[2],func_str[3]);
    int tmp_result_index=atoi(func_str[1]);
    int call_list=atoi(func_str[2]);
    int arg=atoi(func_str[3]);
    IL_ListInsertCall (FUNC_NORMAL,tmp_result_index,arg,call_list);
}

/*从字节码文件中读入标记节点*/
void IL_LableLoad(char *str)
{
    char label_str[2][32];
    sscanf (str,"%s %s",label_str[0],label_str[1]);
    int index=atoi(label_str[1]);
    IL_ListInsertNode( IL_node_create_goto(index,IL_NODE_LAB));
}


/*从字节码文件中读入判零跳转节点*/
void IL_JELoad(char *str)
{
    char JE_str[2][32];
    sscanf (str,"%s %s",JE_str[0],JE_str[1]);
    int index=atoi(JE_str[1]);
    IL_ListInsertNode(IL_node_create_goto(index,IL_NODE_JE))   ;
}

/*从字节码文件中读入无条件跳转节点*/
void IL_JMPLoad(char *str)
{
    char JMP_str[2][32];
    sscanf (str,"%s %s",JMP_str[0],JMP_str[1]);
    int index=atoi(JMP_str[1]);
    IL_ListInsertNode( IL_node_create_goto(index,IL_NODE_GOTO));
}


/*从字节码文件中读入非零跳转节点*/
void IL_JNELoad(char *str)
{
    char JMP_str[2][32];
    sscanf (str,"%s %s",JMP_str[0],JMP_str[1]);
    int index=atoi(JMP_str[1]);
    IL_ListInsertNode (IL_node_create_goto(index,IL_NODE_JNE) );
}


/*从字节码文件中读入动态调用节点*/
void IL_DynamicLoad(char *str)
{
    char dyn_str[4][32];
    sscanf (str,"%s %s %s %s",dyn_str[0],dyn_str[1],dyn_str[2],dyn_str[3]);
    int tmp_result_index=atoi(dyn_str[1]);
    int index=atoi(dyn_str[2]);
    int arg=atoi(dyn_str[3]);
    IL_ListInsertCall (FUNC_DYNAMIC,tmp_result_index,arg,index);
}

/*从字节码中读入调用成员方法节点*/
void IL_MethodCallLoad(char *str)
{
    char met_str[3][32];
    sscanf (str,"%s %s %s",met_str[0],met_str[1],met_str[2]);
    int tmp_result_index=atoi(met_str[1]);
    int arg=atoi(met_str[2]);
    IL_ListInsertCall (FUNC_METHOD,tmp_result_index,arg,-1);
}


/*从字节码中读入构造方法节点*/
void IL_StructCreatorLoad(char * str)
{
    char cre_str[4][32];
    sscanf (str,"%s %s %s %s",cre_str[0],cre_str[1],cre_str[2],cre_str[3]);
    int tmp_result_index=atoi(cre_str[1]);
    int struct_id=atoi(cre_str[2]);
            int arg=atoi(cre_str[3]);
    if(struct_id<0)/*负数说明符号是一个待解析的符号*/
    {
         struct_id-=build_GetUnresolvedOffset();/*重定向带解析标识符*/
        char *str=module_GetUnresolvedSymbol(-struct_id);
        int id=struct_GetPlainId(str);/*先查找看是否该结构已被实现*/
        if(id!=0)/*已被实现*/
        {
            struct_id=id;
            IL_ListInsertStructCreator(struct_id,tmp_result_index,arg);
        }
        else/*没有被实现，则要把其压入引用表中，待之后寻找*/
        {
           IL_node*node= IL_ListInsertStructCreator(struct_id,tmp_result_index,arg);
           /*将字符串压入到未解析的原子表中*/
           module_PutUnresolvedAtom(str,&(node->struct_creator->id));
        }
    }
    else
    {
        struct_id+= build_GetStructOffset();/*重定向结构标识符*/
        IL_ListInsertStructCreator(struct_id,tmp_result_index,arg);
    }


}



/*从字节码中读入元组构造节点*/
void IL_TupleCreatorLoad(char * str)
{
char the_str[3][32];
sscanf (str,"%s %s %s",the_str[0],the_str[1],the_str[2]);
int tmp_result_index=atoi(the_str[1]);
int args=atoi(the_str[2]);
IL_ListInsertListCreator (ELEMENT_TUPLE_CREATOR,tmp_result_index,args);
}


/*从字节码中读入求势节点*/
void IL_CardLoad(char * str)
{
char the_str[2][32];
sscanf (str,"%s %s",the_str[0],the_str[1]);
int tmp_result_index=atoi(the_str[1]);
IL_ListInsertCard (tmp_result_index);
}

/*从字节码中读入类型节点*/
void IL_TypeOfLoad(char * str)
{
char the_str[2][32];
sscanf (str,"%s %s",the_str[0],the_str[1]);
int tmp_result_index=atoi(the_str[1]);
IL_ListInsertTypeof (tmp_result_index);
}


/*删除一个list中的所有节点*/
void IL_FreeAllNode(IL_list * list)
{
    IL_node * c,*t;
    t=list->head;
    while(t)
    {
        c=t->next;
        free(t);
        t=c;
    }
    list->head=NULL;
}
