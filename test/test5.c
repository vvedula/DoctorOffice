#include "syscall.h"

int newLock,newCV;

void testa(void);
void testb(void);

void testa()
{
	Acquire(newLock);
	Printf("\nLock acquired to Wait...\n");
	Wait(newCV,newLock);
	Release(newLock);
	Exit(0);
}

void testb()
{
	Acquire(newLock);
	Printf("\nLock acquired to Signal...\n");
	Signal(newCV,newLock);
	Signal(newCV,newLock);
	Release(newLock);
	Exit(0);
}

int main()
{
	newLock = CreateLock("newLock",7);
	newCV = CreateCV("newCV",5);
	
	Fork(testa);
	Fork(testb);
	
	return 0;
}
