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
#include "script_struct.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tina.h"
#include "api.h"
#include "var.h"
#include "token.h"
#include "type.h"
#include "function.h"
#include "il.h"
#include "module.h"
#define NDEBUG
#include "debug.h"

#define MEMBER_TYPE_MAX 2
#define MEMBER_TYPE_FUNC 0
#define MEMBER_TYPE_NILL 1
char * member_type_list[MEMBER_TYPE_MAX] ={"f","n"};
static int look_up_keyword(char *str)
{
    int i=0;
    for ( i=0; i<MEMBER_TYPE_MAX; i++ )
    {
        if ( strcmp ( member_type_list[i],str) ==0 )
        {
            return i;
        }
    }
    return -1;
}

/*销毁一个结构体*/
static void free_struct(struct_chunk * obj);
/*添加一个成员*/
static int  AddMember(int ID,Var value,int accessibility);
/*环境索引,最初的环境为全局*/
int env_index =ENV_GLOBAL;
static int struct_index=0;
/*粗略实现类的列表,今后改为更为灵活的数据结构*/
static  struct_info struct_list[30];

/*变量保存的临时池,引用计数为零的变量*/
static struct_chunk * StructTmpPool;

/*创建一个对象，传入该对象对应的原型的ID，并传入构造函数所需的参数(如果有的话)*/
Var  struct_Create(int id,Var init_arg[],int args)
{
	Var result;
	result.content.type=VAR_TYPE_OBJ;
    result.content.var_value.handle_value=create_instance(id);
    result.class_id=id;
	/*若存在初始化函数,则调用它*/
    if(struct_list[id].initializer_index>=0)
	{
		int old_env =env_index;
		env_index=id;/*环境索引改变为类内部*/
		/*函数栈压入*/
		vm_RTstackPush();
		/*获取构造函数*/
        Function  *f=func_get_by_index(struct_list[id].initializer_index);
        if(f->arg_counts!=args)
        {
            STOP("struct constructor not matched!");
        }
		self_ptr=result;
        /*传入参数*/
        int i;
		for(i=0; i<f->arg_counts; i++)
		{
            vm_rt_stack_var_cast_set ( i,init_arg[i]);
		}
		IL_list * old_list=current_list;
        func_PlainInvoke ( struct_list[id].initializer_index);/*执行构造函数*/
		current_list=old_list;
		/*函数的栈弹出*/
		vm_RTstackPop();
		self_ptr.content.type=VAR_TYPE_NILL;
		env_index=old_env;/*维持环境*/
	}
    return result;
}

/*通过指定字符串找到类的id*/
static int plain_get_class_id(const char * name)
{


	int i;
    for(i=1; i<30; i++)
	{
        if(strcmp(struct_list[i].name,name)==0)
		{
			return i;
		}
	}
    return 0;
}
/*通过指定字符串找到类的id*/
int get_class_id(const char * name)
{
    int result=0;
	/*检查当前环境所在模块*/
	result=plain_get_class_id(module_ContextMangledName(name));
    if(result==0)
	{
		int i=0;
		/*检查被using的模块*/
        for( ; i<=module_GetMoudleCount (); i++)
		{
			result=plain_get_class_id(module_MangledName(i,name));
            if(result!=0)
			{
				return result;
			}
		}
		/*检查全局或者完全限定*/
		result=plain_get_class_id(name);
		return result;
	}
	else
	{
		return result;
	}
    return 0;
}
/*清空临时对象池*/
void struct_CleanTmpPool()
{
	struct_chunk * node=StructTmpPool;
	while(node)
	{
		struct_chunk * next=node->tmp_next;
		free_struct(node);
		node=next;
	}
}
/*获得类的名字*/
const char * struct_get_name(int ID)
{
    return struct_list[ID].name;
}
/*创建类型*/
static void create_type(int ID,const char * name)
{
	/*变换成唯一名称形式(有模块名修饰)*/
	char unique_name[128]= {0};
	strcpy(unique_name,module_ContextMangledName(name));
    strcpy(struct_list[ID].name,unique_name);
}


/*创建一个结构体原型,返回其索引*/
extern int Tina_CreateProtype( const char * name,int index)
{
    struct_index++;
	create_type(struct_index,name);
    struct_list[struct_index].initializer_index=index;
    return struct_index;
}


/*为指定结构体原型添加成员,并指定其值.
导入的成员必定是共有成员
返回其索引
*/
extern int Tina_ProtypeAddMember(int id, const char * member_name, Var value)
{
	strcpy(value.name,member_name);
    return AddMember(id,value,STRUCT_PUBLIC);
}

/*
未指定的结构体原型的指定成员赋值
*/
extern void Tina_ProtypeSetMember(int struct_id,int member_id,Var value)
{
    struct_list[struct_id].member[member_id]=value;
}

