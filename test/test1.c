/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"
void testLock(void);
void testCV(void);
void testWaitSignalBroadcast(void);


void testLock()
{
	int newLock=0;

	Printf((unsigned int)"\n\ni.a Destroying a lock acquired by some other process...");
	newLock = CreateLock("aaa",3);	
	Acquire(newLock);
	DestroyLock(newLock);


	Printf((unsigned int)"\n\nii. Destroying a lock already destroyed...");
	newLock = CreateLock("abb",3);
	Acquire(newLock);
	Release(newLock);
	DestroyLock(newLock);
	DestroyLock(newLock);


	Printf((unsigned int)"\n\niii.Invalid length of Lock name string...");
	newLock = CreateLock("aaa",-3);
	
	Printf((unsigned int)"\n\niv.Lock name > 100 characters...");
	newLock=CreateLock("aaa",101);	
	
	Printf((unsigned int)"\n\nv.Invalid Virtual Address...");
	newLock=CreateLock(-1,1);

	Printf((unsigned int)"\n\nvi. Invalid length to Acquire function...");
	Acquire(-5);

	Printf((unsigned int)"\n\nvii. Acquiring a lock which is not created...");
	Acquire(55);

	Printf((unsigned int)"\n\nviii. Invalid length to Release function...");
	Release(190);

	Printf((unsigned int)"\n\nix. Releasing a lock which is not created...");
	Release(25);
	
	Exit(0);
}


void testCV()
{
	int newCV;

	Printf((unsigned int)"\n\ni.Destroying a CV already destroyed...");
	newCV = CreateCV("aaa",3);
	DestroyCV(newCV);
	DestroyCV(newCV);

	Printf((unsigned int)"\n\nii.Invalid length to CreateCV function...");
	newCV = CreateCV("aaa",-3);

	Printf((unsigned int)"\n\niii. Invalid virtual address...");
	newCV = CreateCV(-1,1);

	Exit(0);
}

void testtestWaitSignalBroadcast()
{
	Printf((unsigned int)"\n\ni.Wait before a CV is created...");
	Wait(8,4);

	Printf((unsigned int)"\n\nii. Checking wait with out of range Lock or CV...");
	Wait(3,-4);

	Printf((unsigned int)"\n\niii. Signal before a CV is created...");
	Signal(5,6);

	Printf((unsigned int)"\n\niv. Checking Broadcast with out of range Lock or CV");
	Broadcast(2,-3);
	
	Exit(0);
}

int main()
{
	int choice = 0;
		
	Printf((unsigned int)"\nChoose one of the following:");
	Printf((unsigned int)"1.Test case for Lock Create, Lock destroy, Acquire and Release\n");
	Printf((unsigned int)"2.Test case for CreateCV , Destroy CV\n");
	Printf((unsigned int)"3.Test case for Wait, Signal and Broadcast\n");
	Printf((unsigned int)"Enter your choice:" ); 
	choice=Scanf();
	
	if(choice == 1)
	{
		Printf((unsigned int)"\nRunning Test Case 1\n");
		Fork(testLock);
		Fork(testCV);		
	}
	if(choice == 2)
	{
		Printf((unsigned int)"\nRunning Test Case 2\n");
		Fork(testCV);
		Fork(testtestWaitSignalBroadcast);
	}
	if(choice == 3)
	{
		Printf((unsigned int)"\nRunning Test Case 3\n");
		Fork(testtestWaitSignalBroadcast);
	}

}

    
    	

