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
#include "api.h"
#include "token.h"
#include "expression.h"
#include "var.h"
#include "il.h"
#include "assert.h"
#include "function.h"
#include "module.h"
#include "const_segment.h"
#include "script_struct.h"
#define  OP_ASSCOCIATE_LEFT 1
#define  OP_ASSCOCIATE_RIGHT 2
#define LEFT_BRACE_PIORITY -1
#define LEFT_PARENTHESIS_PIORITY -2
#include <math.h>
int is_exp_parsing=0;
/*获得运算符的结合性*/
static int OpAssociativity(char op)
{
    switch ( op )
    {
    case '=':
        return OP_ASSCOCIATE_RIGHT;
        break;
    default:
        return OP_ASSCOCIATE_LEFT;
        break;
    }
}
/*从函数的位置开始，按照函数的参数形式划分之后的字符串，并返回参数的个数*/
/**/
static int GetArgCount(int pos )
{
    TokenInfo t_k;
    token_get(&pos,&t_k);
    int P=0;/*记录括号的平衡性，若为零则平衡，遇左括号减一，右括号加一*/
    int IsEmpty=1;/*此标记用于判断，在两括号之间，是否是空的.即是否夹着别的东西*/
    int comma_count=0;/*记录逗号的数量*/
    if(t_k.type==TOKEN_TYPE_OP&& t_k.content[0]=='(')
    {
        P-=1;
    }
    else
    {
        printf("GetArgCount: illigal pattern\n");
        exit(0);
    }
    while(1)
    {
        token_get(&pos,&t_k);
        switch(t_k.type)
        {
        case  TOKEN_TYPE_OP:
            if(t_k.content[0]=='(')/*遇到左括号，*/
            {
                P-=1;
            }
            break;
        case TOKEN_TYPE_RIGHT_PARENTHESIS:
            P+=1;
            if(P==0)
            {
                if(IsEmpty==1)/*如果内部未遭遇其他的东西，则说明该函数没有参数*/
                {
                    return 0;
                }
                else
                {
                    /*反之的话，则该函数的参数应该为逗号数的数量加一*/
                    return comma_count+1;
                }
            }
            break;
        case TOKEN_TYPE_COMMA:/*记录逗号数*/
            if(P==-1)/*只记录本层的括号*/
            {
                IsEmpty=0;
                comma_count+=1;
            }
            break;
        case TOKEN_TYPE_SEMICOLON:/*遇到分号，非法的表达式*/
            STOP("Illigal expression !!! SEMICOLON");
            break;
        case TOKEN_TYPE_EOF:/*遇到文件流结尾，显然表达式非法*/
            STOP("Illigal expression !!! EOF");
            break;
        default:/*遇到其他符号，说明双括号内夹有东西，不为空*/
            IsEmpty=0;
            break;
        }
    }
}

/*判断一个字符串是否为一个整数*/
static int IsInteger ( char * str )
{
    int i=0;
    while ( str[i]!='\0' )
    {
        if ( str[i]>'9' || str[i]<'0' )
        {
            return -1;
        }
        i++;
    }
    return 1;
}


/*在解析表达式时，判断前后的词法单元是否以相兼容的次序相连*/
static  int IsTokenCompatible(int pre ,int current,int pos)
{

    /*不允许以运算符作为表达式结尾*/
    if(pre==TOKEN_TYPE_OP && current==TOKEN_TYPE_SEMICOLON)
    {

        printf("error expression in %d\n",pos);
        exit(0);
    }

    /*不允许具有操作数属性的同时出现*/
    if(pre==TOKEN_TYPE_NUM||pre==TOKEN_TYPE_SELF || pre==TOKEN_TYPE_STRUCT_NAME )
    {
        if(current==TOKEN_TYPE_NUM||current==TOKEN_TYPE_SELF || current==TOKEN_TYPE_STRUCT_NAME )
        {

            printf("error expression in %d\n",pos);
            exit(0);
        }
    }
}

