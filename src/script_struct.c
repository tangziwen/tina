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

//环境索引,最初的环境为全局
int env_index =ENV_GLOBAL;
static int class_index=0;
//粗浅实现类的列表,今后改为更为灵活的数据结构
static  struct_info class_list[30];



static void type_creator()
{
	Var result;
	result.content.type=VAR_TYPE_HANDLE;
	int id=Tina_API_GetTag();
	result.content.handle_value=create_instance(id);

	result.class_id=Tina_API_GetTag();
	//若存在初始化函数,则调用它
	if(class_list[id].initializer_index>=0)
		{

			int old_env =env_index;
			env_index=id;//环境索引改变为类内部
			//函数栈压入
			vm_rt_stack_push();
			Function  *f=func_get_by_index(class_list[id].initializer_index);
			int i;
			self_ptr=result;
			//传入参数
			for(i=0; i<f->arg_counts; i++)
				{
					vm_rt_stack_var_cast_set ( i,API_argument_list[i]);
				}
			IL_list * old_list=current_list;
			func_invoke ( class_list[id].initializer_index);
			current_list=old_list;
			//函数的栈弹出
			vm_rt_stack_pop();
			self_ptr.content.type=VAR_TYPE_VOID;
			env_index=old_env;//维持原样
		}
	Tina_API_SetReturnValue ( result );
}
//通过指定字符串找到类的id
static int plain_get_class_id(const char * name)
{
	int i;
	for(i=0; i<30; i++)
		{
			if(strcmp(class_list[i].name,name)==0)
				{
					return i;
				}
		}
	return -1;
}
//通过指定字符串找到类的id
int get_class_id(const char * name)
{
	int result;
	//检查当前环境所在模块
	result=plain_get_class_id(module_ContextMangledName(name));
	if(result==-1)
	{
		int i=0;
		//检查被using的模块
		for( ; i<module_index;i++)
		{
			result=plain_get_class_id(module_MangledName(i,name));
			if(result!=-1)
			{
				return result;
			}
		}
		//检查全局或者完全限定
		result=plain_get_class_id(name);
		return result;
	}
	else
	{
		return result;
	}
	return -1;
}


//获得类的名字
const char * struct_get_name(int ID)
{
	return class_list[ID].name;
}
//创建类型
static void create_type(int ID,char * name)
{
	//变换成唯一名称形式(有模块名修饰)
	char unique_name[128]={0};
	strcpy(unique_name,module_ContextMangledName(name));
	strcpy(class_list[ID].name,unique_name);
	strcat(unique_name,"@new");
	//每当创建一个类型,就注册一个与类型同名的函数当做API,来构建对象
	int index =Tina_API_Register ( unique_name,type_creator ,ID);
	//保存当前注册函数的索引
	class_list[ID].api_index=index;
}

//创建一个类的实体
void *  create_instance(int ID)
{
	struct_chunk * new_obj= malloc(sizeof(struct_chunk));
	new_obj=memset(new_obj,0,sizeof(struct_chunk));
	int i;
	for( i=0; i<100; i++)
		{
			new_obj->member[i]=class_list[ID].member[i];
		}
	return new_obj;
}

//获得类指定成员的地址
Var * GetObjectMemberAddress(void * ptr, int member_ID)
{
	struct_chunk * obj=(struct_chunk *) ptr;
	return  &(obj->member[member_ID]);
}

void AddMember(int ID,Var value,int accessibility,int is_sealed)
{
	int i;
	//检测有无同名的成员
	for(i=0; i<30; i++)
		{
			//遇到同名的覆盖
			if(strcmp(class_list[ID].member[i].name,value.name)==0)
				{
					class_list[ID].member[i]=value;
					class_list[ID].accessibility[i]=accessibility;
					class_list[ID].is_sealed[i]=is_sealed;
					return;
				}
		}
	class_list[ID].accessibility[class_list[ID].member_count]=accessibility;
	class_list[ID].is_sealed[class_list[ID].member_count]=is_sealed;
	class_list[ID].member[class_list[ID].member_count++]=value;
}

//设置类的成员
 void SetMember(int ID ,int member_id,Var value)
{
	class_list[ID].member[member_id]=value;
}

//获得类的成员
Var GetMember(int ID ,int member_id)
{
	return class_list[ID].member[member_id];
}

int get_index_of_member(int ID,char * name)
{
	int i;
	for(i=0; i <class_list[ID].member_count; i++)
		{
			if(strcmp(class_list[ID].member[i].name,name)==0)
				{
					return i;
				}
		}
	return -1;
}
int get_accessbility_of_member(int ID,int member_id)
{
	return class_list[ID].accessibility[member_id];
}
#define CLASS_A  0
void script_struct_init()
{
	int i;
	for(i=0; i<30; i++)
		{
			class_list[i].initializer_index=-1;
		}
}


