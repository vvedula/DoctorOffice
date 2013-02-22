#include "syscall.h"

void main()
{
	int lock1 = CreateLock("Lock1",5);
	int cv1 = CreateCV("CV1",3);
	int mv1 = CreateMV("MV1",3,1);
	
	while(1)
	{
		if(GetMV(mv1,0) == 1)
		{
			Acquire(lock1);
			Signal(cv1,lock1);
			Wait(cv1,lock1);	
			Release(lock1);
			break;
		}
	}	
	DestroyCV(cv1);
	DestroyLock(lock1);
	
	Exit(0);
}
