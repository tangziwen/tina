/************************************************************************************
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.
*********************tzw <tzwtangziwen@gmail.com>******************************/
#ifndef TINA_VM_H
#define TINA_VM_H
#include "type.h"
#include "def.h"

int vm_GetCurrentLayer();


void vm_SetLayerVarAmount(int index ,int amount);

void vm_rt_stack_var_set(int index,Var source);



Var vm_rt_stack_var_get(int index);




Var *  vm_rt_stack_var_get_ptr(int index);



void vm_rt_stack_var_cast_set(int index,Var source);


Var * vm_RTstackVarGetAbs(int index);

int vm_GetAbs(int index);


void vm_RTstackPush();


void vm_RTstackPop();

void RefCountDecrease(int type,void * handle);

void RefCountIncrease(int type,void *handle);


void CleanCurrentLocalVar();
//
#endif
