#include "syscall.h"

void main()
{

	int monitorVariable = CreateMV("MV",2,5);
	
	int value = GetMV(monitorVariable,4);
	
	DestroyMV(monitorVariable);
	
	Exit(0);
}
