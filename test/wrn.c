
#include "createEntities.h"
#include "syscall.h"

void waitingRoomNurse() 
{
	int patientWaitingCount=0;
	int patientTaskMV=0;
	 
	while(1)  
	{
		Acquire(wrnLineLock);
		patientWaitingCount = GetMV(patientWaitingCountMV,0);
		if(patientWaitingCount==0)
		{
			/* No patients Wrn => Free */
			/*Printf((unsigned int)"patientWaitingCount = 0\n");	*/
			SetMV(wrnStatusMV,0,FREE);
		} 		
				
		if(patientWaitingCount > 0)
		{ 
			/*Printf((unsigned int)"patientWaitingCount > 0\n");*/
			Signal(patientWaitingCV,wrnLineLock);
			patientWaitingCount =1;
			SetMV(patientWaitingCountMV,0,patientWaitingCount); 
		}
	
		Acquire(wrnLock);
		Release(wrnLineLock);
		/* Wrn Waits on interaction Lock*/
		Wait(wrnWaitingCV,wrnLock);		

		if(GetMV(patientTaskMV,0) == GETFORM)
		{
			/*Printf((unsigned int)"patientTask=>GETFORM\n");*/
			Acquire(printLock);
			Printf((unsigned int)"Waiting Room nurse gives a form to ");
			Printf1((unsigned int)" Adult patient ",GetMV(patientGettingFormMV,0));
			Printf((unsigned int)"\n");
			Release(printLock);
			Release(wrnLock);
		}	   
		if(GetMV(patientTaskMV,0) == GIVEFORM)
		{
			/*Printf((unsigned int)"\npatientTask=>GIVEFORM\n\n");*/
			Acquire(printLock);
			Printf((unsigned int)"\nWaiting Room nurse accepts the form from");
			Printf1((unsigned int) "Adult Patient ",GetMV(patientGivingFormMV,0));
			Printf1((unsigned int)"\nWaiting Room nurse tells the Adult Patient ",GetMV(patientGivingFormMV,0));
			Printf((unsigned int)" to wait in the waiting room for a nurse\n");
			Release(printLock);
			Release(wrnLock);
		}        
	}/*End of While */
	Exit(0);
}	
	
	
void main()
{ 
	/* Create all Entities for WRN */
	createEntities();  
	setEntities();
	
	/* Run Wrn Code */
	Printf((unsigned int)"Starting Waiting Room Nurse:\n");
	waitingRoomNurse();
	
	Exit(0);
}
