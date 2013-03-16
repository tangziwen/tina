#include "tina.h"


extern void Tina_Init()
{
	INIT(script_struct);
    INIT(script_tuple);
     INIT(script_vector);
	INIT(vm);
	printf("*********************this is Tina test version!!!*********************\n");
}
