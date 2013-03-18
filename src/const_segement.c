#include "const_segment.h"
#include "var.h"

static int  const_index=0;
static Var ConstPool[100];/*常量段池*/
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
Var ConstSegmentGetVar(int index)
{
    return ConstPool[index];
}