//扫描类的声明
void struct_ParseDeclare(int * pos)
{
	TokenInfo t_k;
	token_get ( pos,&t_k );//获取类的名字
	char class_name[128];
	strcpy(class_name,t_k.content);
	create_type(class_index,t_k.content);//创建类型
	class_index++;
}

//扫描类
void struct_ParseDefine(int * pos)
{

	TokenInfo t_k;
	token_get ( pos,&t_k );//获取类的名字
	char class_name [128]={0};
	strcpy(class_name,module_ContextMangledName(t_k.content));
	//获取
	int class_index =get_class_id(class_name);
	//探测是否有继承符号
	int inherit_pos =*pos;
	token_get ( &inherit_pos,&t_k );
	//如果有继承符号?
	if(t_k.type==TOKEN_TYPE_MIX)
		{

			*pos =inherit_pos;
			//获取基类的名字
			token_get_assert ( pos,&t_k,TOKEN_TYPE_STRUCT_NAME,"invalid mix" );
			int base_class_id =get_class_id(t_k.content);//获取基类的id
			int i=0;
			//填充基类成员
			for(i=0; i<class_list[base_class_id].member_count; i++)
				{
					//只添加非密封属性
					if(class_list[base_class_id].is_sealed[i]==CLASS_NOT_SEALED)
						{
							AddMember(class_index,class_list[base_class_id].member[i],class_list[base_class_id].accessibility[i],CLASS_NOT_SEALED);
						}
				}
			token_get_assert ( pos,&t_k,TOKEN_TYPE_LEFT_BRACE, class_name);//获得左花括号
		}
	else
		{
			token_get_assert ( pos,&t_k,TOKEN_TYPE_LEFT_BRACE,class_name);
		}

	int pre_scan_pos=(*pos);
	do
		{
			token_get(&pre_scan_pos,&t_k);
			switch(t_k.type)
				{
					//发现函数的定义符号,声明函数
				case TOKEN_TYPE_FUNC_DEF:

					method_parse_declare(&pre_scan_pos,class_name);
					token_SkipBlock(&pre_scan_pos);//略过类定义块
					break;
					//遇到右大括号
				case TOKEN_TYPE_RIGHT_BRACE:
					goto
					DEF; //跳到
					break;
				}
		}
	while(t_k.type!=TOKEN_TYPE_EOF);
DEF:
	;
	//访问性
	char acc=CLASS_PUBLIC;
	//密封性
	char is_sealed=CLASS_NOT_SEALED;
	do
		{
			token_get ( pos,&t_k );
			switch(t_k.type)
				{
					//访问性限制,私有
				case TOKEN_TYPE_PRIVATE:
					acc=CLASS_PRIVATE;
					break;
					//密封性限制,密封
				case TOKEN_TYPE_SEALED:
					is_sealed=CLASS_SEALED;
					break;
					//类变量成员
				case TOKEN_TYPE_VAR_DEF:
				{
					Var value;
					token_get(pos,&t_k);
					strcpy(value.name,t_k.content);
					value.content.int_value=0;
					value.content.type=VAR_TYPE_INT;
					AddMember(class_index,value,acc,is_sealed);
					token_get ( pos,&t_k );//跳过分号
					if(t_k.type!=TOKEN_TYPE_SEMICOLON)
						{
							printf("illegal def in class %s  %d \n  ", t_k.content,*pos);
							exit(0);
						}
					//恢复到默认为公有
					acc=CLASS_PUBLIC;
					//恢复为非密封函数
					is_sealed=CLASS_NOT_SEALED;
				}
				break;
				//发现函数的定义符号,定义成员函数
				case TOKEN_TYPE_FUNC_DEF:
				{
					//成员函数被变了形,所以我们先试探获得他的名义名字
					int test_pos= *pos;
					token_get(&test_pos,&t_k);
					char member_name [128];
					strcpy(member_name,t_k.content);

					Function *f=method_parse_def(pos,class_name);


					//检测是否为一个初始化函数
					if(strcmp(module_ContextMangledName(member_name),class_list[class_index].name)==0)
						{
							class_list[class_index].initializer_index=func_get_index_by_name(f->name);

						}
					else	//否则是一个普通的函数
						{
							Var func_ptr;
							func_ptr.content.type=VAR_TYPE_FUNC;
							func_ptr.content.func.func_index=func_get_index_by_name(f->name);
							func_ptr.content.func.func_type=FUNC_NORMAL;
							strcpy(func_ptr.name,member_name);
							AddMember(class_index,func_ptr,acc,is_sealed);
						}
					//恢复到默认为公有
					acc=CLASS_PUBLIC;
					//恢复为非密封函数
					is_sealed=CLASS_NOT_SEALED;
				}
				break;
				default:
					break;
					//遇到右大括号
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
return &(class_list[struct_id].member[member_id]);

}