/*解析表达式*/
void exp_Parse ( int * pos,int mode,int layer )
{
    is_exp_parsing=1;
    int brace =-1;
    /*记录前一个词法单元的类型，*/
    /*遇到不可能遇到的类型组合当即报错，使得错误寻找更加准确*/
    int pre_tk_type=TOKEN_TYPE_NIL;
    while ( 1 )
    {
        TokenInfo t_k;
        token_get ( pos,&t_k );
        IsTokenCompatible(pre_tk_type,t_k.type,*pos);
        switch ( t_k.type )
        {
        case TOKEN_TYPE_SEMICOLON:
            goto finish_lable;
            break;
        case TOKEN_TYPE_SELF:
        {
            put_self_to_RPN();
        }
            break;
            /*遇到括号将弹出函数的参数*/
            /*当参数并不是一个表达式而是一个单个的数字或变量时*/
            /*我们要做一下变通,把其变为a-> a+NULL的形式*/
        case TOKEN_TYPE_COMMA:
        {
            /*记录操作符的个数,若为零则说明参数是简单形式比如f(a),
            则我们要把它变成f(a+NULL)的形式*/
            int i=0;
            while ( get_top_priority_RPN() !=LEFT_PARENTHESIS_PIORITY )
            {
                TransferStackTop();
                i++;
            }

            if ( i==0 )
            {
                Var a;
                a.content.type=VAR_TYPE_NILL;
                PutLiteralTo_RPN(ConstSegmentPush (a));
                push_op_to_RPN_stack ( '+' );
                TransferStackTop();
            }
        }
            break;
        case TOKEN_TYPE_STRUCT_NAME:
        {
            int test_pos = *pos;
            /*试探读取下一个字符,看是否是左括号,如果是的话,那么就是一个构造函数。*/
            TokenInfo next;
            token_get ( &test_pos,&next );
            if(next.type==TOKEN_TYPE_OP&& next.content[0]=='(')
            {
                /*出现了左括号，那么寻找该结构体类型的构造函数，并把它压入*/
                PushStructCreatorToStack(get_class_id(t_k.content),GetArgCount(*pos));
            }
            else/*若没有出现左括号，那么可能目的是为了访问静态成员*/
            {

                Var struct_var;
                struct_var.content.type=VAR_TYPE_STRUCT_NAME;
                struct_var.content.var_value.struct_id=get_class_id(t_k.content);
                PutLiteralTo_RPN (ConstSegmentPush (struct_var));
            }
        }
            break;
        case TOKEN_TYPE_API:
        {
            int test_pos = *pos;
            /*试探读取下一个字符,看是否是左括号,如果是的话,那么就是一个普通的*/
            /*函数,否则就退化一个函数指针.*/
            TokenInfo next;
            token_get ( &test_pos,&next );
            if(next.type==TOKEN_TYPE_OP&& next.content[0]=='(')
            {
                push_API_to_RPN_stack ( API_Search ( t_k.content ) );
                Element *top =GetRPNStackTop();
                top->info.func_args=GetArgCount(*pos);
            }
            else/*退化成一个函数指针*/
            {
                /*退化成为一个函数指针常量*/
                Var a;
                a.content.type=VAR_TYPE_FUNC;
                /*储存函数索引*/
                a.content.var_value.func.func_index=API_Search(t_k.content);
                a.content.var_value.func.func_type=FUNC_API;
                PutLiteralTo_RPN(ConstSegmentPush (a));
            }
        }
            break;
            /*遇见vector关键字，说明是构造向量的节点*/
        case TOKEN_TYPE_VECTOR:
        {
            PushListCreatorToStack (ELEMENT_VECTOR_CREATE,GetArgCount(*pos));
        }
            break;
            /*遇见tuple关键字，说明是构造向量的节点*/
        case TOKEN_TYPE_TUPLE:
        {
            PushListCreatorToStack (ELEMENT_TUPLE_CREATE,GetArgCount(*pos));
        }
            break;
        case TOKEN_TYPE_FUNC:
        {
            Var a;
            int test_pos = *pos;
            /*试探读取下一个字符,看是否是左括号,如果是的话,那么就是一个普通的*/
            /*函数,否则就退化一个函数指针.*/
            TokenInfo next;
            token_get ( &test_pos,&next );
            if(next.type==TOKEN_TYPE_OP&& next.content[0]=='(')
            {

                push_func_to_RPN_stack ( func_get_index_by_name ( t_k.content ),ELEMENT_FUNC);
                Element *top =GetRPNStackTop();
                top->info.func_args=GetArgCount(*pos);
            }
            else
            {
                /*退化成为一个函数指针常量*/
                a.content.type=VAR_TYPE_FUNC;
                /*储存函数索引*/
                a.content.var_value.func.func_index=func_get_index_by_name(t_k.content);
                a.content.var_value.func.func_type=FUNC_NORMAL;
                PutLiteralTo_RPN(ConstSegmentPush (a));
            }
        }
            break;
        case TOKEN_TYPE_NIL:
        {
            Var a;
            a.content.type=VAR_TYPE_NILL;
            a.content.var_value.int_value=0;
            PutLiteralTo_RPN(ConstSegmentPush (a));
        }
            break;
        case TOKEN_TYPE_CHAR:
        {
            Var a;
            a.content.type=VAR_TYPE_CHAR;
            a.content.var_value.char_value=t_k.content[0];
            PutLiteralTo_RPN(ConstSegmentPush (a));
        }
            break;
        case TOKEN_TYPE_NUM:
        {
            Var a;
            if ( IsInteger ( t_k.content ) ==1 )
            {
                a.content.type=VAR_TYPE_INT;
                a.content.var_value.int_value=atoi ( t_k.content );

            }
            else
            {
                a.content.type=VAR_TYPE_REAL;
                a.content.var_value.real_value=atof ( t_k.content );
            }
            PutLiteralTo_RPN(ConstSegmentPush (a));
        }
            break;
        case TOKEN_TYPE_TRUE:
        {
            Var a;
            a.content.type=VAR_TYPE_BOOL;
            a.content.var_value.bool_value=1;
            PutLiteralTo_RPN(ConstSegmentPush (a));
        }
            break;
        case TOKEN_TYPE_FALSE:
        {
            Var a;
            a.content.type=VAR_TYPE_BOOL;
            a.content.var_value.bool_value=0;
            PutLiteralTo_RPN(ConstSegmentPush (a));
        }
            break;
            /*若是个变量标记，则通过其名称，在当前被扫描function的*/
            /*var_list字段中检索该变量，得到其相对索引*/
        case TOKEN_TYPE_SYMBOL:
        {
            /*有可能该变量是一个函数指针,所以我们先试探读取下一个标记,如果是左括号,则把其视为一个函数*/
            int test_pos = *pos;
            /*试探读取下一个字符,看是否是左括号,如果是的话,那么就是一个普通的*/
            /*函数,否则就退化一个函数指针.*/
            TokenInfo next;
            token_get ( &test_pos,&next );

            /*将该变量升格为函数调用*/
            if(next.type==TOKEN_TYPE_OP && next.content[0]=='(')
            {
                push_func_to_RPN_stack(var_get_local_index ( t_k.content,layer),ELEMENT_CALL_BY_PTR);
                Element *top =GetRPNStackTop();

                top->info.func_args=GetArgCount(*pos);
                //
            }
            else
            {
                /*仅仅视作普通变量*/
                int a= var_get_local_index ( t_k.content,layer );
                put_var_to_RPN ( a );
            }

        }
            break;
            /*成员消息*/
        case TOKEN_TYPE_MESSAGE:
        {
            int test_pos = *pos;
            /*试探读取下一个字符,看是否是左括号,如果是的话,那么就是一个普通的*/
            /*函数,否则就退化一个函数指针.*/
            TokenInfo next;
            token_get ( &test_pos,&next );
            /*把字符串压入*/
            Var a;
            a.content.type=VAR_TYPE_MESSAGE;
            /*在get_token中已经分配过内存了,所以在此直接指向*/
            a.content.var_value.str=t_k.str;
            PutLiteralTo_RPN(ConstSegmentPush (a));
            TransferStackTop();/*把点运算符转移至队列*/
            /*后面出现了左括号，故判定为成员函数调用*/
            if(next.type==TOKEN_TYPE_OP&& next.content[0]=='(')
            {

                push_func_to_RPN_stack(0,ELEMENT_CALL_BY_MEMBER);/*在逆波兰栈中压入call_by_member*/
                Element *top =GetRPNStackTop();
                top->info.func_args=GetArgCount(*pos);
                break;
            }
        }
            break;
            /*字符串字面量*/
        case TOKEN_TYPE_STRING:
        {
            Var a;
            a.content.type=VAR_TYPE_STR;
            a.content.var_value.str=t_k.str;
            PutLiteralTo_RPN(ConstSegmentPush (a));
        }
            break;
        case TOKEN_TYPE_OP:
            if ( isRPN_StackEmpty() ==1 )
            {

                /*遇见方括号,则说明前一个变量充当数组*/
                if ( t_k.content[0]=='[' )
                {
                    push_op_to_RPN_stack(OP_SUBSCRIPT);//放入取下标运算符
                }
                push_op_to_RPN_stack (t_k.content[0] );
            }
            else
            {
                /*如果栈内优先级要高于或等于栈外的,则弹出站内的,再把该运算符压入*/
                /*但是要注意,在这里为了简便,我把"(",也当做是运算符,但是遇到*/
                /*"("时不用弹出,直接如栈,这里"("的优先级值为0(最低),所以要单独*/
                /*判断*/
                /*这里左小括号和左方括号亦当作运算符.,但是它要无条件进栈*/
                if ( t_k.content[0]=='(' || t_k.content[0]=='[' )
                {
                    if ( t_k.content[0]=='[' )/*遇到左方括号，则说明前一个为数组名*/
                    {
                        STOP("ALLRIGHT!!!!");
                        push_op_to_RPN_stack(OP_SUBSCRIPT);//放入取下标运算符
                    }
                    else
                    {
                        if(mode==EXP_CONTROL)
                        {
                            brace--;
                        }
                    }
                    push_op_to_RPN_stack ( t_k.content[0] );
                }
                else
                {
                    /*判断优先级放入运算符，并弹出更加优先的运算符，并要确保不把'(' 、'[ '错误弹出*/
                    if ( (get_top_priority_RPN()>0)&&get_top_priority_RPN()>=GetOpPriority ( t_k.content[0] ) )
                    {
                        TransferStackTop();
                        push_op_to_RPN_stack ( t_k.content[0] );
                    }
                    else
                    {
                        push_op_to_RPN_stack ( t_k.content[0] );
                    }
                }

            }
            break;
            /*遇到右小括号后*/
        case TOKEN_TYPE_RIGHT_PARENTHESIS:
        {
            /*用这个变量检测是否放入了运算符*/
            int i=0;

            if(mode==EXP_CONTROL)
            {
                brace+=1;
                if(brace==0)
                {

                    goto finish_lable;
                }
            }
            /*把左小括号 之后的所有运算符放入逆波兰队列中*/
            while ( get_top_priority_RPN() !=LEFT_PARENTHESIS_PIORITY && RPN_GetStackTopType() ==ELEMENT_OP )
            {
                TransferStackTop();
                i++;
            }
            /*删除左括号*/
            RemoveRPN_StackTop();

            /*之前的过程并没有放入运算符,且此时栈顶为一个函数,
            那么这里有可能出现如下的情况
            (a)函数的参数只有一个,且是单元表达式的情况:F(a),这就要把函数的参数的单元表达式a,变为a+NULL的形式
            (b)函数本身就是一个没有参数的过程:F(),这时我们要直接把函数压入
            (c)函数有若干参数，但是传递参数的时候，仅为单元形式:F(3,2,1)
            所以我们需要根据索引查找当前函数的参数个数
            */
            if ( i==0&&
                 ( RPN_GetStackTopType() ==ELEMENT_API
                   || RPN_GetStackTopType() ==ELEMENT_FUNC
                   ||RPN_GetStackTopType()==ELEMENT_CALL_BY_PTR
                   ||RPN_GetStackTopType()==ELEMENT_CALL_BY_MEMBER
                   ||RPN_GetStackTopType()==ELEMENT_VECTOR_CREATE
                   ||RPN_GetStackTopType()==ELEMENT_STRUCT_CREATOR
                   )
                 )
            {
                Element * top= GetRPNStackTop();
                if ( top->info.func_args!=0 )//情况(a),(c)
                {
                    Var nul_var;
                    nul_var.content.type=VAR_TYPE_NILL;
                    PutLiteralTo_RPN(ConstSegmentPush (nul_var));
                    push_op_to_RPN_stack ( '+' );
                    TransferStackTop();
                }
                else if(top->info.func_args==0)//情况(b)
                {
                    TransferStackTop();
                }
            }
            else/*不出现上述情况下,直接将函数移走*/
            {
                /*如果顶端是个函数或是具有类似性质的构造器,则将其放入队列中*/
                if ( RPN_GetStackTopType() ==ELEMENT_API
                     || RPN_GetStackTopType() ==ELEMENT_FUNC
                     ||RPN_GetStackTopType()==ELEMENT_CALL_BY_PTR
                     || RPN_GetStackTopType()==ELEMENT_CALL_BY_MEMBER
                     ||RPN_GetStackTopType()==ELEMENT_VECTOR_CREATE
                     ||RPN_GetStackTopType()==ELEMENT_STRUCT_CREATOR
                     )
                {
                    TransferStackTop();
                }
            }
        }
            break;
            /*右方括号的情况*/
        case TOKEN_TYPE_RIGHT_BRACKET:
        {
            /*用这个变量检测是否放入了运算符*/
            int i=0;
            /*把左方括号之前的所有运算符放入逆波兰队列中*/
            while ( get_top_priority_RPN() !=LEFT_BRACE_PIORITY && RPN_GetStackTopType() ==ELEMENT_OP )
            {
                TransferStackTop();
                i++;
            }
            /*删除左方号括*/
            RemoveRPN_StackTop();
        }
            break;
        default:
            break;
        }
        pre_tk_type=t_k.type;
    }
finish_lable:
    ;
    /*当符号栈内不为空,则全部的放入逆波兰队列中*/
    while ( !isRPN_StackEmpty() )
    {
        TransferStackTop();
    }
    /*把单个的变量a变为a+NULL的形式*/
    if ( GetRPNQueueTail() ==ELEMENT_VAR|| GetRPNQueueTail() == ELEMENT_LITERAL )
    {
        Var nul_var;
        nul_var.content.type=VAR_TYPE_NILL;
        PutLiteralTo_RPN(ConstSegmentPush (nul_var));
        push_op_to_RPN_stack ( '+' );
        TransferStackTop();
    }

    /*PrintRPN();*/
    /*转换成中间码*/
    generate_IL();
    /*重设表达式相关数据,为下次计算做准备*/
    exp_reset();
    is_exp_parsing=0;

}



