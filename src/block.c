
#include <stdlib.h>
#include <stdio.h>
#include "block.h"
#include "tina.h"
#include "print.h"
#include "token.h"
#include "if.h"
#include "var.h"
#include "expression.h"
#include "il.h"
#include "while.h"
#include "return.h"
#include "for.h"
#include "function.h"

/*该字串用于储存最近一次调用的状态信息信息*/
static char state_str[128];
int block_Parse(int * pos,int layer,int break_label,int continue_label,int mode)
{
    TokenInfo t_k;
    int brace =-1;
    /*略过左大括号*/
    token_Get(pos,&t_k);
    /*进入新的层次*/
    layer++;
    while(1)
    {
        /*因为需要更多的信息,所以我们有两个变量
         *控制步进关系*/
        int test_pos=*pos;
        token_Get(&test_pos,&t_k);
        switch(t_k.type)
        {
        case TOKEN_TYPE_EOF:
            exit(0);
            break;
        case TOKEN_TYPE_SELF:
            if(mode ==FUNC_GLOBAL)
            {
                sprintf(state_str,"you can only use \"self\" in method\n");
                return -1;
            }
            else
            {
                if(mode==FUNC_MEMBER)
                {
                    /*回退一格,解析表达式*/
                    exp_Parse(pos,EXP_NORMAL,layer);
                }
            }
            break;
        case TOKEN_TYPE_BREAK:
        {
            if(break_label==-1)
            {
                sprintf(state_str,"error,there is no loop \n");
                return -1;
            }
            IL_node * break_node = IL_node_create_jmp ( break_label,IL_NODE_JMP );
            (*pos)=test_pos;
            token_Get(pos,&t_k);
            IL_ListInsertNode(break_node);
        }
        break;
        case TOKEN_TYPE_CONTINUE:
        {
            if(continue_label==-1)
            {
                sprintf(state_str,"error,there is no loop \n");
                return -1;
            }
            IL_node * continue_node = IL_node_create_jmp ( continue_label,IL_NODE_JMP );
            (*pos)=test_pos;
            token_Get(pos,&t_k);
            IL_ListInsertNode(continue_node);
        }
        break;
        case TOKEN_TYPE_CHAR:
            /*回退一个,解析表达式*/
            exp_Parse(pos,EXP_NORMAL,layer);
            break;
        case TOKEN_TYPE_VAR_DEF:
            (*pos)=test_pos;
            exp_Parse(pos,EXP_NORMAL,layer);
            break;
        case TOKEN_TYPE_NUM:
            /*回退一格,解析表达式*/
            exp_Parse(pos,EXP_NORMAL,layer);
            break;
        case TOKEN_TYPE_SYMBOL:
            /*回退一个,解析表达式*/
            exp_Parse(pos,EXP_NORMAL,layer);
            break;
        case TOKEN_TYPE_RETURN:/*解析返回值表达式*/
            (*pos)=test_pos;
            return_parse(pos,layer);
            break;
        case TOKEN_TYPE_API:
            /*回退一个,解析表达式*/
            exp_Parse(pos,EXP_NORMAL,layer);
            break;
        case TOKEN_TYPE_STRUCT_NAME:
            /*回退一个,解析表达式*/
            exp_Parse(pos,EXP_NORMAL,layer);
            break;
        case TOKEN_TYPE_FUNC:
            /*回退一个,解析表达式*/
            exp_Parse(pos,EXP_NORMAL,layer);
            break;
            /*遇到左括号,当前层次增加*/
        case TOKEN_TYPE_LEFT_BRACE:
            brace--;
            layer++;
            (*pos)=test_pos;
            break;
            /*遇到右括号当前层次减少,整段的销毁原先最内层次的变量.*/
        case TOKEN_TYPE_RIGHT_BRACE:
            brace++;
            layer--;
            (*pos)=test_pos;
            if(brace==0)
            {
                goto end;
            }
            break;
        case TOKEN_TYPE_PRINT:
            (*pos) =test_pos;
            print_parse(pos,layer);
            break;
        case TOKEN_TYPE_WHILE:
            (*pos) =test_pos;
            while_parse(pos,layer,mode);
            break;
        case TOKEN_TYPE_FOR:
            (*pos) =test_pos;
            for_parse ( pos,layer ,mode);
            break;
            /*解析if语句*/
        case TOKEN_TYPE_IF:
            (*pos) =test_pos;
            /*这里直接修改外部的位置了*/
            if_parse(pos,layer,break_label,continue_label,mode);
            break;
        case TOKEN_TYPE_OP:
            /*如果是左小括号的话,回退一格,进行表达式求值,如果不是*/
            /*则是一个编译错误*/
            if(t_k.content[0]=='(')
            {
                exp_Parse(pos,EXP_NORMAL,layer);
            }
            else
            {
                sprintf(state_str,"error illegal operator %c\n",t_k.content[0]);
                return -1;
            }
            break;
        default :
            sprintf(state_str,"error !! token %d is unknown %s\n",t_k.type,t_k.content);
            return -1;
            break;
        }
    }
end:
    sprintf(state_str,"block_Parse worked fine\n");
    return 0;
    ;
}

/*获取最近一次block_Parse状态信息*/
char * block_GetLastStateStr()
{
    return state_str;
}