/*引用计数自增一*/
void StructRefCountIncrease(void * handle)
{
	struct_chunk * obj=(struct_chunk *) handle;
	/*将对象从临时池中取走*/
	if(obj->ref_count==0)
	{
		if(StructTmpPool==obj)
		{
			StructTmpPool=obj->tmp_next;
		}
		else
		{
			struct_chunk*pre=obj->tmp_pre;
			struct_chunk*next=obj->tmp_next;
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
	test(
		printf("increase %d\n",obj->ref_count);
	);

}
/*销毁一个结构体*/
static void free_struct(struct_chunk * obj)
{
	test(
		printf("free obj  %d\n",obj);
	);
	{
	/*
	结构体的某些成员可能也是对象或数组的引用,
	在此要进行判断,并使引用计数器自减
	*/
		int i=0;
		for(; i<obj->member_count; i++)
		{

			int type=var_GetType(obj->member[i]);
			if(type==VAR_TYPE_TUPLE || type ==VAR_TYPE_OBJ)
			{
                RefCountDecrease(type,obj->member[i].content.var_value.handle_value);
			}
		}
	}
	free(obj);
}


/*引用计数自减一*/
void StructRefCountDecrease(void * handle)
{
	struct_chunk * obj=(struct_chunk *) handle;
	obj->ref_count--;
	test(
		printf("decrease %d\n",obj->ref_count);
	);

	if(obj->ref_count<=0)
	{

		free_struct(obj);
	}
}


/*创建一个类的实体*/
void *  create_instance(int ID)
{
	struct_chunk * new_obj= malloc(sizeof(struct_chunk));
	new_obj=memset(new_obj,0,sizeof(struct_chunk));
	int i;
    for( i=0; i<struct_list[ID].member_count; i++)
	{
        new_obj->member[i]=struct_list[ID].member[i];
	}
	/*加入临时对象池之中*/
	new_obj->tmp_pre=NULL;
	new_obj->tmp_next=StructTmpPool;
    new_obj->member_count=struct_list[ID].member_count;
	StructTmpPool=new_obj;
	/*引用计数初始化*/
	new_obj->ref_count=0;
	return new_obj;
}

/*获得类指定成员的地址*/
Var * GetObjectMemberAddress(void * ptr, int member_ID)
{
	struct_chunk * obj=(struct_chunk *) ptr;
	return  &(obj->member[member_ID]);
}

static int  AddMember(int ID,Var value,int accessibility)
{
	int i;
	/*检测有无同名的成员*/
	for(i=0; i<30; i++)
	{
		/*遇到同名的覆盖*/
        if(strcmp(struct_list[ID].member[i].name,value.name)==0)
		{
            struct_list[ID].member[i]=value;
            struct_list[ID].accessibility[i]=accessibility;
			return i;
		}
	}
    struct_list[ID].accessibility[struct_list[ID].member_count]=accessibility;
    struct_list[ID].member[struct_list[ID].member_count]=value;
    struct_list[ID].member_count++;
    return struct_list[ID].member_count-1;
}

/*设置类的成员*/
void SetMember(int ID ,int member_id,Var value)
{
    struct_list[ID].member[member_id]=value;
}

/*获得类的成员*/
Var * GetMember(int ID ,int member_id)
{
    return  & (struct_list[ID].member[member_id]);
}

int get_index_of_member(int ID,char * name)
{
	int i;
    for(i=0; i <struct_list[ID].member_count; i++)
	{
        if(strcmp(struct_list[ID].member[i].name,name)==0)
		{
			return i;
		}
	}
	return -1;
}
int get_accessbility_of_member(int ID,int member_id)
{
    return struct_list[ID].accessibility[member_id];
}
#define CLASS_A  0
void script_struct_init()
{
	int i;
    for(i=1; i<30; i++)
	{
        struct_list[i].initializer_index=-1;
        int j=0;
	}
}


/*扫描类的声明*/
void struct_ParseDeclare(int * pos)
{
    struct_index++;
	TokenInfo t_k;
	token_Get ( pos,&t_k );/*获取类的名字*/
	char class_name[128];
	strcpy(class_name,t_k.content);
	create_type(struct_index,t_k.content);/*创建类型*/
}

/*扫描类*/
void struct_ParseDefine(int * pos)
{

    TokenInfo t_k;
    token_Get ( pos,&t_k );/*获取类的名字*/
    char class_name [128]= {0};
    strcpy(class_name,module_ContextMangledName(t_k.content));
    /*获取*/
    int struct_index =get_class_id(class_name);
    token_get_assert ( pos,&t_k,TOKEN_TYPE_LEFT_BRACE,class_name);
    int pre_scan_pos=(*pos);
    do
    {
        token_Get(&pre_scan_pos,&t_k);
        switch(t_k.type)
        {
        /*发现函数的定义符号,声明函数*/
        case TOKEN_TYPE_FUNC_DEF:

            method_parse_declare(&pre_scan_pos,class_name);
            token_SkipBlock(&pre_scan_pos);/*略过类定义块*/
            break;
            /*遇到右大括号*/
        case TOKEN_TYPE_RIGHT_BRACE:
            goto
            DEF; /*跳到*/
            break;
        }
    }
    while(t_k.type!=TOKEN_TYPE_EOF);
DEF:
    ;
    /*访问性*/
    char acc=STRUCT_PUBLIC;
    do
    {
        token_Get ( pos,&t_k );
        switch(t_k.type)
        {
        /*访问性限制,私有*/
        case TOKEN_TYPE_PRIVATE:
            acc=STRUCT_PRIVATE;
            break;
            /*类变量成员*/
        case TOKEN_TYPE_VAR_DEF:
        {
            Var value;
            token_Get(pos,&t_k);
            strcpy(value.name,t_k.content);
            value.content.type=VAR_TYPE_NILL;
            AddMember(struct_index,value,acc);
            token_Get ( pos,&t_k );/*跳过分号*/
            if(t_k.type!=TOKEN_TYPE_SEMICOLON)
            {
                printf("illegal def in class %s  %d \n  ", t_k.content,*pos);
                exit(0);
            }
            /*恢复到默认为公有*/
            acc=STRUCT_PUBLIC;
        }
            break;
            /*发现函数的定义符号,定义成员函数*/
        case TOKEN_TYPE_FUNC_DEF:
        {
            /*成员函数被变了形,所以我们先试探获得他的名义名字*/
            int test_pos= *pos;
            token_Get(&test_pos,&t_k);
            char member_name [128];
            strcpy(member_name,t_k.content);

            Function *f=method_parse_def(pos,class_name);


            /*检测是否为一个初始化函数*/
            if(strcmp(module_ContextMangledName(member_name),struct_list[struct_index].name)==0)
            {
                struct_list[struct_index].initializer_index=func_get_index_by_name(f->name);
            }
            else	/*否则是一个普通的函数*/
            {
                Var func_ptr;
                func_ptr.content.type=VAR_TYPE_FUNC;
                func_ptr.content.var_value.func.func_index=func_get_index_by_name(f->name);
                func_ptr.content.var_value.func.func_type=FUNC_NORMAL;
                strcpy(func_ptr.name,member_name);
                AddMember(struct_index,func_ptr,acc);
            }
            /*恢复到默认为公有*/
            acc=STRUCT_PUBLIC;
        }
            break;
        default:
            break;
            /*遇到右大括号*/
        case TOKEN_TYPE_RIGHT_BRACE:
            goto
            END;
            break;
        }
    }
    while(t_k.type!=TOKEN_TYPE_EOF);
END:
    ;
}

Var * GetPrototypeMemberAddress(char * struct_name,char * struct_member_name)
{
    int struct_id= get_class_id(struct_name);
    int member_id= get_index_of_member(struct_id,struct_member_name);
    return &(struct_list[struct_id].member[member_id]);

}
/*将成员写入字节码中*/
static void member_Compile(FILE *f,struct_info * prototype)
{
    int i=0;
    for(; i<prototype->member_count;i++)
    {
        fprintf(f,"M %d %s ",prototype->accessibility[i],prototype->member[i].name);
        switch(var_GetType (prototype->member[i]))
        {
        case VAR_TYPE_FUNC:
            fprintf(f,"f %d\n",prototype->member[i].content.var_value.func.func_index);
            break;
        case VAR_TYPE_NILL:
            fprintf(f,"n nill\n");
            break;
        }
    }
}

/*将结构体信息写入字节码中*/
void struct_Compile(FILE *f)
{
    int i=1;
    for( ; i<=struct_index;i++)
    {
        fprintf(f,"S %d %s\n",struct_list[i].initializer_index,struct_list[i].name);
        /*将成员写入字节码中*/
        member_Compile(f,&(struct_list[i] ));
    }
}
/*从字节码中读入结构体信息*/
void struct_Load(char *str)
{
    char struct_str[2][32];
    sscanf (str,"%s %s %s",struct_str[0],struct_str[1],struct_str[2]);
    Tina_CreateProtype(struct_str[2],atoi(struct_str[1]));
}

/*从字节码中读入成员信息*/
void struct_MemberLoad(char *str)
{
    char member_str[5][32];
    sscanf (str,"%s %s %s %s %s",member_str[0],member_str[1],member_str[2],member_str[3],member_str[4]);
    Var member;
    int acc=atoi (member_str[1]);//获取访问性
    strcpy(member.name,member_str[2]);//获取成员名称
    int type=look_up_keyword(member_str[3]);
    switch(type)
    {
    case MEMBER_TYPE_FUNC:
    {
        int index=atoi (member_str[4]);
        member.content.type=VAR_TYPE_FUNC;
        member.content.var_value.func.func_index=index;
        member.content.var_value.func.func_type=FUNC_NORMAL;
        AddMember (struct_index,member,acc);
    }
        break;
    case MEMBER_TYPE_NILL:
    {
        var_SetNil(&member);
        AddMember (struct_index,member,acc);
    }
        break;
    }
}
