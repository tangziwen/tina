#include "const_segment.h"
#include "var.h"
#include <stdio.h>
#include <string.h>


#define CONST_MAX_SIZE 100

#define CONST_TYPE_MAX 10
#define CONST_TYPE_INT 0
#define CONST_TYPE_BOOL 1
#define CONST_TYPE_NILL 2
#define CONST_TYPE_MSG 3
#define CONST_TYPE_CHAR 4
#define CONST_TYPE_STRUCT_NAME 5
#define CONST_TYPE_REAL 6
#define CONST_TYPE_FUNC 7
#define CONST_TYPE_API 8
#define CONST_TYPE_STR 9
char * const_key_world_list[CONST_TYPE_MAX] ={"i","b","n","m","c","s","r","f","a","str"};

static int look_up_keyword(char *str)
{
    int i=0;
    for ( i=0; i<CONST_MAX_SIZE; i++ )
    {
        if ( strcmp ( const_key_world_list[i],str) ==0 )
        {
            return i;
        }
    }
    return -1;
}

static int  const_index=0;
static Var ConstPool[CONST_MAX_SIZE];/*常量段池*/
/*将指定的变量加入常量段,并返回其索引,用不返回非零值*/
int ConstSegmentPush(Var const_var)
{
    const_index+=1;
    ConstPool[const_index]=const_var;
    return const_index;
}

/*返回当前常量段中拥有的常量的数量*/
int ConstSegmentGetCount()
{
    return const_index;
}

/*返回指定索引的常量段中的变量*/
Var * ConstSegmentGetVar(int index)
{
    return & (ConstPool[index]);
}


/*把常量段写入字节码文件中*/
void ConstSegmentWrite(FILE *f)
{
int i=1;
for(;i<=const_index;i++)
{
    fprintf(f,"C ");
    switch(var_GetType (ConstPool[i]))
    {
    case VAR_TYPE_NILL :
        fprintf(f,"n NILL\n",i);
        break;
    case VAR_TYPE_INT :
        fprintf(f,"i %d\n",var_GetInt (ConstPool[i]));
        break;
    case VAR_TYPE_BOOL :
        fprintf(f,"b %d\n",var_GetBool(ConstPool[i]));
        break;
    case VAR_TYPE_REAL :
        fprintf(f,"r %lf\n",var_GetDouble(ConstPool[i]));
        break;
    case VAR_TYPE_STRUCT_NAME :
        fprintf(f,"s %d\n",var_GetStructId(ConstPool[i]));
        break;
    case VAR_TYPE_CHAR :
        fprintf(f,"c %c\n",var_GetChar(ConstPool[i]));
        break;
    case VAR_TYPE_MESSAGE :
        fprintf(f,"m %s\n",var_GetMsg(ConstPool[i]));
        break;
    case VAR_TYPE_FUNC :
        if(ConstPool[i].content.var_value.func.func_type==FUNC_API)
        {
            fprintf(f,"a %d\n",ConstPool[i].content.var_value.func.func_index);
        }else
        {
            fprintf(f,"f %d\n",ConstPool[i].content.var_value.func.func_index);
        }
        break;
    case VAR_TYPE_STR:
    {
        fprintf(f,"str \"%s\"\n",ConstPool[i].content.var_value.str);
    }
        break;
    }
}
}

/*把常量段从字节码中读取到内存中*/
void ConstSegmentLoad(char *str)
{
    char const_char[3][32];
    sscanf (str,"%s %s %s",const_char[0],const_char[1],const_char[2]);
    int type=look_up_keyword(const_char[1]);
    switch(type)
    {
    case CONST_TYPE_INT:
    {
        int value=atoi(const_char[2]);
        Var a;
        var_SetInt (&a,value);
        ConstSegmentPush (a);
    }
        break;
    case CONST_TYPE_BOOL:
    {
        int value=atoi(const_char[2]);
        Var a;
        var_SetBool(&a,value);
        ConstSegmentPush (a);
    }
        break;
    case CONST_TYPE_CHAR:
    {
        int value=atoi(const_char[2]);
        Var a;
        var_SetChar(&a,value);
        ConstSegmentPush (a);
    }
        break;
    case CONST_TYPE_MSG:
    {
        char * msg=malloc (strlen(const_char[2])+1);
        strcpy (msg,const_char[2]);
        Var a;
        var_SetMsg (&a,msg);
        ConstSegmentPush (a);
    }
        break;
    case CONST_TYPE_NILL:
    {
        Var a;
        var_SetNil(&a);
        ConstSegmentPush(a);
    }
        break;
    case CONST_TYPE_REAL:
    {
        double value=atof(const_char[2]);
        Var a;
        var_SetReal(&a,value);
        ConstSegmentPush (a);
    }
        break;
    case CONST_TYPE_STRUCT_NAME:
    {
        int value=atoi(const_char[2]);
         Var a;
        a.content.var_value.struct_id =value;
        a.content.type=VAR_TYPE_STRUCT_NAME;
        ConstSegmentPush (a);
    }
        break;
    case CONST_TYPE_FUNC:
    {
        int value=atoi(const_char[2]);
         Var a;
         a.content.type=VAR_TYPE_FUNC;
         a.content.var_value.func.func_index=value;
         a.content.var_value.func.func_type=FUNC_NORMAL;
         ConstSegmentPush (a);
    }
        break;
    case CONST_TYPE_API:
    {
        int value=atoi(const_char[2]);
         Var a;
         a.content.type=VAR_TYPE_FUNC;
         a.content.var_value.func.func_index=value;
         a.content.var_value.func.func_type=FUNC_API;
         ConstSegmentPush (a);
    }
        break;
    case CONST_TYPE_STR:
    {
        char const_char[2][32];
        char const_str[65535];/*足够大的缓存空间来存储字符串*/
        sscanf (str,"%s %s",const_char[0],const_char[1]);

        /*处理最后的字符串值*/
        {
            int i=0;
            while(str[i]!='\"')
            {
                i++;
            }
            i+=1;
            int j=0;
            while(str[i]!='\"')
            {
                const_str[j]=str[i];
                j++;
                i++;
            }
            const_str[j]='\0';
        }
        Var a;
        a.content.type=VAR_TYPE_STR;
        int str_size=strlen(const_str);
        a.content.var_value.str=malloc (str_size+1);
        strcpy(a.content.var_value.str,const_str);/*略过开头的引号，从第二个元素开始拷贝*/
        ConstSegmentPush (a);
    }
        break;
    default :
        STOP("INVALID CONST BYTE CODE");
        break;
    }
}
/*清除已有的记录，方便下次编译*/
void ConstSegmentDump()
{
const_index=0;
}