Element RPN_Queue[32];
int RPN_QueueIndex=-1;


/*把一个字面量放入逆波兰队列之中，参数为其在常量段的索引*/
void PutLiteralTo_RPN(int index)
{
    RPN_QueueIndex++;
    RPN_Queue[RPN_QueueIndex].type=ELEMENT_LITERAL;
    RPN_Queue[RPN_QueueIndex].index=index;
}

/*把self指针压入*/
void put_self_to_RPN()
{
    RPN_QueueIndex++;
    RPN_Queue[RPN_QueueIndex].type=ELEMENT_SELF;
}
/*把一个变量压入逆波兰表达式中*/
void put_var_to_RPN ( int index )
{
    RPN_QueueIndex++;
    RPN_Queue[RPN_QueueIndex].type=ELEMENT_VAR;
    RPN_Queue[RPN_QueueIndex].index=index;
}


Element RON_stack[16];
int RPN_StackIndex=-1;
/*把一个操作符压入转换逆波兰表达式的辅助栈中*/
/*输入的是该运算符的字符形式.*/
void push_op_to_RPN_stack ( char op )
{
    RPN_StackIndex+=1;
    RON_stack[RPN_StackIndex].op=op;
    RON_stack[RPN_StackIndex].type = ELEMENT_OP;
}

