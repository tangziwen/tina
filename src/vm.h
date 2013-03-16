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

/*��ȡ��ǰ����ʱ,��ִ�еľֲ��鵱ǰ����*/
int vm_GetCurrentLayer();

/*����ָ�����εľֲ�������Ŀ*/
void vm_SetLayerVarAmount(int index ,int amount);

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
Var * vm_RTstackVarGetAbs(int index);

//ͨ����ǰ��������ҵ��þֲ������ľ�������
int vm_GetAbs(int index);

//����ʱ��ջ����
void vm_RTstackPush();

//����ʱ��ջ����(����)
void vm_RTstackPop();
/*���ü�������һ*/
void RefCountDecrease(int type,void * handle);
/*���ü�������һ*/
void RefCountIncrease(int type,void *handle);

/*�����ǰ�����еľֲ�������ֵ,
��Щ�ֲ�����������ΪNil����
*/
void CleanCurrentLocalVar();
//
#endif
