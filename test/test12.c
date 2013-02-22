
#include "syscall.h"

void main()
{

	int monitorVariable = CreateMV("MV",2,5);
	int monitorVariable2 = CreateMV("MV2",3,1);
	
	SetMV(monitorVariable,4,10);
	
	DestroyMV(monitorVariable);
	DestroyMV(monitorVariable2);
	
	Exit(0);
}
