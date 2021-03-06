
#include "syscall.h"

void main()
{
	int lock1 = CreateLock("Lock1",5);
	int cv1 = CreateCV("CV1",3);
	int mv1 = CreateMV("MV1",3,1);
	
	Acquire(lock1);
	SetMV(mv1,0,1);
	Wait(cv1,lock1);
	Signal(cv1,lock1);
	Release(lock1);

	DestroyCV(cv1);
	DestroyLock(lock1);
	
	Exit(0);
}
