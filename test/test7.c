
#include "syscall.h"

int newLock1,newLock2,newCV1,newCV2;

void threadA()
{
	Acquire(newLock1);
	Printf((unsigned int)"\nLock acquired for Waiting...\n");
	Printf((unsigned int)"\nThread A calls Wait ..\n");
	Wait(newCV1,newLock1);
	Release(newLock1);
	Exit(0);
}

void threadB()
{
	Acquire(newLock1);
	Printf((unsigned int)"\nThread B calls broadcast with different CV...\n");
	Broadcast(newCV1+5,newLock1);
	Release(newLock1);
	Exit(0);
}


void threadC()
{
	Acquire(newLock2);
	Printf((unsigned int)"\nLock acquired for Waiting...\n");
	Printf((unsigned int)"\nThread C calls wait..\n");
	Wait(newCV2,newLock2);
	Release(newLock2);
	Exit(0);
}

void threadD()
{
	Acquire(newLock2);
	Printf((unsigned int)"\nLock acquired for Waiting...\n");
	Printf((unsigned int)"\nThread D calls broadcast on different Lock..\n");
	Broadcast(newCV2,newLock2+5);
	Release(newLock2);
	Exit(0);
}

int main()
{
	newLock1 = CreateLock("Lock1",5);
	newCV1 = CreateCV("CV1",3);

	newLock2 = CreateLock("Lock2",5);
	newCV2=CreateCV("CV2",3);

	Fork(threadA);
	Fork(threadB);
	Fork(threadC);
	Fork(threadD);
	
	return 0;
}
