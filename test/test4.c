#include "syscall.h"

void testLockLimit()
{
	int newLock,k;
	for(k=0;k<105;k++)
	{
		newLock = CreateLock("aaa",3);	
	}
	Exit(0);
}

void testCVLimit()
{
	int newCV,k;
	for(k=0;k<105;k++)
	{
		newCV = CreateCV("aaa",3);	
	}
	Exit(0);
}

int main()
{
	Fork(testLockLimit);
	Fork(testCVLimit);
	
	return 0;
}
