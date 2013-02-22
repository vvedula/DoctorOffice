
#include "syscall.h"

int newLock;
void newThread1(void);

void main()
 {
	Printf("\nProcess 1\n");
	newLock = CreateLock("newLock",7);
	Fork(newThread1);
	Yield();
	Yield();
	Yield();
}

void newThread1()
{
	Acquire(0);
	Yield();
	Release(newLock);
	Exit(0);
}

