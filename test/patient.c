#include "createEntities.h" 
#include "syscall.h"

int randomValue=0;

int random()
{
	return(randomValue++);	
}

 void patient()
{
	int myIndex,k;
	int patientWaitingCount;
		
	Acquire(patientIndexLock);
	myIndex=GetMV(globalPatientIndexMV,0);
	SetMV(globalPatientIndexMV,0,myIndex+1);
	Release(patientIndexLock);

	Acquire(printLock);
	Printf1((unsigned int)"Adult Patient:",myIndex);
	Printf((unsigned int)" has entered the Doctor's Office Waiting Room.\n");
	Release(printLock);

	Acquire(wrnLineLock);
	if(GetMV(wrnStatusMV,0)==BUSY)
	{
		/*If wrn is busy, enter into line*/
		/*Printf((unsigned int)" wrnStatus=>BUSY\n");*/
		Acquire(printLock); 
		Printf1((unsigned int)"Adult Patient:",myIndex);
		Printf((unsigned int)" gets in line of the Waiting Room Nurse to get registration form.\n");
		Release(printLock); 
		 		 
		patientWaitingCount=GetMV(patientWaitingCountMV,0);
		SetMV(patientWaitingCountMV,0,patientWaitingCount+1);
		
		Acquire(printLock); 
		Printf1((unsigned int)"Adult Patient:",myIndex);
		Printf((unsigned int)" waits in line to get form\n");
		Release(printLock); 
		
		Wait(patientWaitingCV,wrnLineLock);
	} 		 
	else if(GetMV(wrnStatusMV,0)==FREE)
	{
		/* If wrn is FREE, make wrnStatus = BUSY*/
		Printf1((unsigned int)"Adult Patient:",myIndex);
		Printf((unsigned int)" finds no one is in line\n");
		SetMV(wrnStatusMV,0,BUSY);
	}
	Release(wrnLineLock);	
	Acquire(wrnLock);
	SetMV(patientTaskMV,0,GETFORM);	
	SetMV(patientGettingFormMV,0,myIndex);
	Signal(wrnWaitingCV,wrnLock); 
	Release(wrnLock);
	
	/* Allocate random values of age & name to patient*/
	Acquire(infoLock);
	/*SetMV(patientES_age,myIndex,random()%100 + 1);
	SetMV(patientES_name,myIndex,random()%26 + 1);
	SetMV(patientES_pid,myIndex,myIndex);*/
	Release(infoLock);
		
	for(k=0;k<10;k=k+1) {
			Yield();
	}
	
	Acquire(wrnLineLock);
	if(GetMV(wrnStatusMV,0) == BUSY)
	{
		/*Printf((unsigned int)"wrnStatus => Busy\n");*/
		Acquire(printLock);
		Printf1((unsigned int)"Adult Patient:",myIndex);
		Printf((unsigned int)" gets in line of the Waiting Room Nurse to submit registration form.\n");
		Release(printLock);
		patientWaitingCount=GetMV(patientWaitingCountMV,0);
		SetMV(patientWaitingCountMV,0,patientWaitingCount+1);
		Wait(patientWaitingCV,wrnLineLock);
	}
	else if(GetMV(wrnStatusMV,0) == FREE)	
	{ 
		/*Printf((unsigned int)"wrnStatus=>Free\n");*/
		SetMV(wrnStatusMV,0,BUSY);
	}
	Release(wrnLineLock);
	Acquire(wrnLock);
	SetMV(patientTaskMV,0, GIVEFORM);
	SetMV(patientGivingFormMV,0, myIndex);
	Acquire(printLock);	
	Printf1((unsigned int)"Adult patient:",myIndex);
	Printf((unsigned int)" submits the filled form to the Waiting Room Nurse.\n");
	Release(printLock);	
	Signal(wrnWaitingCV,wrnLock);
	Release(wrnLock);		 	
}

void main()
{
	/* Create Entities for Patient */
	createEntities();
	
	/* Run patient code */
	patient();

	Exit(0);	
}

 
