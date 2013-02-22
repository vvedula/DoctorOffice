
#include "syscall.h"

void main()
{
	int lock1 = CreateLock("Lock1",5);
	int cv1 = CreateCV("CV1",3);
	
	Acquire(lock1);
	Wait(cv1,lock1);
	Release(lock1);

	DestroyCV(cv1);
	DestroyLock(lock1);
	
	Exit(0);
}
