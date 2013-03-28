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
#ifndef TINA_CLASS_H
#define TINA_CLASS_H
#include "type.h"
#include "def.h"
typedef struct
{
	char name[32];
	Var  member [100];
	char accessibility[100];//访问性
	int member_count;
	int initializer_index;
	int api_index;
} struct_info;

typedef struct struct_chunk_t
{
	int ref_count;/*引用计数器*/
	Var member[100];
	struct struct_chunk_t *tmp_pre;
	struct struct_chunk_t * tmp_next;
	int member_count;
} struct_chunk;

void struct_init();
Var * GetObjectMemberAddress(void * ptr, int member_ID);
int get_index_of_member(int ID,char * name);
void *  create_instance(int ID);
const char * struct_get_name(int ID);
int get_accessbility_of_member(int ID,int member_id);
void struct_ParseDeclare(int * pos);
void struct_ParseDefine(int * pos);
int get_class_id(const char * name);
void SetMember(int ID ,int member_id,Var value);

/*创建一个对象，传入该对象对应的原型的ID，并传入构造函数所需的参数(如果有的话)*/
Var  struct_Create(int id, Var init_arg[], int args);
/*引用计数自增一*/
void StructRefCountIncrease(void * handle);
/*引用计数自减一*/
void StructRefCountDecrease(void * handle);

//获得类的成员
Var *GetMember(int ID ,int member_id);
/*清空临时对象池*/
void struct_CleanTmpPool();
Var * GetPrototypeMemberAddress(char * struct_name,char * struct_member_name);
/*将结构体信息写入字节码中*/
void struct_WriteByteCode(FILE *f);
/*从字节码中读入成员信息*/
void struct_MemberLoad(char *str);
/*从字节码中读入结构体信息*/
void struct_Load(char *str);

/*将结构体中的数据清除，方便下次调用*/
void struct_Dump();
/*通过指定字符串找到类的id*/
int struct_GetPlainId(const char * name);

/*返回当前一共有的结构体*/
int struct_GetCount();
#endif