/*把一个API压入转换逆波兰的辅助栈中,输入的是他的索引值*/
void push_API_to_RPN_stack ( int index )
{
    RPN_StackIndex++;
    RON_stack[RPN_StackIndex].index=index;
    RON_stack[RPN_StackIndex].type=ELEMENT_API;
    RON_stack[RPN_StackIndex].op=OP_CALL;
}

/*把结构体构造器压入*/
void PushStructCreatorToStack(int struct_id,int arg)
{
    RPN_StackIndex++;
    RON_stack[RPN_StackIndex].info.func_args=arg;
    RON_stack[RPN_StackIndex].type=ELEMENT_STRUCT_CREATOR;
    RON_stack[RPN_StackIndex].op=OP_CALL;
    RON_stack[RPN_StackIndex].index=struct_id;
}

void PushListCreatorToStack(int type,int args)
{
    RPN_StackIndex++;
    RON_stack[RPN_StackIndex].type=type;
    RON_stack[RPN_StackIndex].op=OP_CALL;
    RON_stack[RPN_StackIndex].info.func_args=args;
}




/*把一个脚本函数压入转换逆波兰的辅助栈中,输入的是他的索引值*/
/*mode是它的模式有两种,一种是函数名,一种是函数指针*/
void push_func_to_RPN_stack ( int index,int mode)
{
    RPN_StackIndex++;
    RON_stack[RPN_StackIndex].index=index;
    RON_stack[RPN_StackIndex].type=mode;
    RON_stack[RPN_StackIndex].op=OP_CALL;
}


