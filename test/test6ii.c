
#include "syscall.h"

void newThread2(void);

void main()
{
	Printf("\nProcess 2\n");
	Fork(newThread2);
	Yield();
	Yield();
	Yield();
	Yield();
	Exit(0);
}

void newThread2()
{
	Acquire(0);
	Yield();
	Release(0);
	Exit(0);
}
