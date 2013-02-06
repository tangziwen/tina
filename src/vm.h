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
#ifndef TINA_VM_H
#define TINA_VM_H
#include "type.h"
#include "def.h"
/*vm.h��������tina�ű�����ʱջ�еı����������
 * ��ɨ�躯����ʱ��,����Ϊÿһ��������еľֲ�����
 * ��ָ����һ������ڸú���������
 * �ڽ������ʽ��ʱ�����ǽ������Ե������������м������.
 * �������м����ִ��ʱ,��Ϊ�������õ�ջ����,������Ҫ��λ��������ľ���λ��
 */


//ջ����������������ʱ����ֲ�����
extern Var vm_stack_var[1024];
//���ڼ�סÿһ������,�����ڻ���
extern int vm_stack_layer[128];


//������ʱ��������е�ջ������ֵ,���Ǹñ���������
//�Լ�ֵ�������ʵ��ĸı�,�����һ�����õĻ�,����ȡ��ָ��ı�����ֵ
//
void vm_rt_stack_var_set(int index,Var source);


//���ָ����������ջ�ֲ�����
Var  vm_rt_stack_var_get(int index);



//���ָ����������ջ�ֲ�������ָ��
Var *  vm_rt_stack_var_get_ptr(int index);



//������ʱ�а���ǿ��ת����ջ�������и�ֵ,���Ըñ�������������һ����Ϣ(���������� ����,��ֵ)��
//��Դ�������Ա���һ��
void vm_rt_stack_var_cast_set(int index,Var source);


//��������л�õ�ǰ��ָ��������ջ����
Var * vm_rt_stack_var_get_abs(int index);

//ͨ����ǰ��������ҵ��þֲ������ľ�������
int vm_get_absolutely(int index);

//����ʱ��ջ����
void vm_rt_stack_push();

//����ʱ��ջ����(����)
void vm_rt_stack_pop();
//
extern int vm_stack_offset;
extern int vm_stack_index;
#endif