int RPN_GetStackTopType()
{
    return RON_stack[RPN_StackIndex].type;
}
/*获得逆波兰表达式辅助栈栈顶的运算符的优先级*/
int get_top_priority_RPN()
{
    return GetOpPriority ( RON_stack[RPN_StackIndex].op );
}

/*检测逆波兰表达式的栈是否为空栈是否为空*/
int isRPN_StackEmpty()
{
    if ( RPN_StackIndex<0 )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*获得栈中最上面的元素*/
Element  * GetRPNStackTop()
{
    return &(RON_stack[RPN_StackIndex]);
}

/*转移栈顶的操作符或函数进入输出队列*/
void TransferStackTop()
{
    RPN_QueueIndex++;
    RPN_Queue[RPN_QueueIndex]=RON_stack[RPN_StackIndex];
    RPN_StackIndex--;
}
/*把输出队列的末尾的元素移入栈中*/
void TransferQueueEnd ( int type )
{
    RPN_StackIndex++;
    RON_stack[RPN_StackIndex]=RPN_Queue[RPN_QueueIndex];
    RON_stack[RPN_StackIndex].type=type;
    RPN_QueueIndex--;
}
/*销毁栈顶的元素*/
void RemoveRPN_StackTop()
{
    RPN_StackIndex--;
}

/*获取操作符的优先级*/
int GetOpPriority ( char op )
{
    switch ( op )
    {
    case '(':
        return -2;
    case '[':
        return -1;
    case '=':
        return 1;
    case OP_ASSIGN_DIVIDE:
        return 1;
    case OP_ASSIGN_MINUS:
        return 1;
    case OP_ASSIGN_MULTIPLE:
        return 1;
    case OP_ASSIGN_PLUS:
        return 1;
    case OP_AND :
        return 2;
    case OP_OR :
        return 2;
    case OP_EQUAL:
        return 3;
    case OP_NOT_EQUAL:
        return 3;
    case '>':
        return 4;
    case '<':
        return 4;
    case OP_LESS_OR_EQUAL:
        return 5;
    case OP_LARGE_OR_EQUAL:
        return 5;
    case '+':
        return 6;
    case '-':
        return 6;
    case '*':
        return 7;
    case '/':
        return 7;
    case OP_CALL:
        return 9;
    case '.':
        return 9;
    case OP_SUBSCRIPT:
        return 9;
    default :
        printf ( "in  \"GetOpPriority\",illigal operator   %d\n",op );
        exit(0);
        return -100;
        break;
    }
    return -100;
}
/*获得队列尾部信息*/
int GetRPNQueueTail()
{
    return RPN_Queue[RPN_QueueIndex].type;
}
/*打印逆波兰队列*/
void PrintRPN()
{
    printf("PRN:\n");
    int i;
    for ( i=0; i<=RPN_QueueIndex; i++ )
    {
        switch ( RPN_Queue[i].type )
        {
        case ELEMENT_OP:
            printf ( "%d",RPN_Queue[i].op );
            break;
        case ELEMENT_LITERAL:
            printf("#%d ",RPN_Queue[i].index);
            break;
        case ELEMENT_VAR:
            printf ( "var " );
            break;
        case ELEMENT_API:
            printf ( " API " );
            break;
        case ELEMENT_FUNC:
            printf ( " FUNC " );
            break;
        case ELEMENT_CALL_BY_PTR:
            printf("CALL_BY_PTR ");
            break;
        case ELEMENT_CALL_BY_MEMBER:
            printf("CALL_MEMBER ");
            break;
        default:
            break;
        }
    }
    printf ( "\nEND:\n" );
}

static Element IL_Stack[32];
static int IL_StackIndex=-1;

/*根据以求得的逆波兰表达式生成四元式的中间代码*/
void generate_IL()
{
    /*临时变量索引*/
    int tmp_index=-1;
    int i =0;
    IL_element A;
    IL_element B;

    for ( i=0; i<=RPN_QueueIndex; i++ )
    {
        switch ( RPN_Queue[i].type )
        {
        /*遇见变量,将其放入中间栈中*/
        case ELEMENT_VAR:
            IL_StackIndex++;
            IL_Stack[IL_StackIndex].type=ELEMENT_VAR;
            IL_Stack[IL_StackIndex].index=RPN_Queue[i].index;/*储存其对于其函数的相对索引*/
            break;
            /*遇到self指针也当成普通变量压入*/
        case ELEMENT_SELF:
            IL_StackIndex++;
            IL_Stack[IL_StackIndex].type=ELEMENT_SELF;
            IL_Stack[IL_StackIndex].index=RPN_Queue[i].index;/*储存其对于其函数的相对索引*/
            break;
            /*遇见常量也将其放入中间栈中*/
        case ELEMENT_LITERAL:
            IL_StackIndex++;
            IL_Stack[IL_StackIndex].type=ELEMENT_LITERAL;
            IL_Stack[IL_StackIndex].index=RPN_Queue[i].index;/*储存其对于其函数的相对索引*/
            break;
        case ELEMENT_STRUCT_CREATOR:
        {
            /*向栈前去寻找参数.*/
            Element e =RPN_Queue[i];
            /*然后从栈中抹去参数*/
            int k=IL_StackIndex-e.info.func_args;
            IL_StackIndex=k;
            /*把结果加入到中间语言执行序列中去*/
            tmp_index++;
            IL_ListInsertStructCreator(e.index,tmp_index,e.info.func_args);
            /*同时把中间结果写回栈中*/
            IL_StackIndex++;
            IL_Stack[IL_StackIndex].type=ELEMENT_TMP;
            IL_Stack[IL_StackIndex].index=tmp_index;
        }
            break;
            /*遇见API其地位等同于操作符*/
            /*做一个变形，将API设为a操作数，变成a+nil的形式*/
        case ELEMENT_API:
        {
            /*向栈前去寻找参数.*/
            Element e =RPN_Queue[i];
            /*然后从栈中抹去参数*/
            int k=IL_StackIndex-e.info.func_args;
            IL_StackIndex=k;
            /*把结果加入到中间语言执行序列中去*/
            tmp_index++;
            IL_ListInsertCall (FUNC_API,tmp_index,e.info.func_args,e.index);
            /*同时把中间结果写回栈中*/
            IL_StackIndex++;
            IL_Stack[IL_StackIndex].type=ELEMENT_TMP;
            IL_Stack[IL_StackIndex].index=tmp_index;
        }
            break;
            /*遇见FUNC其地位等同于操作符*/
            /*做一个变形，将FUNC设为a操作数，变成a+nil的形式*/
        case ELEMENT_FUNC:
        {

            /*向栈前去寻找参数.*/
            Element e =RPN_Queue[i];
            /*然后从栈中抹去参数*/
            int k=IL_StackIndex-e.info.func_args;
            IL_StackIndex=k;
            /*把结果加入到中间语言执行序列中去*/
            tmp_index++;
            IL_ListInsertCall (FUNC_NORMAL,tmp_index,e.info.func_args,e.index);
            /*同时把中间结果写回栈中*/
            IL_StackIndex++;
            IL_Stack[IL_StackIndex].type=ELEMENT_TMP;
            IL_Stack[IL_StackIndex].index=tmp_index;
        }
            break;
            /*遇见脚本中的函数指针调用*/
        case ELEMENT_CALL_BY_PTR:
        {
            /*向栈前去寻找参数.*/
            Element e =RPN_Queue[i];
            /*然后从栈中抹去参数*/
            int k=IL_StackIndex-e.info.func_args-1;
            IL_StackIndex=k;
            /*把结果加入到中间语言执行序列中去*/
            tmp_index++;
            IL_ListInsertCall (FUNC_DYNAMIC,tmp_index,e.info.func_args,RPN_Queue[i].index);
            /*同时把中间结果写回栈中*/
            IL_StackIndex++;
            IL_Stack[IL_StackIndex].type=ELEMENT_TMP;
            IL_Stack[IL_StackIndex].index=tmp_index;
        }
            break;
        case ELEMENT_CALL_BY_MEMBER:
        {
            /*向栈前去寻找参数.*/
            Element e =RPN_Queue[i];
            /*然后从栈中抹去参数*/
            int k=IL_StackIndex-e.info.func_args;
            IL_StackIndex=k;
            /*把结果加入到中间语言执行序列中去*/
            tmp_index++;
            IL_ListInsertCall (FUNC_METHOD,tmp_index,e.info.func_args,-1);
            /*同时把中间结果写回栈中*/
            IL_StackIndex++;
            IL_Stack[IL_StackIndex].type=ELEMENT_TMP;
            IL_Stack[IL_StackIndex].index=tmp_index;
        }
            break;
            /*向量构造函数*/
        case ELEMENT_VECTOR_CREATE:
        {

            /*向栈前去寻找参数.*/
            Element e =RPN_Queue[i];
            /*然后从栈中抹去参数*/
            int k=IL_StackIndex-e.info.func_args;
            IL_StackIndex=k;
            /*把结果加入到中间语言执行序列中去*/
            tmp_index++;
            IL_ListInsertListCreator (ELEMENT_VECTOR_CREATE,tmp_index,e.info.func_args);
            /*同时把中间结果写回栈中*/
            IL_StackIndex++;
            IL_Stack[IL_StackIndex].type=ELEMENT_TMP;
            IL_Stack[IL_StackIndex].index=tmp_index;
        }
            break;
            /*元组构造函数*/
        case ELEMENT_TUPLE_CREATE:
        {
            /*向栈前去寻找参数.*/
            Element e =RPN_Queue[i];
            /*然后从栈中抹去参数*/
            int k=IL_StackIndex-e.info.func_args;
            IL_StackIndex=k;
            /*把结果加入到中间语言执行序列中去*/
            tmp_index++;
            IL_ListInsertListCreator (ELEMENT_TUPLE_CREATE,tmp_index,e.info.func_args);
            /*同时把中间结果写回栈中*/
            IL_StackIndex++;
            IL_Stack[IL_StackIndex].type=ELEMENT_TMP;
            IL_Stack[IL_StackIndex].index=tmp_index;
        }
            break;
            /*遇见操作符,则从中间栈中中取出两个数,并加入到中间语言执行序列之中*/
        case ELEMENT_OP:
            /*注意从栈中取出的顺序与计算顺序相反,先取b再取a*/
            B.type=IL_Stack[IL_StackIndex].type;
            B.index=IL_Stack[IL_StackIndex].index;
            B.info=IL_Stack[IL_StackIndex].info;
            IL_StackIndex--;
            A.type=IL_Stack[IL_StackIndex].type;
            A.index=IL_Stack[IL_StackIndex].index;
            A.info=IL_Stack[IL_StackIndex].info;
            IL_StackIndex--;
            /*把结果加入到中间语言执行序列中去*/
            tmp_index++;
            IL_ListInsertEXP ( IL_exp_create( RPN_Queue[i].op,A,B ),tmp_index);
            /*同时把中间结果写回栈中*/
            IL_StackIndex++;
            IL_Stack[IL_StackIndex].type=ELEMENT_TMP;
            IL_Stack[IL_StackIndex].index=tmp_index;
            break;
        }
    }
}



void exp_reset()
{
    IL_StackIndex=-1;
    RPN_QueueIndex=-1;
    RPN_StackIndex=-1;
}
