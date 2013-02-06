/************************************************************************************
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.

�˳���Ϊ�������,���������ѭ�������������ᷢ����GPL ͨ�ù������
֤��������߰汾��Լ�����ٷ�����(��)�޸�
(��GPL���֤��ϸ�������ʿɲο�λ��gpl-CN.txt��ķǹٷ�����)

�����˳�����ϣ������������,�������߶��䲻���κε���,��ʹ����ҵ�ϻ�
�����ض���;����ʽ��������
*********************tzw <tzwtangziwen@gmail.com>******************************/


#include <stdio.h>
#include <assert.h>
#include "vm.h"
#include "var.h"
#include <stdlib.h>
#include <string.h>
//ջ����������������ʱ����ֲ�����
Var vm_stack_var[1024];
//���ڼ�סÿһ������,�����ڻ���
int vm_stack_layer[128];
int vm_stack_offset=0;
int vm_stack_index=0;

void vm_rt_stack_var_cast_set(int the_index,Var source)
{
	vm_stack_var[vm_stack_offset+the_index]=source;
}


//���ָ����������ջ�ֲ�����
Var  vm_rt_stack_var_get(int the_index)
{
	return  vm_stack_var[vm_stack_offset+the_index];
}

//���ָ����������ջ�ֲ�������ָ��
Var *  vm_rt_stack_var_get_ptr(int the_index)
{
	return  &(vm_stack_var[vm_stack_offset+the_index]);
}

//������ʱ����������е�ջ����
void vm_rt_stack_var_set ( int the_index,Var source )
{
	int dist_the_index=vm_get_absolutely(the_index);
	//��������ֵ�Ķ�����һ������,������ʵ���ϸı��������ָ��ı�����ֵ
	if ( vm_stack_var[dist_the_index].content.type==VAR_TYPE_REF )
		{
			Var * a=vm_rt_stack_var_get_abs ( vm_stack_var[dist_the_index].content.handle_value );
			if ( source.content.type!=VAR_TYPE_REF )   //ֻ�б���ֵ�Ķ���������
				{
					( *a ) =source;
				}
			else     //���߶�������
				{
					Var *b;
					b =vm_rt_stack_var_get_abs ( source.content.handle_value );
					( *a ) = ( *b );
				}
		}
	else     //��ֵ�������õ����
		{
			if(source.content.type!=VAR_TYPE_REF)
				{
					//��ֵ��һ���ַ�����
					if(source.content.type==VAR_TYPE_STR)
						{
							//�����ֵҲ�ǣ���ô��Ҫ��������ʹ��
							if(vm_stack_var[dist_the_index].content.type==VAR_TYPE_STR)
								{
									free(vm_stack_var[dist_the_index].content.str);
								}
							vm_stack_var[dist_the_index].content.str=malloc(strlen(source.content.str)+1);
							strcpy(vm_stack_var[dist_the_index].content.str,source.content.str);
						}
					vm_stack_var[dist_the_index]=source;
				}
			else
				{
					Var *b;
					b =vm_rt_stack_var_get_abs ( source.content.handle_value );
					vm_stack_var[dist_the_index]=(*b);
				}
		}
}

//��������л�õ�ǰ��ָ��������ջ������ָ��
Var * vm_rt_stack_var_get_abs ( int the_index )
{
	return & ( vm_stack_var[the_index] );
}

//ͨ����ǰ��������ҵ��þֲ������ľ�������
int vm_get_absolutely ( int the_index )
{
	return ( vm_stack_offset+the_index );
}
//����ʱ��ջ����
void vm_rt_stack_push()
{
	vm_stack_offset+=vm_stack_layer[vm_stack_index];
	vm_stack_index++;

}

//����ʱ��ջ����(����)
void vm_rt_stack_pop()
{
	vm_stack_index--;
	vm_stack_offset-=vm_stack_layer[vm_stack_index];
}
