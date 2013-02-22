
#include "syscall.h"


/***********************************************
DEFINING TOTAL NUMBER OF WORKERS IN SIMULATION
************************************************/
	#define PATIENTS 5
	#define NURSES 3
	#define ROOMS 2
	#define DOCTORS 2
	#define XRAYTECH 2


/*Declaration of struct*/
struct ExamSheet
{
	int pid;
	char *name;
	int age;
	char *symptom;
	int status;
	int room;
	int no_of_images;
	char *treatment;
	int examiningDoctor;
	int servingNurse;
	int xRayRoom;
	char *xRayResult;
	int type;
}wrnExamSheet,patientExamSheet[100],myExamSheet[10],wallPocket[5],xRayWaitingRoomWallPocket[100],examSheetDocCopy[5],examSheetXrayCopy[2],examSheetCashierCopy;

#define FREE 0
#define BUSY 1
#define GETFORM 2
#define GIVEFORM 3
#define SERVE_PATIENT 4
#define SERVE_NURSE 5
#define SERVE_NEITHER 6
#define FETCH_PATIENT 7
#define WAITING_FOR_DOC 8
#define NEED_NURSE 9
#define DONE 10
#define OCCUPIED 11
#define FIND_EXAM_ROOM 12
#define WAIT_FOR_NURSE 13
#define WITH_NURSE 14
#define WAITING_FOR_DOC_AGAIN 15
#define NEED_NURSE_AGAIN 16
#define FIRST_TIME 17
#define SECOND_TIME 18

#ifndef NULL
	#define NULL 0L
#endif

/*-------------------------------Locks-------------------------------------------*/

int wrnLineLock;
int wrnLock;
int infoLock;

int waitingRoomLock;
int taskLock;
int examRoomLock[50];

int docLineLock;
int needNurseLock;
int tempRoomLock[5];
int tempLock[5];
int newTempLock;

int xRayLineLock;
int xRayInteractionLock[2];
int xRayWaitingRoomLock;

int cashierLineLock;
int cashierInteractionLock;
int wallPocketLock;
int leaveLock;

int nurseIndexLock;
int patientIndexLock;
int doctorIndexLock;
int xrayIndexLock;
int printlock;
int ran=500;
int count=0;
/*------------------------------Conditions--------------------------------------------------------*/
int patientWaitingCV;
int wrnWaitingCV;
int wrnWaitingNurseCV;
int wrnWaitingPatientCV;
int wrnWaitingAgainPatientCV;
int nurseWaitingCV;
int waitingRoomCV;
int examRoomCV[50];
int waitingRoomNurseCV;
int nurseLineCV[5];
int docLineCV[3];
int tempCV[5];

int xRayLineCV[2];
int xRayInteractionCV[2];
int xRayInteractionPatCV[2];
int xRayInteractionNurseCV[2];
int xRayWaitingRoomCV;
int xRayWaitingRoomNurseCV;

int cashierLineCV;
int cashierInteractionCV;

char randomChar();
int randomNumber();
void waitingRoomNurse();
void patient(void);
void nurse(void);
void doctor(void);
void xRayTechnician(void);
void cashier();

int randomNumber() {
	int num;
	num=9999999;
	num=num/ran;
	return num;
}
char randomChar() {
	char randomCharacter;
	int randomNum = randomNumber()%26;
	randomNum += 97;
	randomCharacter = (char)randomNum;
	return randomCharacter;
}
 /*-------------------------------------------------Variables---------------------------------*/
int patientPresent=0;
int nurseWaitingCount=0;
int wrnStatus=-1;
int wrnTaskStatus=SERVE_NEITHER;
int patientTask=-1;
int nurseTask=-1;
int roomList[50];
int currentNurse=-1;
int currentNurse_dLL=-1;
int currentPatient=-1;
int currentPatient_erl=-1;
int currentPatient_dLL=-1;
int currentRoom=-1;
int currentDoc[5];
int currentRoom_erl=-1;
int currentSymptom[100];
int currentNurse_wrl=-1;
int patientWaitingCount=0;
int patientGettingForm=-1;
int patientGivingForm=-1;
int registeredPatientCount=0;


int examRoomStatus[50];
int docCount=0;
int docStatus[5];
int counter=0;
int nursePatCount=0;
int patList[5];
int leavingPatients=0;
int no_of_xRayTechnicians;
int nurseWaitingForXrayCount[2];
int tempXrayRoom=-1;
int xRayPatientList[2];
int xRayStatus[2];
int nurseStatus[5];
int cashierLine[100];
int position=0;
int myPosition=0;
int cashierStatus=FREE;
int patientCashierWaitingCount=0;
int waitingRoomCnt=0;
int enteringPatientCnt=0;
int xRayRoomCnt_atNurse=0;
int xRayRoomCnt_atXray=0;
int patientWaitingForXrayCount[2];

int globalNurseIndex=0;
int globalPatientIndex=0;
int globalDoctorIndex=0;
int globalXrayIndex=0;

void waitingRoomNurse() {
	int patientList[50],p,a;
	p=0;
	for(a=0;a<50;a=a+1)
	{
		patientList[a]=-1;
	}
/*	printf("\n\nEntered WRN");*/
	/* Local storage for patient id's to give to nurse */
	while(1)
	{	
		/*Acquire lock for interacting with the line */		
		Acquire(wrnLineLock);
		if(nurseWaitingCount == 0 && patientWaitingCount == 0)
		{
			 wrnStatus=FREE;
			 wrnTaskStatus = SERVE_NEITHER;
		}
		if(patientWaitingCount > 0)
		{
			Signal(patientWaitingCV,wrnLineLock);
			patientWaitingCount--;
			wrnTaskStatus=SERVE_PATIENT;
			/*Printf("\n\n\n\nBefore goto ---------------------------------------------------");*/
			if(patientWaitingCount == 0)
			{
				/* //Printf("\n\n\n\nBefore goto in if---------------------------------------------------"); */
				goto  xyz;
				/* //Printf("\n\n\n\nAfter goto in if---------------------------------------------------"); */
			}
		}
		/* //Printf("\n\n\n\nAfter goto ---------------------------------------------------"); */
		if(nurseWaitingCount > 0 && wrnTaskStatus != SERVE_PATIENT)
		{
			Signal(nurseWaitingCV,wrnLineLock);
			nurseWaitingCount--;
			wrnTaskStatus=SERVE_NURSE;				
		}
		else if(nurseWaitingCount > 0 && patientWaitingCount == 0)
		{
			Signal(nurseWaitingCV,wrnLineLock);
			nurseWaitingCount--;
			wrnTaskStatus=SERVE_NURSE;				
		}
		/*Printf("\n\n\n\nBefore destination---------------------------------------------------");*/
xyz:		Acquire(wrnLock);
		/*Printf("\n\n\n\nAfter destination---------------------------------------------------");*/
		Release(wrnLineLock);
		Wait(wrnWaitingCV,wrnLock);			
		
		if(wrnTaskStatus == SERVE_PATIENT)
		{
			/*Check task to do with patient form. After receiving Signal from Patient */
			if(patientTask==GETFORM)
			{
					/*Give the patient a new Blank Exam sheet*/
					wrnExamSheet.name = "BLANK";
					wrnExamSheet.age = -1;		/*Age = -1 indicates a blank field*/
					Acquire(printlock);
					Printf((unsigned int)"\nWRN:Waiting Room nurse gives a form to ");
					Printf1((unsigned int)" Adult patient ",patientGettingForm);
					Release(printlock);
					/* Take the Form & Leave the Line. So that i can become free again */
					Signal(wrnWaitingPatientCV,wrnLock);
					Release(wrnLock);
			}
			if(patientTask==GIVEFORM)
			{
				Acquire(printlock);
				Printf((unsigned int)"\nWaiting Room nurse accepts the form from");
				Printf1((unsigned int) "Adult Patient ",patientGivingForm);
				Printf2((unsigned int)" with name ",(unsigned int)patientExamSheet[patientGivingForm].name);
				Printf1((unsigned int)" and age ",patientExamSheet[patientGivingForm].age);
				Printf1((unsigned int)"\nWaiting Room nurse creates an examination sheet for [Adult] patient ",patientGivingForm);
				Printf2((unsigned int)" with name ",(unsigned int)patientExamSheet[patientGivingForm].name);
				Printf1((unsigned int)" and age ",patientExamSheet[patientGivingForm].age);
				Printf1((unsigned int)"\nWaiting Room nurse tells the Adult Patient ",patientGivingForm);
				Printf((unsigned int)" to wait in the waiting room for a nurse");
				Release(printlock);
				registeredPatientCount=registeredPatientCount+1;
				patientList[p]=patientGivingForm;
				p=p+1;
				Signal(wrnWaitingAgainPatientCV,wrnLock);
				Release(wrnLock);
			}
		}
		if(wrnTaskStatus == SERVE_NURSE)
		{
			if(nurseTask == FETCH_PATIENT)
			{
				int m;
				for(m=0;m<p;m=m+1)
					if(patientList[m]!=-1)
					{
						int patient=patientList[m];
						patientList[m]=-1;
						Acquire(printlock);
						Printf((unsigned int)"\nWaiting Room nurse gives examination sheet of ");
						Printf1((unsigned int)"patient",patient);
						Printf1((unsigned int)" to Nurse ",currentNurse);
						Release(printlock);
						Signal(wrnWaitingNurseCV,wrnLock);
						break;
					}
				}
				Release(wrnLock);
				ran=ran-1;
			}
		}
		Exit(0);
	}
	


void patient(void) {
	int myIndex,k1,k2;
	char temp[10];		/*Temporary array used to initialize name*/
	Acquire(patientIndexLock);
	myIndex=globalPatientIndex;
	globalPatientIndex=globalPatientIndex+1;
	Release(patientIndexLock);
	
	Acquire(printlock);
	Printf1((unsigned int)"\nAdult Patient: ",myIndex);
	Printf((unsigned int)" has entered the Doctor's Office Waiting Room.");
	Release(printlock);
	/*patientExamSheet[i] = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));*/
		
	Acquire(wrnLineLock);
	if(wrnStatus == BUSY) {
		/*If WRN is busy,patient enters line*/
		Acquire(printlock);
		Printf1((unsigned int)"\nAdult Patient:",myIndex);
		Printf((unsigned int)" gets in line of the Waiting Room Nurse to get registration form.");
		Release(printlock);
		patientWaitingCount=patientWaitingCount+1;
		Wait(patientWaitingCV,wrnLineLock);
	}
	else if(wrnStatus == FREE) {
		wrnStatus = BUSY;
	}

	/* Patient was waiting in line for wrn. When wrn signals him, he has to come out of line.*/
	/*enteringPatientCnt++;*/
	Release(wrnLineLock);
	
	/* Interaction with wrn begins */
	Acquire(wrnLock);
	wrnTaskStatus=SERVE_PATIENT;
	patientTask = GETFORM;
	patientGettingForm=myIndex;
	/* Tell the wrn that i have acquired the form */
	Signal(wrnWaitingCV,wrnLock);
	Wait(wrnWaitingPatientCV,wrnLock);
	/* After Receiving Blank sheet from wrn */
	Release(wrnLock);

	/* After Getting out of Line. Patient Fills out the form */
	Acquire(infoLock);
	
	/* Allocate random values of age & name to patient*/

/* 	srand(time(0));
 */	
	patientExamSheet[myIndex].age = randomNumber()%100 + 1;
	
	 for(k1=0;k1<10;k1=k1+1) {
	        temp[k1] = randomChar();  
        }
        patientExamSheet[myIndex].name = temp;
        patientExamSheet[myIndex].pid = myIndex;
	patientExamSheet[myIndex].room = -1;
	patientExamSheet[myIndex].status = -1;
        
        Release(infoLock);
        /* Give some delay for patient to enter his info */ 
        for(k2=0;k2<10;k2=k2+1) {
        	Yield();
        }
        /* Patient Re-enters the waiting Queue */
        Acquire(wrnLineLock);
        
	if(wrnStatus == BUSY) {
		Acquire(printlock);
		Printf1((unsigned int)"\nAdult Patient:",myIndex);
		Printf((unsigned int)" gets in line of the Waiting Room Nurse to submit registration form.");
		Release(printlock);
		patientWaitingCount=patientWaitingCount+1;
		Wait(patientWaitingCV,wrnLineLock);
	}
	else if(wrnStatus == FREE)	{
		wrnStatus = BUSY;
	}
	
	/* Patient was waiting in line for wrn. When wrn signals him, he has to come out of line.*/
	Release(wrnLineLock);
	
	/* Interacts with Wrn again */
	Acquire(wrnLock);
	wrnTaskStatus=SERVE_PATIENT;
	patientTask=GIVEFORM;
	patientGivingForm=myIndex;

        /* Tell wrn that i have to submit the form*/
	Acquire(printlock);	
	Printf1((unsigned int)"\nAdult patient ",myIndex);
	Printf((unsigned int)" submits the filled form to the Waiting Room Nurse.");
	Release(printlock);	
	Signal(wrnWaitingCV,wrnLock);
	Wait(wrnWaitingAgainPatientCV,wrnLock);
		
	/*Now patient has to wait in waiting Room. He acquires Waiting for Nurse Lock & Waits on it */

	Acquire(waitingRoomLock);
	waitingRoomCnt=waitingRoomCnt+1;
	Release(wrnLock);	
	Wait(waitingRoomCV,waitingRoomLock);
	
	/* Patient Nurse Interaction Begins */

	currentPatient = myIndex;

	Signal(waitingRoomNurseCV,waitingRoomLock);
	Wait(waitingRoomNurseCV,waitingRoomLock);
	Acquire(printlock);
	Printf1((unsigned int)"\nAdult Patient/Parent ",myIndex);
	Printf1((unsigned int)" is following Nurse ", patientExamSheet[myIndex].servingNurse);
	Printf1((unsigned int)" to Examination Room ",patientExamSheet[myIndex].room);	
	Release(printlock);
	Signal(waitingRoomNurseCV,waitingRoomLock);
	Wait(waitingRoomNurseCV,waitingRoomLock);
	
	Acquire(examRoomLock[patientExamSheet[myIndex].room]);
	Release(waitingRoomLock);

	Signal(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
	Wait(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
	
	/*srand(time(0));*/
	currentSymptom[myIndex] = randomNumber()%3;	/*Assign Random Symptoms to Patient*/
	if(currentSymptom[myIndex]==0)
	{
		Acquire(printlock);
		Printf1((unsigned int)"\nAdult Patient/Parent ",myIndex);
		Printf((unsigned int)" says, \"My symptoms are Nausea.\"");
		Release(printlock);
	}
	else if(currentSymptom[myIndex]==1)
	{
		Acquire(printlock);
		Printf1((unsigned int)"\nAdult Patient/Parent ",myIndex);
		Printf((unsigned int)" says, \"My symptoms are Pain.\"");
		Release(printlock);
	}
	else if(currentSymptom[myIndex]==2)
	{
		Acquire(printlock);
		Printf1((unsigned int)"\nAdult Patient/Parent ",myIndex);
		Printf((unsigned int)" says, \"My symptoms are I Hear Alien Voices.\"");
		Release(printlock);
	}
	Signal(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
	Wait(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
	
	/*	leavingPatients=leavingPatients+1;*/
	Signal(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
	Wait(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
	
	if(patientExamSheet[myIndex].treatment[0]=='X')
	{
		Acquire(printlock);
		Printf1((unsigned int)"\nAdult Patient ",myIndex);
		Printf1((unsigned int)" has been informed by Doctor ", patientExamSheet[myIndex].examiningDoctor);
		Printf((unsigned int)" that he needs an Xray.");
		Release(printlock);
	}
	else if(patientExamSheet[myIndex].treatment[0]=='S')
	{
		Acquire(printlock);
		Printf1((unsigned int)"\nAdult Patient ",myIndex);
		Printf1((unsigned int)" has been informed by Doctor ", patientExamSheet[myIndex].examiningDoctor);
		Printf((unsigned int)" that he will be administered a shot.");
		Release(printlock);
	}
	else if(patientExamSheet[myIndex].treatment[0]=='F')
	{
		Acquire(printlock);
		Printf1((unsigned int)"\nAdult Patient ",myIndex);
		Printf1((unsigned int)" has been diagnosed by Doctor ", patientExamSheet[myIndex].examiningDoctor);
		Release(printlock);
	}
	
	Signal(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
	Wait(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
	

	Signal(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
	Wait(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
	/* Wait for Nurse to come see patient */
	
	if(patientExamSheet[myIndex].treatment[0]=='X')
	{
		Acquire(printlock);
		Printf1((unsigned int)"\nAdult Patient ",myIndex);
		Printf((unsigned int)" waits for a Nurse to escort them to the Xray room.");
		Release(printlock);

		Signal(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
		Wait(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
		
		Acquire(xRayLineLock);
		Signal(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
		Release(examRoomLock[patientExamSheet[myIndex].room]);
		
		if(xRayStatus[patientExamSheet[myIndex].xRayRoom] == BUSY)
		{
			patientWaitingForXrayCount[patientExamSheet[myIndex].xRayRoom]=patientWaitingForXrayCount[patientExamSheet[myIndex].xRayRoom]+1;
			Wait(xRayLineCV[patientExamSheet[myIndex].xRayRoom],xRayLineLock);
		}
		else if(xRayStatus[patientExamSheet[myIndex].xRayRoom] == FREE)
		{
			xRayStatus[patientExamSheet[myIndex].xRayRoom] = BUSY;
		}
		Acquire(xRayInteractionLock[patientExamSheet[myIndex].xRayRoom]);
		Release(xRayLineLock);
		xRayPatientList[patientExamSheet[myIndex].xRayRoom] = myIndex;

	
		Signal(xRayInteractionCV[patientExamSheet[myIndex].xRayRoom],xRayInteractionLock[patientExamSheet[myIndex].xRayRoom]);
		Wait(xRayInteractionCV[patientExamSheet[myIndex].xRayRoom],xRayInteractionLock[patientExamSheet[myIndex].xRayRoom]);
							
		Acquire(xRayWaitingRoomLock);
		Acquire(printlock);
		Printf1((unsigned int)"\nAdult Patient ",myIndex);
		Printf1((unsigned int)" waits for a Nurse to escort him/her to examination room\n",patientExamSheet[myIndex].room);
		Release(printlock);
		
		patientPresent=patientPresent+1;
		Signal(xRayInteractionCV[patientExamSheet[myIndex].xRayRoom],xRayInteractionLock[patientExamSheet[myIndex].xRayRoom]);
		Release(xRayInteractionLock[patientExamSheet[myIndex].xRayRoom]);
		Wait(xRayWaitingRoomCV,xRayWaitingRoomLock);
		
		Acquire(printlock);
		Printf1((unsigned int)"\n[Adult Patient] ",myIndex);
		Printf1((unsigned int)" is following Nurse ", patientExamSheet[myIndex].servingNurse);
		Printf1((unsigned int)" to Examination Room ",patientExamSheet[myIndex].room);
		Release(printlock);
		Signal(xRayWaitingRoomNurseCV,xRayWaitingRoomLock);
		Acquire(examRoomLock[patientExamSheet[myIndex].room]);
		Release(xRayWaitingRoomLock);
		
		Wait(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);	
	}
	else if(patientExamSheet[myIndex].treatment[0]=='S')
	{
		Acquire(printlock);
		Printf1((unsigned int)"\nAdult Patient ",myIndex);
		Printf((unsigned int)" says, \"Yes I am ready for the shot\".");
		Release(printlock);
		Signal(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
		Wait(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);				
	}
	else if(patientExamSheet[myIndex].treatment[0]=='F')
	{
	}
	Signal(examRoomCV[patientExamSheet[myIndex].room],examRoomLock[patientExamSheet[myIndex].room]);
	Acquire(printlock);
	Printf1((unsigned int)"\nAdult Patient ",myIndex);
	Printf((unsigned int)" enters the queue for Cashier");
	Release(printlock);
	
	Acquire(cashierLineLock);
	Release(examRoomLock[patientExamSheet[myIndex].room]);
	if(cashierStatus == BUSY) {
		/*If Cashier is busy,patient enters line*/
		patientCashierWaitingCount=patientCashierWaitingCount+1;
		Wait(cashierLineCV,cashierLineLock);
	}
	else if(cashierStatus == FREE) {
		cashierStatus = BUSY;
	}
	Release(cashierLineLock);
	Acquire(cashierInteractionLock);
	Acquire(printlock);
	Printf1((unsigned int)"\nAdult Patient ",myIndex);
	Printf((unsigned int)" reaches the Cashier.");
	Printf1((unsigned int)"\nAdult Patient ",myIndex);
	Printf((unsigned int)" hands over his examination sheet to the Cashier.");
	Release(printlock);
	cashierLine[position]=myIndex;
	position=position+1;
	Signal(cashierInteractionCV,cashierInteractionLock);
	Wait(cashierInteractionCV,cashierInteractionLock);
	
	Acquire(printlock);
	Printf1((unsigned int)"\nAdult Patient ",myIndex);
	Printf((unsigned int)" pays the Cashier $.... ");
	Release(printlock);
	
	Signal(cashierInteractionCV,cashierInteractionLock);
	Wait(cashierInteractionCV,cashierInteractionLock);
	
	Acquire(printlock);
	Printf1((unsigned int)"\nAdult Patient ",myIndex);
	Printf((unsigned int)" receives a receipt from the Cashier.\n");
	Printf1((unsigned int)"Adult Patient ",myIndex);
	Printf((unsigned int)" leaves the doctor's office.\n");
	Release(printlock);

	Signal(cashierInteractionCV,cashierInteractionLock);
	Acquire(leaveLock);
	leavingPatients=leavingPatients+1;
	Release(leaveLock);
	Release(cashierInteractionLock);
	ran=ran-1;
	Exit(0);
}

void nurse(void)
{
	int roomFound=0,roomAssigned=0;
	int myIndex;
	Acquire(nurseIndexLock);
	myIndex=globalNurseIndex;
	globalNurseIndex=globalNurseIndex+1;
	Release(nurseIndexLock);
	/*myExamSheet[myIndex]=malloc(sizeof(struct ExamSheet));*/
	while(1)
	{	
		int y,k,t,x;
		roomFound=0;
		roomAssigned=0;
		/*myExamSheet[myIndex].room = -1;*/
		myExamSheet[myIndex].status = FREE;
		
		for(y=0;y<15;y=y+1)
		{
			Yield();
		}
		
		/* Search for an Empty Room*/
		Acquire(taskLock);
			
		for(k=0;k<ROOMS;k=k+1)
		{
			if(examRoomStatus[k] == FREE)
			{
			/*	Acquire(printlock);
				Printf1((unsigned int)"\n Nurse ",myIndex);
				Printf1((unsigned int)" gets free room ",k);
				Release(printlock);*/
				examRoomStatus[k]=BUSY;
				roomList[myIndex]=k;	/*j-th nurse makes k-th room busy*/
				myExamSheet[myIndex].room = k;
				roomFound=1;
				break;
			}
		}
		Release(taskLock);
		
		/* If free Room Found & Patients Present, fetch one*/
		Acquire(wrnLineLock);
		if(roomFound == 1 && enteringPatientCnt <= PATIENTS)
		{
			enteringPatientCnt=enteringPatientCnt+1;
		/*	printf("\n\nEPC:%myIndex",enteringPatientCnt);*/
/*			Acquire(printlock);
			Printf1((unsigned int)"\n Nurse ",myIndex);
			Printf((unsigned int)" waits on wrn");
			Release(printlock);*/
		/* Talk to WRN after finding free room */
			if(wrnStatus == BUSY) {
			/* Nurse Getting in Line*/
				nurseWaitingCount=nurseWaitingCount+1;
				Wait(nurseWaitingCV,wrnLineLock);
			}
			else if(wrnStatus == FREE) {
				wrnStatus = BUSY;
			}
				
			/* Nurse was waiting in line for wrn. When wrn signals her, she has to come out of line.*/
			Release(wrnLineLock);
			Acquire(wrnLock);
			if(waitingRoomCnt > 0 )
			{
				waitingRoomCnt--;
				currentNurse = myIndex;
				wrnTaskStatus=SERVE_NURSE;
				nurseTask = FETCH_PATIENT;
				Signal(wrnWaitingCV,wrnLock);
				Acquire(printlock);
				Printf1((unsigned int)"\nNurse ",myIndex);
				Printf((unsigned int)" tells Waiting Room Nurse to give a new examination sheet.");
				Release(printlock);
				Wait(wrnWaitingNurseCV,wrnLock);
			
				Acquire(waitingRoomLock);
			
				Signal(waitingRoomCV,waitingRoomLock);
				Wait(waitingRoomNurseCV,waitingRoomLock);
				Acquire(printlock);
				Printf1((unsigned int)"\nNurse ",myIndex); 
				Printf1((unsigned int)" escorts Adult Patient/Parent ",currentPatient);
				Printf1((unsigned int)" to the examination room ",roomList[myIndex]);
				Release(printlock);
				patientExamSheet[currentPatient].room = roomList[myIndex];
				patientExamSheet[currentPatient].servingNurse = myIndex;
				patientExamSheet[currentPatient].room = roomList[myIndex];
				myExamSheet[myIndex].pid=patientExamSheet[currentPatient].pid;
				myExamSheet[myIndex].age=patientExamSheet[currentPatient].age;
				myExamSheet[myIndex].name=patientExamSheet[currentPatient].name;
				myExamSheet[myIndex].status=patientExamSheet[currentPatient].status;
				myExamSheet[myIndex].room=patientExamSheet[currentPatient].room;
				myExamSheet[myIndex].examiningDoctor=patientExamSheet[currentPatient].examiningDoctor;
				myExamSheet[myIndex].servingNurse=patientExamSheet[currentPatient].servingNurse;
				myExamSheet[myIndex].xRayRoom=patientExamSheet[currentPatient].xRayRoom;
				myExamSheet[myIndex].treatment =patientExamSheet[currentPatient].treatment;
				myExamSheet[myIndex].type=patientExamSheet[currentPatient].type;
				
				myExamSheet[myIndex].room = roomList[myIndex];
				
				Signal(waitingRoomNurseCV,waitingRoomLock);
				Wait(waitingRoomNurseCV,waitingRoomLock);
	
				Acquire(examRoomLock[roomList[myIndex]]);
				roomAssigned=1;
				Signal(waitingRoomNurseCV,waitingRoomLock);
				Release(waitingRoomLock);
				Release(wrnLock);				
				
				Wait(examRoomCV[roomList[myIndex]],examRoomLock[roomList[myIndex]]);
				Acquire(printlock);
				Printf1((unsigned int)"\nNurse: ",myIndex);
				Printf1((unsigned int)" takes the temperature and blood pressure of Patient:",myExamSheet[myIndex].pid);
				Printf1((unsigned int)"\nNurse: ",myIndex);
				Printf1((unsigned int)" asks Patient:",myExamSheet[myIndex].pid);
				Printf((unsigned int)"\"What Symptoms do you have?\"");
				Release(printlock);
				
				Signal(examRoomCV[roomList[myIndex]],examRoomLock[roomList[myIndex]]);
				Wait(examRoomCV[roomList[myIndex]],examRoomLock[roomList[myIndex]]);
				
				if(currentSymptom[myExamSheet[myIndex].pid] == 0)
				{
					myExamSheet[myIndex].symptom="Nausea";
					patientExamSheet[myExamSheet[myIndex].pid].symptom="Nausea";
				}
				else if(currentSymptom[myExamSheet[myIndex].pid] == 1)
				{
					myExamSheet[myIndex].symptom="Pain";
					patientExamSheet[myExamSheet[myIndex].pid].symptom="Pain";
				}
				else
				{
					myExamSheet[myIndex].symptom="I hear alien voices";
					patientExamSheet[myExamSheet[myIndex].pid].symptom="I hear alien voices";
				}

				
				myExamSheet[myIndex].status = WAITING_FOR_DOC;
				patientExamSheet[myExamSheet[myIndex].pid].pid=myExamSheet[myIndex].pid;
				patientExamSheet[myExamSheet[myIndex].pid].age=myExamSheet[myIndex].age;
				patientExamSheet[myExamSheet[myIndex].pid].name=myExamSheet[myIndex].name;
				patientExamSheet[myExamSheet[myIndex].pid].status=myExamSheet[myIndex].status;
				patientExamSheet[myExamSheet[myIndex].pid].room=myExamSheet[myIndex].room;
				patientExamSheet[myExamSheet[myIndex].pid].examiningDoctor=myExamSheet[myIndex].examiningDoctor;
				patientExamSheet[myExamSheet[myIndex].pid].servingNurse=myExamSheet[myIndex].servingNurse;
				patientExamSheet[myExamSheet[myIndex].pid].xRayRoom=myExamSheet[myIndex].xRayRoom;
				patientExamSheet[myExamSheet[myIndex].pid].treatment=myExamSheet[myIndex].treatment;
				patientExamSheet[myExamSheet[myIndex].pid].type=myExamSheet[myIndex].type;
								
				Signal(examRoomCV[roomList[myIndex]],examRoomLock[roomList[myIndex]]);
				Wait(examRoomCV[roomList[myIndex]],examRoomLock[roomList[myIndex]]);
				
				Acquire(wallPocketLock);	
				wallPocket[roomList[myIndex]].pid=myExamSheet[myIndex].pid;
				wallPocket[roomList[myIndex]].age=myExamSheet[myIndex].age;
				wallPocket[roomList[myIndex]].name=myExamSheet[myIndex].name;
				wallPocket[roomList[myIndex]].status=myExamSheet[myIndex].status;
				wallPocket[roomList[myIndex]].room=myExamSheet[myIndex].room;
				wallPocket[roomList[myIndex]].examiningDoctor=myExamSheet[myIndex].examiningDoctor;
				wallPocket[roomList[myIndex]].servingNurse=myExamSheet[myIndex].servingNurse;
				wallPocket[roomList[myIndex]].xRayRoom=myExamSheet[myIndex].xRayRoom;
				wallPocket[roomList[myIndex]].treatment=myExamSheet[myIndex].treatment;
				wallPocket[roomList[myIndex]].type=myExamSheet[myIndex].type;				
				Release(wallPocketLock); 
				/*printf("Nurse %d releasing eRLock 1st loop",j);*/
				Release(examRoomLock[roomList[myIndex]]);
			}
			else
			{
				Release(wrnLock);
			}
		}
		else
		{
			Release(wrnLineLock);
		}
		
		
		Acquire(wallPocketLock);
		for(t=0;t<ROOMS;t=t+1)
		{
			if(wallPocket[t].status==WAITING_FOR_DOC || wallPocket[t].status == WAITING_FOR_DOC_AGAIN || wallPocket[t].status == NEED_NURSE || 	wallPocket[t].status == NEED_NURSE_AGAIN)
			{
				/* Make Earlier Room Free*/
				Acquire(taskLock);
				if(roomAssigned == 0 && roomFound == 1)
				{
					examRoomStatus[roomList[myIndex]]= FREE;
				}
				Release(taskLock);
				roomList[myIndex]=t;
				
				myExamSheet[myIndex].pid=wallPocket[t].pid;
				myExamSheet[myIndex].age=wallPocket[t].age;
				myExamSheet[myIndex].name=wallPocket[t].name;
				myExamSheet[myIndex].status=wallPocket[t].status;
				myExamSheet[myIndex].room=wallPocket[t].room;
				myExamSheet[myIndex].examiningDoctor=wallPocket[t].examiningDoctor;
				myExamSheet[myIndex].servingNurse=wallPocket[t].servingNurse;
				myExamSheet[myIndex].xRayRoom=wallPocket[t].xRayRoom;
				myExamSheet[myIndex].treatment =wallPocket[t].treatment;
				myExamSheet[myIndex].type =wallPocket[t].type;
				
				wallPocket[t].status = OCCUPIED;
				myExamSheet[myIndex].room=roomList[myIndex];
				break;
			}
			else if(xRayWaitingRoomWallPocket[xRayRoomCnt_atNurse].status == WAIT_FOR_NURSE && roomAssigned == 0 && roomFound==1)
			{
				myExamSheet[myIndex].pid=xRayWaitingRoomWallPocket[xRayRoomCnt_atNurse].pid;
				myExamSheet[myIndex].age=xRayWaitingRoomWallPocket[xRayRoomCnt_atNurse].age;
				myExamSheet[myIndex].name=xRayWaitingRoomWallPocket[xRayRoomCnt_atNurse].name;
				myExamSheet[myIndex].status=xRayWaitingRoomWallPocket[xRayRoomCnt_atNurse].status;
				myExamSheet[myIndex].examiningDoctor=xRayWaitingRoomWallPocket[xRayRoomCnt_atNurse].examiningDoctor;
				myExamSheet[myIndex].servingNurse=xRayWaitingRoomWallPocket[xRayRoomCnt_atNurse].servingNurse;	
				myExamSheet[myIndex].xRayRoom=xRayWaitingRoomWallPocket[xRayRoomCnt_atNurse].xRayRoom;
				myExamSheet[myIndex].treatment =xRayWaitingRoomWallPocket[xRayRoomCnt_atNurse].treatment;
				myExamSheet[myIndex].type =xRayWaitingRoomWallPocket[xRayRoomCnt_atNurse].type;
				myExamSheet[myIndex].room = roomList[myIndex];
			
				xRayWaitingRoomWallPocket[xRayRoomCnt_atNurse].status = FREE;
				/*Printf1((unsigned int)"ESStatus \n",myExamSheet[myIndex].status);*/
				Printf1((unsigned int)"PP\n :",patientPresent);
				xRayRoomCnt_atNurse = xRayRoomCnt_atNurse+1;
				
				break;
			}
		}
		Release(wallPocketLock);
		
		Acquire(docLineLock);
		if(myExamSheet[myIndex].status == WAITING_FOR_DOC || myExamSheet[myIndex].status == WAITING_FOR_DOC_AGAIN)
		{
		/*	Printf1((unsigned int)"Patient \n",myExamSheet[myIndex].pid);*/
			if(docCount>0)
			{
				for(x=0;x<DOCTORS;x=x+1)
				{
					if(docStatus[x]==FREE)
					{
						currentDoc[myIndex]=x;
						docStatus[x]=BUSY;
						break;
					}
				}
				if(myExamSheet[myIndex].type == 0)
				{
					Acquire(printlock);
					Printf1((unsigned int)" \nNurse ",myIndex);
					Printf1((unsigned int)" informs Doctor ",currentDoc[myIndex]);
					Printf1((unsigned int)" that Adult Patient ",myExamSheet[myIndex].pid);
					Printf1((unsigned int)" is waiting in the examination room",roomList[myIndex]);
					Release(printlock);
				}
				else
				{
					Acquire(printlock);
					Printf1((unsigned int)" \nNurse ",myIndex);
					Printf1((unsigned int)" informs Doctor ",currentDoc[myIndex]);
					Printf1((unsigned int)" that Child Patient ",myExamSheet[myIndex].pid-PATIENTS);
					Printf1((unsigned int)" is waiting in the examination room ",roomList[myIndex]);
					Release(printlock);
				}
				myExamSheet[myIndex].servingNurse=myIndex;
				patientExamSheet[myExamSheet[myIndex].pid].servingNurse = myIndex;
				patList[currentDoc[myIndex]]=myExamSheet[myIndex].pid;
			
				if(myExamSheet[myIndex].type == 0)
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nNurse ",myIndex);
					Printf1((unsigned int)" hands over to the Doctor ",currentDoc[myIndex]);
					Printf1((unsigned int)" the examination sheet of Adult Patient ",myExamSheet[myIndex].pid);
					Release(printlock);
				}
				else
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nNurse ",myIndex);
					Printf1((unsigned int)" hands over to the Doctor ",currentDoc[myIndex]);
					Printf1((unsigned int)" the examination sheet of Child Patient ",myExamSheet[myIndex].pid-PATIENTS);
					Release(printlock);
				}
				if(myExamSheet[myIndex].status == WAITING_FOR_DOC)
				{
					patientExamSheet[myExamSheet[myIndex].pid].status = FIRST_TIME;
				}
				else if(myExamSheet[myIndex].status == WAITING_FOR_DOC_AGAIN)
				{
					patientExamSheet[myExamSheet[myIndex].pid].status = SECOND_TIME;
				}
				myExamSheet[myIndex].status = OCCUPIED;
				Signal(docLineCV[currentDoc[myIndex]],docLineLock);
				docCount=docCount-1;				
				Wait(nurseLineCV[myIndex],docLineLock);
			}
			else
			{
				Acquire(wallPocketLock);
				if(wallPocket[myExamSheet[myIndex].room].status == OCCUPIED) 
				{
					wallPocket[myExamSheet[myIndex].room].pid=myExamSheet[myIndex].pid;
					wallPocket[myExamSheet[myIndex].room].age=myExamSheet[myIndex].age;
					wallPocket[myExamSheet[myIndex].room].name=myExamSheet[myIndex].name;
					wallPocket[myExamSheet[myIndex].room].status=myExamSheet[myIndex].status;
					wallPocket[myExamSheet[myIndex].room].room=myExamSheet[myIndex].room;
					wallPocket[myExamSheet[myIndex].room].examiningDoctor=myExamSheet[myIndex].examiningDoctor;
					wallPocket[myExamSheet[myIndex].room].servingNurse=myExamSheet[myIndex].servingNurse;
					wallPocket[myExamSheet[myIndex].room].xRayRoom=myExamSheet[myIndex].xRayRoom;
					wallPocket[myExamSheet[myIndex].room].treatment=myExamSheet[myIndex].treatment;
					wallPocket[myExamSheet[myIndex].room].type=myExamSheet[myIndex].type;
				}
				Release(wallPocketLock);
			}
		}
		Release(docLineLock);
		
		for(t=0;t<15;t=t+1)
		{
			Yield();
		}
		
		if(roomList[myIndex] != -1)
		{
			Acquire(examRoomLock[roomList[myIndex]]);		
			if(myExamSheet[myIndex].status == NEED_NURSE)
			{
				myExamSheet[myIndex].status=OCCUPIED;
				myExamSheet[myIndex].servingNurse = myIndex;
				patientExamSheet[myExamSheet[myIndex].pid].servingNurse = myIndex;
				patientExamSheet[myExamSheet[myIndex].pid].room = roomList[myIndex];
			
				if(myExamSheet[myIndex].treatment[0]=='X')
				{
					Signal(examRoomCV[myExamSheet[myIndex].room],examRoomLock[myExamSheet[myIndex].room]);
					Wait(examRoomCV[roomList[myIndex]],examRoomLock[roomList[myIndex]]);
			
					Acquire(xRayLineLock);
					if(XRAYTECH == 2)
					{
						if(xRayStatus[0] == BUSY && xRayStatus[1] == BUSY)
						{
							if(patientWaitingForXrayCount[0] > patientWaitingForXrayCount[1])
							{
								tempXrayRoom = 1;
							}	
							else
							{
								tempXrayRoom = 0;
							}
						}
						else if(xRayStatus[0] == BUSY && xRayStatus[1] == FREE)
						{
							tempXrayRoom = 1;
						}
						else if(xRayStatus[1] == BUSY && xRayStatus[0] == FREE)
						{
							tempXrayRoom = 0;
						}
						else
						{
							tempXrayRoom = 0;
						}
					}
					if(XRAYTECH == 1)
					{
						tempXrayRoom = 0;
					}
					patientExamSheet[myExamSheet[myIndex].pid].xRayRoom = tempXrayRoom;
					Release(xRayLineLock);
					Signal(examRoomCV[myExamSheet[myIndex].room],examRoomLock[myExamSheet[myIndex].room]);
					Wait(examRoomCV[myExamSheet[myIndex].room],examRoomLock[myExamSheet[myIndex].room]);
					
					Acquire(taskLock);
					examRoomStatus[roomList[myIndex]]= FREE;
					Release(taskLock);
					myExamSheet[myIndex].status=FREE;
					Release(examRoomLock[myExamSheet[myIndex].room]);
				}
				else if(myExamSheet[myIndex].treatment[0]=='S')
				{
					if(myExamSheet[myIndex].type == 0)
					{
						Acquire(printlock);
						Printf1((unsigned int)"\nNurse ",myIndex);
						Printf1((unsigned int)" goes to supply cabinet to give to take medicine for Adult Patient ",myExamSheet[myIndex].pid);
						Printf1((unsigned int)"\nNurse ",myIndex);
						Printf1((unsigned int)" asks Adult Patient ",myExamSheet[myIndex].pid);
						Printf((unsigned int)" \"Are you ready for the shot?\"");
						Release(printlock);
					}
					else
					{
						Acquire(printlock);
						Printf1((unsigned int)"\nNurse ",myIndex);
						Printf1((unsigned int)" goes to supply cabinet to give to take medicine for Child Patient ",myExamSheet[myIndex].pid-PATIENTS);
						Printf1((unsigned int)"\nNurse ",myIndex);
						Printf1((unsigned int)" asks Child Patient ",myExamSheet[myIndex].pid-PATIENTS);
						Printf((unsigned int)" \"Are you ready for the shot?\"");
						Release(printlock);
					}
					Signal(examRoomCV[myExamSheet[myIndex].room],examRoomLock[myExamSheet[myIndex].room]);
					Wait(examRoomCV[roomList[myIndex]],examRoomLock[roomList[myIndex]]);
					if(patientExamSheet[myExamSheet[myIndex].pid].type == 0)
					{
						Acquire(printlock);
						Printf1((unsigned int)"\nNurse ",myIndex);
						Printf1((unsigned int)" tells Adult Patient ", myExamSheet[myIndex].pid);
						Printf((unsigned int)" \"Your shot is over.\"");
						Printf1((unsigned int)"\nNurse ",myIndex);
						Printf1((unsigned int)" escorts Adult Patient ",myExamSheet[myIndex].pid);
						Printf((unsigned int)" to Cashier.");
						Release(printlock);
					}
					else
					{
						Acquire(printlock);
						Printf1((unsigned int)"\nNurse ",myIndex);
						Printf1((unsigned int)" tells Child Patient ", myExamSheet[myIndex].pid-PATIENTS);
						Printf((unsigned int)" \"Your shot is over.\"");
						Printf1((unsigned int)"\nNurse ",myIndex);
						Printf1((unsigned int)" escorts Parent ",myExamSheet[myIndex].pid-PATIENTS);
						Printf((unsigned int)" to Cashier.");
						Release(printlock);
					}
					Signal(examRoomCV[myExamSheet[myIndex].room],examRoomLock[myExamSheet[myIndex].room]);
					Wait(examRoomCV[roomList[myIndex]],examRoomLock[roomList[myIndex]]);				
						
					Acquire(taskLock);
					examRoomStatus[roomList[myIndex]]= FREE;
					Release(taskLock);
					myExamSheet[myIndex].status=FREE;
					Release(examRoomLock[myExamSheet[myIndex].room]);
				}
				else if(myExamSheet[myIndex].treatment[0]=='F')
				{
					if(myExamSheet[myIndex].type == 0)
					{
						Acquire(printlock);
						Printf1((unsigned int)"\nNurse ",myIndex);
						Printf1((unsigned int)" escorts Adult Patient ",myExamSheet[myIndex].pid);
						Printf((unsigned int)" to Cashier.");
						Release(printlock);
					}
					else
					{
						Acquire(printlock);
						Printf1((unsigned int)"\nNurse ",myIndex);
						Printf1((unsigned int)" escorts Parent ",myExamSheet[myIndex].pid-PATIENTS);
						Printf((unsigned int)" to Cashier.");
						Release(printlock);
					}
					Signal(examRoomCV[myExamSheet[myIndex].room],examRoomLock[myExamSheet[myIndex].room]);
					Wait(examRoomCV[roomList[myIndex]],examRoomLock[roomList[myIndex]]);				
				
					Acquire(taskLock);
					examRoomStatus[roomList[myIndex]]= FREE;
					Release(taskLock);
					myExamSheet[myIndex].status=FREE;
					Release(examRoomLock[myExamSheet[myIndex].room]);
				}
			}
			else if(myExamSheet[myIndex].status == NEED_NURSE_AGAIN)
			{
				myExamSheet[myIndex].servingNurse = myIndex;
				patientExamSheet[myExamSheet[myIndex].pid].servingNurse = myIndex;
				myExamSheet[myIndex].status=WITH_NURSE;
			
				if(myExamSheet[myIndex].type == 0)
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nNurse ",myIndex);
					Printf1((unsigned int)" escorts Adult Patient ",myExamSheet[myIndex].pid);
					Printf((unsigned int)" to Cashier.");
					Release(printlock);
				}
				else
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nNurse ",myIndex);
					Printf1((unsigned int)" escorts Parent ",myExamSheet[myIndex].pid-PATIENTS);
					Printf((unsigned int)" to Cashier.");
					Release(printlock);
				}
				Signal(examRoomCV[myExamSheet[myIndex].room],examRoomLock[myExamSheet[myIndex].room]);
				Wait(examRoomCV[myExamSheet[myIndex].room],examRoomLock[myExamSheet[myIndex].room]);
				myExamSheet[myIndex].status=FREE;

				Acquire(taskLock);
				examRoomStatus[roomList[myIndex]]= FREE;
				Release(taskLock);
				Release(examRoomLock[myExamSheet[myIndex].room]);
			}
			else
			{
				Release(examRoomLock[myExamSheet[myIndex].room]);
			}		
		}
		
		Acquire(xRayWaitingRoomLock);
		if(myExamSheet[myIndex].status == WAIT_FOR_NURSE && patientPresent > 0)
		{
			myExamSheet[myIndex].status = WITH_NURSE;
			myExamSheet[myIndex].servingNurse = myIndex;
			myExamSheet[myIndex].room = roomList[myIndex];
			if(myExamSheet[myIndex].type == 0)
			{
				Acquire(printlock);
				Printf1((unsigned int)"\nNurse ",myIndex);
				Printf1((unsigned int)" escorts Adult Patient ",myExamSheet[myIndex].pid);
				Printf1((unsigned int)" to the examination room ",myExamSheet[myIndex].room);
				Release(printlock);
			}
			else
			{
				Acquire(printlock);
				Printf1((unsigned int)"\nNurse ",myIndex);
				Printf1((unsigned int)" escorts Parent ",myExamSheet[myIndex].pid-PATIENTS);
				Printf1((unsigned int)" to the examination room ",myExamSheet[myIndex].room);
				Release(printlock);
			}
			
			patientExamSheet[myExamSheet[myIndex].pid].pid=myExamSheet[myIndex].pid;
			patientExamSheet[myExamSheet[myIndex].pid].age=myExamSheet[myIndex].age;
			patientExamSheet[myExamSheet[myIndex].pid].name=myExamSheet[myIndex].name;
			patientExamSheet[myExamSheet[myIndex].pid].status=myExamSheet[myIndex].status;
			patientExamSheet[myExamSheet[myIndex].pid].room=myExamSheet[myIndex].room;
			patientExamSheet[myExamSheet[myIndex].pid].examiningDoctor=myExamSheet[myIndex].examiningDoctor;
			patientExamSheet[myExamSheet[myIndex].pid].servingNurse=myExamSheet[myIndex].servingNurse;
			patientExamSheet[myExamSheet[myIndex].pid].xRayRoom=myExamSheet[myIndex].xRayRoom;
			patientExamSheet[myExamSheet[myIndex].pid].treatment=myExamSheet[myIndex].treatment;
			patientExamSheet[myExamSheet[myIndex].pid].type=myExamSheet[myIndex].type;
			
			Signal(xRayWaitingRoomCV,xRayWaitingRoomLock);
			Wait(xRayWaitingRoomNurseCV,xRayWaitingRoomLock);
			myExamSheet[myIndex].status = WAITING_FOR_DOC_AGAIN;
			Acquire(wallPocketLock);
			wallPocket[myExamSheet[myIndex].room].pid=myExamSheet[myIndex].pid;
			wallPocket[myExamSheet[myIndex].room].age=myExamSheet[myIndex].age;
			wallPocket[myExamSheet[myIndex].room].name=myExamSheet[myIndex].name;
			wallPocket[myExamSheet[myIndex].room].status=myExamSheet[myIndex].status;
			wallPocket[myExamSheet[myIndex].room].room=myExamSheet[myIndex].room;
			wallPocket[myExamSheet[myIndex].room].examiningDoctor=myExamSheet[myIndex].examiningDoctor;
			wallPocket[myExamSheet[myIndex].room].servingNurse=myExamSheet[myIndex].servingNurse;
			wallPocket[myExamSheet[myIndex].room].xRayRoom=myExamSheet[myIndex].xRayRoom;
			wallPocket[myExamSheet[myIndex].room].treatment=myExamSheet[myIndex].treatment;
			wallPocket[myExamSheet[myIndex].room].type=myExamSheet[myIndex].type;
			Release(wallPocketLock);
		}
		Release(xRayWaitingRoomLock);
		ran=ran-1;
		Acquire(leaveLock);
		if(leavingPatients==(PATIENTS))
		{
			Release(leaveLock);
			break;
		}
		Release(leaveLock);
	}
	Exit(0);
}
		
void doctor(void)
{
	int myIndex,randomNumb,xrayRandomNumb,y;
	Acquire(doctorIndexLock);
	myIndex=globalDoctorIndex;
	globalDoctorIndex=globalDoctorIndex+1;
	Release(doctorIndexLock);
	while(1)
	{
		for(y=0;y<15;y=y+1)
		{
			Yield();
		}

		Acquire(docLineLock);
		docCount=docCount+1;
		docStatus[myIndex]=FREE;

		Wait(docLineCV[myIndex],docLineLock);
		
		/* Doctor interacts with Nurse */
		
		
		examSheetDocCopy[myIndex].pid=patientExamSheet[patList[myIndex]].pid;
		examSheetDocCopy[myIndex].age=patientExamSheet[patList[myIndex]].age;
		examSheetDocCopy[myIndex].name=patientExamSheet[patList[myIndex]].name;
		examSheetDocCopy[myIndex].status=patientExamSheet[patList[myIndex]].status;
		examSheetDocCopy[myIndex].room=patientExamSheet[patList[myIndex]].room;
		examSheetDocCopy[myIndex].examiningDoctor=patientExamSheet[patList[myIndex]].examiningDoctor;
		examSheetDocCopy[myIndex].servingNurse=patientExamSheet[patList[myIndex]].servingNurse;
		examSheetDocCopy[myIndex].xRayRoom=patientExamSheet[patList[myIndex]].xRayRoom;
		examSheetDocCopy[myIndex].treatment =patientExamSheet[patList[myIndex]].treatment;
		examSheetDocCopy[myIndex].type =patientExamSheet[patList[myIndex]].type;
		examSheetDocCopy[myIndex].no_of_images =patientExamSheet[patList[myIndex]].no_of_images;

		Signal(nurseLineCV[examSheetDocCopy[myIndex].servingNurse],docLineLock);
			
		
		Release(docLineLock);
	
		/* Interaction with Patient Starts */		
		Acquire(examRoomLock[examSheetDocCopy[myIndex].room]);

		if(examSheetDocCopy[myIndex].status == SECOND_TIME)
		{
			Acquire(tempRoomLock[examSheetDocCopy[myIndex].room]);
			if(examSheetDocCopy[myIndex].type == 0)
			{
				Acquire(printlock);
				Printf1((unsigned int)"\nDoctor ",myIndex);
				Printf((unsigned int)" is examining the Xrays of"); 
				Printf1((unsigned int)"[Adult] Patient",examSheetDocCopy[myIndex].pid);
				Printf1((unsigned int)" in Examination room ",examSheetDocCopy[myIndex].room);
				Release(printlock);
			}
			else
			{
				Acquire(printlock);
				Printf1((unsigned int)"\nDoctor ",myIndex);
				Printf((unsigned int)" is examining the Xrays of"); 
				Printf1((unsigned int)"[Child] Patient",examSheetDocCopy[myIndex].pid-PATIENTS);
				Printf1((unsigned int)" in Examination room ",examSheetDocCopy[myIndex].room);
				Release(printlock);
			}
			Acquire(printlock);
			Printf1((unsigned int)"\nDoctor ",myIndex);
			Printf1((unsigned int)" has left Examination Room ",examSheetDocCopy[myIndex].room);
			Printf1((unsigned int)"\nDoctor ",myIndex);
			Printf((unsigned int)" is going to their office.");
			Release(printlock);
			
			patientExamSheet[examSheetDocCopy[myIndex].pid].pid=examSheetDocCopy[myIndex].pid;
			patientExamSheet[examSheetDocCopy[myIndex].pid].age=examSheetDocCopy[myIndex].age;
			patientExamSheet[examSheetDocCopy[myIndex].pid].name=examSheetDocCopy[myIndex].name;
			patientExamSheet[examSheetDocCopy[myIndex].pid].status=examSheetDocCopy[myIndex].status;
			patientExamSheet[examSheetDocCopy[myIndex].pid].room=examSheetDocCopy[myIndex].room;
			patientExamSheet[examSheetDocCopy[myIndex].pid].examiningDoctor=examSheetDocCopy[myIndex].examiningDoctor;
			patientExamSheet[examSheetDocCopy[myIndex].pid].servingNurse=examSheetDocCopy[myIndex].servingNurse;
			patientExamSheet[examSheetDocCopy[myIndex].pid].xRayRoom=examSheetDocCopy[myIndex].xRayRoom;
			patientExamSheet[examSheetDocCopy[myIndex].pid].treatment=examSheetDocCopy[myIndex].treatment;
			patientExamSheet[examSheetDocCopy[myIndex].pid].type=examSheetDocCopy[myIndex].type;
			patientExamSheet[examSheetDocCopy[myIndex].pid].no_of_images=examSheetDocCopy[myIndex].no_of_images;
			
			patientExamSheet[examSheetDocCopy[myIndex].pid].examiningDoctor=myIndex;
			
			Release(examRoomLock[examSheetDocCopy[myIndex].room]); 	
			Acquire(tempRoomLock[examSheetDocCopy[myIndex].room]);	
			
			Acquire(wallPocketLock);
			
			wallPocket[examSheetDocCopy[myIndex].pid].pid=examSheetDocCopy[myIndex].pid;
			wallPocket[examSheetDocCopy[myIndex].pid].age=examSheetDocCopy[myIndex].age;
			wallPocket[examSheetDocCopy[myIndex].pid].name=examSheetDocCopy[myIndex].name;
			wallPocket[examSheetDocCopy[myIndex].pid].status=examSheetDocCopy[myIndex].status;
			wallPocket[examSheetDocCopy[myIndex].pid].room=examSheetDocCopy[myIndex].room;
			wallPocket[examSheetDocCopy[myIndex].pid].examiningDoctor=examSheetDocCopy[myIndex].examiningDoctor;
			wallPocket[examSheetDocCopy[myIndex].pid].servingNurse=examSheetDocCopy[myIndex].servingNurse;
			wallPocket[examSheetDocCopy[myIndex].pid].xRayRoom=examSheetDocCopy[myIndex].xRayRoom;
			wallPocket[examSheetDocCopy[myIndex].pid].treatment=examSheetDocCopy[myIndex].treatment;
			wallPocket[examSheetDocCopy[myIndex].pid].type=examSheetDocCopy[myIndex].type;
			wallPocket[examSheetDocCopy[myIndex].pid].no_of_images=examSheetDocCopy[myIndex].no_of_images;
			
			wallPocket[examSheetDocCopy[myIndex].room].status = NEED_NURSE_AGAIN;
			Release(wallPocketLock);
			Release(tempRoomLock[examSheetDocCopy[myIndex].room]);
		}
		else
		{
			if(examSheetDocCopy[myIndex].type == 0)
			{
				Acquire(printlock);
				Printf1((unsigned int)"\nDoctor ",myIndex);
				Printf((unsigned int)" is reading the examination sheet of");
				Printf1((unsigned int)"[Adult] Patient", examSheetDocCopy[myIndex].pid);
				Printf1((unsigned int)" in Examination room ",examSheetDocCopy[myIndex].room);
				Release(printlock);
			}
			else
			{
				Acquire(printlock);
				Printf1((unsigned int)"\nDoctor ",myIndex);
				Printf1((unsigned int)" is reading the examination sheet of [Child] Patient", examSheetDocCopy[myIndex].pid-PATIENTS);
				Printf1((unsigned int)" in Examination room ",examSheetDocCopy[myIndex].room);
				Release(printlock);
			}		
	/*			Doctor randomly chooses one of the following:
				Patient needs a shot
				Patient needs an X-Ray
				Patient is fine.
	*/		
		
			
			randomNumb = randomNumber()%100;
		
			/* There is a 25% chance that a Patient will need an Xray */
			if(randomNumb >= 0 && randomNumb <= 25)
			{
				if(examSheetDocCopy[myIndex].type == 0)
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nDoctor ",myIndex);
					Printf((unsigned int)" notes down in the sheet that Xray is needed for");
					Printf1((unsigned int)" Adult Patient ",examSheetDocCopy[myIndex].pid);
					Printf1((unsigned int)" in Examination room ",examSheetDocCopy[myIndex].room);
					Release(printlock);
				}
				else
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nDoctor ",myIndex);
					Printf((unsigned int)" notes down in the sheet that Xray is needed for");
					Printf1((unsigned int)"Child Patient ",examSheetDocCopy[myIndex].pid-PATIENTS);
					Printf1((unsigned int)" in Examination room ",examSheetDocCopy[myIndex].room);
					Release(printlock);
				}
				examSheetDocCopy[myIndex].treatment="XRAY";
				patientExamSheet[examSheetDocCopy[myIndex].pid].treatment="XRAY";
			
				xrayRandomNumb = randomNumber()%100;
				if(xrayRandomNumb >= 0 && xrayRandomNumb <= 33)
				{
					examSheetDocCopy[myIndex].no_of_images = 1;
					patientExamSheet[examSheetDocCopy[myIndex].pid].no_of_images=1;
				}
				if(xrayRandomNumb >= 34 && xrayRandomNumb <= 67)
				{
					examSheetDocCopy[myIndex].no_of_images = 2;
					patientExamSheet[examSheetDocCopy[myIndex].pid].no_of_images=2;
				}
				if(xrayRandomNumb >= 68 && xrayRandomNumb <= 100)
				{
					examSheetDocCopy[myIndex].no_of_images = 3;
					patientExamSheet[examSheetDocCopy[myIndex].pid].no_of_images=3;
				}
			}
		
			/* There is a separate 25% chance that a Patient will need a shot.*/
			if(randomNumb >= 75 && randomNumb <= 100)
			{
				if(examSheetDocCopy[myIndex].type == 0)
				{	
					Acquire(printlock);
					Printf1((unsigned int)"\nDoctor ",myIndex);
					Printf1((unsigned int)" notes down in the sheet that Adult Patient ",examSheetDocCopy[myIndex].pid);
					Printf1((unsigned int)" needs to be given a shot in Examination room ",examSheetDocCopy[myIndex].room);
					Release(printlock);
				}
				else
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nDoctor ",myIndex);
					Printf1((unsigned int)" notes down in the sheet that Child Patient ",examSheetDocCopy[myIndex].pid-PATIENTS);
					Printf1((unsigned int)" needs to be given a shot in Examination room ",examSheetDocCopy[myIndex].room);
					Release(printlock);
				}
				examSheetDocCopy[myIndex].treatment="SHOT";
				patientExamSheet[examSheetDocCopy[myIndex].pid].treatment="SHOT";
			}
			if(randomNumb >25 && randomNumb <75)
			{
				if(examSheetDocCopy[myIndex].type == 0)
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nDoctor ",myIndex);
					Printf1((unsigned int)" diagnoses Adult Patient ",examSheetDocCopy[myIndex].pid);
					Printf1((unsigned int)" to be fine and is leaving Examination Room ",examSheetDocCopy[myIndex].room);
					Release(printlock);
				}
				else
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nDoctor ",myIndex);
					Printf1((unsigned int)" diagnoses Child Patient ",examSheetDocCopy[myIndex].pid-PATIENTS);
					Printf1((unsigned int)" to be fine and is leaving Examination Room ",examSheetDocCopy[myIndex].room);
					Release(printlock);
				}
				examSheetDocCopy[myIndex].treatment="FINE";
				patientExamSheet[examSheetDocCopy[myIndex].pid].treatment="FINE";
			}
			
			patientExamSheet[examSheetDocCopy[myIndex].pid].pid=examSheetDocCopy[myIndex].pid;
			patientExamSheet[examSheetDocCopy[myIndex].pid].age=examSheetDocCopy[myIndex].age;
			patientExamSheet[examSheetDocCopy[myIndex].pid].name=examSheetDocCopy[myIndex].name;
			patientExamSheet[examSheetDocCopy[myIndex].pid].status=examSheetDocCopy[myIndex].status;
			patientExamSheet[examSheetDocCopy[myIndex].pid].room=examSheetDocCopy[myIndex].room;
			patientExamSheet[examSheetDocCopy[myIndex].pid].examiningDoctor=examSheetDocCopy[myIndex].examiningDoctor;
			patientExamSheet[examSheetDocCopy[myIndex].pid].servingNurse=examSheetDocCopy[myIndex].servingNurse;
			patientExamSheet[examSheetDocCopy[myIndex].pid].xRayRoom=examSheetDocCopy[myIndex].xRayRoom;
			patientExamSheet[examSheetDocCopy[myIndex].pid].treatment=examSheetDocCopy[myIndex].treatment;
			patientExamSheet[examSheetDocCopy[myIndex].pid].type=examSheetDocCopy[myIndex].type;
						
			patientExamSheet[examSheetDocCopy[myIndex].pid].examiningDoctor=myIndex;
		
			Signal(examRoomCV[examSheetDocCopy[myIndex].room],examRoomLock[examSheetDocCopy[myIndex].room]);
			Wait(examRoomCV[examSheetDocCopy[myIndex].room],examRoomLock[examSheetDocCopy[myIndex].room]);	
		
			Acquire(tempRoomLock[examSheetDocCopy[myIndex].room]);
			
			Acquire(printlock);
			Printf1((unsigned int)"\nDoctor ",myIndex);
			Printf1((unsigned int)" has left Examination Room ",examSheetDocCopy[myIndex].room);
			Printf1((unsigned int)"\nDoctor ",myIndex);
			Printf((unsigned int)" is going to their office.");
			Release(printlock);
		
			Signal(examRoomCV[examSheetDocCopy[myIndex].room],examRoomLock[examSheetDocCopy[myIndex].room]);
			Wait(examRoomCV[examSheetDocCopy[myIndex].room],examRoomLock[examSheetDocCopy[myIndex].room]);	
		
			Release(examRoomLock[examSheetDocCopy[myIndex].room]); 
			Acquire(wallPocketLock);
			examSheetDocCopy[myIndex].status=NEED_NURSE;
			
			wallPocket[examSheetDocCopy[myIndex].room].pid=examSheetDocCopy[myIndex].pid;
			wallPocket[examSheetDocCopy[myIndex].room].age=examSheetDocCopy[myIndex].age;
			wallPocket[examSheetDocCopy[myIndex].room].name=examSheetDocCopy[myIndex].name;
			wallPocket[examSheetDocCopy[myIndex].room].status=examSheetDocCopy[myIndex].status;
			wallPocket[examSheetDocCopy[myIndex].room].room=examSheetDocCopy[myIndex].room;
			wallPocket[examSheetDocCopy[myIndex].room].examiningDoctor=examSheetDocCopy[myIndex].examiningDoctor;
			wallPocket[examSheetDocCopy[myIndex].room].servingNurse=examSheetDocCopy[myIndex].servingNurse;
			wallPocket[examSheetDocCopy[myIndex].room].xRayRoom=examSheetDocCopy[myIndex].xRayRoom;
			wallPocket[examSheetDocCopy[myIndex].room].treatment=examSheetDocCopy[myIndex].treatment;
			wallPocket[examSheetDocCopy[myIndex].room].type=examSheetDocCopy[myIndex].type;
			
			Release(wallPocketLock);
			Release(tempRoomLock[examSheetDocCopy[myIndex].room]);
			ran=ran-1;
		}
	}
	Exit(0);	
}	

void xrayTechnician(void)
{
	/* struct ExamSheet *examSheetXrayCopy = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));  */
	int myIndex,t,randomResult;
	Acquire(xrayIndexLock);
	myIndex=globalXrayIndex;
	globalXrayIndex=globalXrayIndex+1;
	Release(xrayIndexLock);
	
	while(1)
	{
		Acquire(xRayLineLock);
		
		if(patientWaitingForXrayCount[myIndex] > 0)
		{
			Signal(xRayLineCV[myIndex],xRayLineLock);
			patientWaitingForXrayCount[myIndex]=patientWaitingForXrayCount[myIndex]-1;
		}
		else
		{
			xRayStatus[myIndex] = FREE;
		}
		Acquire(xRayInteractionLock[myIndex]);
		Release(xRayLineLock);
		Wait(xRayInteractionCV[myIndex],xRayInteractionLock[myIndex]);
		
		examSheetXrayCopy[myIndex].pid=patientExamSheet[xRayPatientList[myIndex]].pid;
		examSheetXrayCopy[myIndex].age=patientExamSheet[xRayPatientList[myIndex]].age;
		examSheetXrayCopy[myIndex].name=patientExamSheet[xRayPatientList[myIndex]].name;
		examSheetXrayCopy[myIndex].status=patientExamSheet[xRayPatientList[myIndex]].status;
		examSheetXrayCopy[myIndex].room=patientExamSheet[xRayPatientList[myIndex]].room;
		examSheetXrayCopy[myIndex].examiningDoctor=patientExamSheet[xRayPatientList[myIndex]].examiningDoctor;
		examSheetXrayCopy[myIndex].servingNurse=patientExamSheet[xRayPatientList[myIndex]].servingNurse;
		examSheetXrayCopy[myIndex].xRayRoom=patientExamSheet[xRayPatientList[myIndex]].xRayRoom;
		examSheetXrayCopy[myIndex].treatment =patientExamSheet[xRayPatientList[myIndex]].treatment;
		examSheetXrayCopy[myIndex].type =patientExamSheet[xRayPatientList[myIndex]].type;
		examSheetXrayCopy[myIndex].no_of_images =patientExamSheet[xRayPatientList[myIndex]].no_of_images;

		if(examSheetXrayCopy[myIndex].type == 0)
		{
			Acquire(printlock);
			Printf1((unsigned int)"\nXray technician ",myIndex);
			Printf1((unsigned int)" asks Adult Patient", examSheetXrayCopy[myIndex].pid);
			Printf((unsigned int)" to get on the table.");
			Release(printlock);
		}
		else
		{
			Acquire(printlock);
			Printf1((unsigned int)"\nXray technician ",myIndex);
			Printf1((unsigned int)" asks Child Patient", examSheetXrayCopy[myIndex].pid-PATIENTS);
			Printf((unsigned int)" to get on the table.");
			Release(printlock);
		}
		for(t=0;t<examSheetXrayCopy[myIndex].no_of_images;t=t+1)
		{
			if(examSheetXrayCopy[myIndex].type == 0)
			{
				Acquire(printlock);
				Printf1((unsigned int)"\nXray Technician ",myIndex);
				Printf1((unsigned int)" asks Adult Patient ",examSheetXrayCopy[myIndex].pid);
				Printf((unsigned int)" to move.");
				Printf1((unsigned int)"\nXray Technician ",myIndex);
				Printf1((unsigned int)" takes an Xray Image of Adult Patient ",examSheetXrayCopy[myIndex].pid);
				Release(printlock);
			}
			else
			{
				Acquire(printlock);
				Printf1((unsigned int)"\nXray Technician ",myIndex);
				Printf1((unsigned int)" asks Child Patient ",examSheetXrayCopy[myIndex].pid-PATIENTS);
				Printf((unsigned int)" to move.");
				Printf1((unsigned int)"\nXray Technician ",myIndex);
				Printf1((unsigned int)" takes an Xray Image of Child Patient ",examSheetXrayCopy[myIndex].pid-PATIENTS);
				Release(printlock);
			}
			
			randomResult = randomNumber()%3;
			if(randomResult == 0)
			{
				if(examSheetXrayCopy[myIndex].type == 0)
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nXray Technician ",myIndex);
					Printf1((unsigned int)" records [nothing] on Adult Patient ",examSheetXrayCopy[myIndex].pid);
					Printf((unsigned int)"'s examination sheet.");
					Release(printlock);
				}
				else
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nXray Technician ",myIndex);
					Printf1((unsigned int)" records [nothing] on Child Patient ",examSheetXrayCopy[myIndex].pid-PATIENTS);
					Printf((unsigned int)"'s examination sheet.");
					Release(printlock);
				}
				examSheetXrayCopy[myIndex].xRayResult = "nothing";
			}
			if(randomResult == 1)
			{
				if(examSheetXrayCopy[myIndex].type == 0)
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nXray Technician ",myIndex);
					Printf1((unsigned int)" records [break] on Adult Patient ",examSheetXrayCopy[myIndex].pid);
					Printf((unsigned int)"'s examination sheet.");
					Release(printlock);
				}
				else
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nXray Technician ",myIndex);
					Printf1((unsigned int)" records [break] on Child Patient ",examSheetXrayCopy[myIndex].pid-PATIENTS);
					Printf((unsigned int)"'s examination sheet.");
					Release(printlock);
				}
				examSheetXrayCopy[myIndex].xRayResult = "break";
			}
			if(randomResult == 2)
			{
				if(examSheetXrayCopy[myIndex].type == 0)
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nXray Technician ",myIndex);
					Printf1((unsigned int)" records [compound fracture] on Adult Patient ",examSheetXrayCopy[myIndex].pid);
					Printf((unsigned int)"'s examination sheet.");
					Release(printlock);
				}
				else
				{
					Acquire(printlock);
					Printf1((unsigned int)"\nXray Technician ",myIndex);
					Printf1((unsigned int)" records [compound fracture] on Child Patient ",examSheetXrayCopy[myIndex].pid-PATIENTS);
					Printf((unsigned int)"'s examination sheet.");
					Release(printlock);
				}
				examSheetXrayCopy[myIndex].xRayResult = "compound fracture";
			}
						
		}
		patientExamSheet[examSheetXrayCopy[myIndex].pid].xRayResult = examSheetXrayCopy[myIndex].xRayResult;
		if(examSheetXrayCopy[myIndex].type == 0)
		{
			Acquire(printlock);
			Printf1((unsigned int)"\nX-ray Technician ",myIndex);
			Printf1((unsigned int)" tells [Adult Patient] ",examSheetXrayCopy[myIndex].pid);
			Printf((unsigned int)" to wait in Xray waiting room.");
			Printf1((unsigned int)"\nX-ray Technician ",myIndex);
			Printf1((unsigned int)" puts [Adult Patient] ", examSheetXrayCopy[myIndex].pid);
			Printf((unsigned int)" in Xray waiting room wall pocket.");
			Release(printlock);
		}
		else
		{
			Acquire(printlock);
			Printf1((unsigned int)"\nX-ray Technician ",myIndex);
			Printf1((unsigned int)" tells [Child] ",examSheetXrayCopy[myIndex].pid-PATIENTS);
			Printf((unsigned int)" to wait in Xray waiting room.");
			Printf1((unsigned int)"\nX-ray Technician ",myIndex);
			Printf1((unsigned int)" puts [Child] ", examSheetXrayCopy[myIndex].pid-PATIENTS);
			Printf((unsigned int)" in Xray waiting room wall pocket.");
			Release(printlock);
		}
		Acquire(wallPocketLock);
		examSheetXrayCopy[myIndex].status = WAIT_FOR_NURSE;
		
		
		xRayWaitingRoomWallPocket[xRayRoomCnt_atXray].pid=examSheetXrayCopy[myIndex].pid;
		xRayWaitingRoomWallPocket[xRayRoomCnt_atXray].age=examSheetXrayCopy[myIndex].age;
		xRayWaitingRoomWallPocket[xRayRoomCnt_atXray].name=examSheetXrayCopy[myIndex].name;
		xRayWaitingRoomWallPocket[xRayRoomCnt_atXray].status=examSheetXrayCopy[myIndex].status;
		xRayWaitingRoomWallPocket[xRayRoomCnt_atXray].room=examSheetXrayCopy[myIndex].room;
		xRayWaitingRoomWallPocket[xRayRoomCnt_atXray].examiningDoctor=examSheetXrayCopy[myIndex].examiningDoctor;
		xRayWaitingRoomWallPocket[xRayRoomCnt_atXray].servingNurse=examSheetXrayCopy[myIndex].servingNurse;
		xRayWaitingRoomWallPocket[xRayRoomCnt_atXray].xRayRoom=examSheetXrayCopy[myIndex].xRayRoom;
		xRayWaitingRoomWallPocket[xRayRoomCnt_atXray].treatment=examSheetXrayCopy[myIndex].treatment;		
		xRayWaitingRoomWallPocket[xRayRoomCnt_atXray].type=examSheetXrayCopy[myIndex].type;
		xRayWaitingRoomWallPocket[xRayRoomCnt_atXray].no_of_images=examSheetXrayCopy[myIndex].no_of_images;		
		xRayWaitingRoomWallPocket[xRayRoomCnt_atXray].xRayResult=examSheetXrayCopy[myIndex].xRayResult;		

		xRayRoomCnt_atXray=xRayRoomCnt_atXray+1;
		Release(wallPocketLock);
		Signal(xRayInteractionPatCV[myIndex],xRayInteractionLock[myIndex]);	
		Wait(xRayInteractionPatCV[myIndex],xRayInteractionLock[myIndex]);
		
		Release(xRayInteractionLock[myIndex]);
		ran=ran-1;
	}
	Exit(0);
}

void cashier()
{
	int amount,y;
	/* struct ExamSheet *examSheetCashierCopy = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));  */
	
	while(1)
	{
		for(y=0;y<15;y=y+1)
		{
			Yield();
		}
	
		Acquire(cashierLineLock);
		if(patientCashierWaitingCount == 0)
		{
			 cashierStatus=FREE;
		}
		else
		{
			Signal(cashierLineCV,cashierLineLock);
			patientCashierWaitingCount=patientCashierWaitingCount-1;
		}
		
		Acquire(cashierInteractionLock);
		Release(cashierLineLock);
		Wait(cashierInteractionCV,cashierInteractionLock);	
		
				examSheetCashierCopy.pid=patientExamSheet[cashierLine[myPosition]].pid;
				examSheetCashierCopy.age=patientExamSheet[cashierLine[myPosition]].age;
				examSheetCashierCopy.name=patientExamSheet[cashierLine[myPosition]].name;
				examSheetCashierCopy.status=patientExamSheet[cashierLine[myPosition]].status;
				examSheetCashierCopy.room=patientExamSheet[cashierLine[myPosition]].room;
				examSheetCashierCopy.examiningDoctor=patientExamSheet[cashierLine[myPosition]].examiningDoctor;
				examSheetCashierCopy.servingNurse=patientExamSheet[cashierLine[myPosition]].servingNurse;
				examSheetCashierCopy.xRayRoom=patientExamSheet[cashierLine[myPosition]].xRayRoom;
				examSheetCashierCopy.treatment =patientExamSheet[cashierLine[myPosition]].treatment;		
		
		myPosition=myPosition+1;
		if(examSheetCashierCopy.type == 0)
		{
			Acquire(printlock);
			Printf1((unsigned int)"\nCashier receives the examination sheet from Adult Patient ",examSheetCashierCopy.pid);
			Release(printlock);
		}
		else
		{
			Acquire(printlock);
			Printf1((unsigned int)"\nCashier receives the examination sheet for Child Patient ",examSheetCashierCopy.pid-PATIENTS);
			Printf1((unsigned int)" from Parent ",examSheetCashierCopy.pid-PATIENTS);
			Release(printlock);
		}
		
		if(examSheetCashierCopy.treatment[0]=='X')
		{
			amount = randomNumber()%1000;
		
		}
		else if(examSheetCashierCopy.treatment[0]=='S')
		{
			amount = randomNumber()%500;
		}
		else
		{
			amount = randomNumber()%100;
		}
		
		if(examSheetCashierCopy.type == 0)
		{
			Acquire(printlock);
			Printf1((unsigned int)"\nCashier reads the examination sheet of Adult Patient ",examSheetCashierCopy.pid);
			Printf1((unsigned int)" and asks him to pay $",amount);
			Release(printlock);
		}
		else
		{
			Acquire(printlock);
			Printf1((unsigned int)"\nCashier reads the examination sheet of Child Patient ",examSheetCashierCopy.pid-PATIENTS);
			Printf1((unsigned int)" and asks Parent ",examSheetCashierCopy.pid-PATIENTS);
			Printf1((unsigned int)" to pay $",amount);
			Release(printlock);
		}
		Signal(cashierInteractionCV,cashierInteractionLock);	
		Wait(cashierInteractionCV,cashierInteractionLock);	

		if(examSheetCashierCopy.type == 0)
		{
			Acquire(printlock);
			Printf1((unsigned int)"\nCashier accepts $",amount);
			Printf1((unsigned int)" from Adult Patient ",examSheetCashierCopy.pid);
			Printf1((unsigned int)"\nCashier gives a receipt of $",amount);
			Printf1((unsigned int)" to Adult Patient ",examSheetCashierCopy.pid);
			Release(printlock);
		}
		else
		{
			Acquire(printlock);
			Printf1((unsigned int)"\nCashier accepts $",amount);
			Printf1((unsigned int)" from Adult Parent ",examSheetCashierCopy.pid-PATIENTS);
			Printf1((unsigned int)"\nCashier gives a receipt of $",amount);
			Printf1((unsigned int)" to Parent ",examSheetCashierCopy.pid-PATIENTS);
			Release(printlock);
		}
		Signal(cashierInteractionCV,cashierInteractionLock);
		Wait(cashierInteractionCV,cashierInteractionLock);		
		Release(cashierInteractionLock);
	}
	Exit(0);
}



void main() {
	/* Initialization*/
	int i;
	wrnLineLock = CreateLock("wrnLineLock",sizeof("wrnLineLock"));
	wrnLock = CreateLock("WL",sizeof("WL"));	
	infoLock = CreateLock("IL",sizeof("IL"));
	waitingRoomLock = CreateLock("WRL",sizeof("WRL"));
	taskLock = CreateLock("TL",sizeof("TL"));
	docLineLock = CreateLock("DLL",sizeof("DLL"));
	needNurseLock = CreateLock("NNL",sizeof("NNL"));
	newTempLock = CreateLock("NTL",sizeof("NTL"));
	xRayWaitingRoomLock = CreateLock("XWRL",sizeof("XWRL"));
	cashierLineLock = CreateLock("CLL",sizeof("CLL"));
	cashierInteractionLock = CreateLock("CIL",sizeof("CIL"));
	wallPocketLock = CreateLock("WPL",sizeof("WPL"));
	xRayLineLock = CreateLock("XRLL",sizeof("XRLL"));	
	leaveLock = CreateLock("LL",sizeof("LL"));
	
	nurseIndexLock = CreateLock("NIL",sizeof("NIL"));
	patientIndexLock = CreateLock("PIL",sizeof("PIL"));
	doctorIndexLock = CreateLock("DIL",sizeof("DIL"));
	xrayIndexLock = CreateLock("XIL",sizeof("XIL"));
		
	patientWaitingCV = CreateCV("PWCV",sizeof("PWCV"));
	wrnWaitingCV = CreateCV("WWCV",sizeof("WWCV"));	
	wrnWaitingPatientCV = CreateCV("WWPCV",sizeof("WWPCV"));	
	wrnWaitingAgainPatientCV = CreateCV("WWAPCV",sizeof("WWAPCV"));	
	wrnWaitingNurseCV = CreateCV("WWNCV",sizeof("WWNCV"));	
	nurseWaitingCV = CreateCV("NWCV",sizeof("NWCV"));	
	waitingRoomCV = CreateCV("WRCV",sizeof("WRCV"));
	waitingRoomNurseCV = CreateCV("WRNCV",sizeof("WRNCV"));
	
	xRayWaitingRoomCV = CreateCV("XWRCV",sizeof("XWRCV"));
	cashierLineCV = CreateCV("CLCV",sizeof("CLCV"));
	cashierInteractionCV = CreateCV("CICV",sizeof("CICV"));
	xRayWaitingRoomNurseCV = CreateCV("XWRNV",sizeof("XWRNV"));
	
	
	for(i=0;i<ROOMS;i=i+1)
	{
		examRoomStatus[i] = FREE;
		examRoomLock[i] = CreateLock("ERL",sizeof("ERL"));
		examRoomCV[i] = CreateCV("ERCV",sizeof("ERCV"));
		tempRoomLock[i] = CreateLock("TRL",sizeof("TRL"));
	}
	
	for(i=0;i<DOCTORS;i=i+1)
	{
		docStatus[i]=-1;
		patList[i]=-1;
		docLineCV[i]=CreateCV("DLCV",sizeof("DLCV"));
	}
	
	wrnExamSheet.pid=-1;
	wrnExamSheet.age=-1;
	wrnExamSheet.name=NULL;
	wrnExamSheet.status=-1;
	wrnExamSheet.room=-1;
	wrnExamSheet.examiningDoctor=-1;
	wrnExamSheet.servingNurse=-1;
	wrnExamSheet.xRayRoom=-1;
	wrnExamSheet.treatment ="";
	wrnExamSheet.type =0;
	
	
	for(i=0;i<PATIENTS;i=i+1)
	{
		/*patientExamSheet[i] = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));*/
		patientExamSheet[i].pid=-1;
		patientExamSheet[i].age=-1;
		patientExamSheet[i].name=NULL;
		patientExamSheet[i].status=-1;
		patientExamSheet[i].room=-1;
		patientExamSheet[i].examiningDoctor=-1;
		patientExamSheet[i].servingNurse=-1;
		patientExamSheet[i].xRayRoom=-1;
		currentSymptom[i]=-1;
		patientExamSheet[i].treatment = "";
		patientExamSheet[i].type = 0;
		
	}
	
	for(i=0;i<NURSES;i=i+1)
	{
		myExamSheet[i].pid=-1;
		myExamSheet[i].age=-1;
		myExamSheet[i].name=NULL;
		myExamSheet[i].status=-1;
		myExamSheet[i].room=-1;
		myExamSheet[i].examiningDoctor=-1;
		myExamSheet[i].servingNurse=-1;
		myExamSheet[i].xRayRoom=-1;
		myExamSheet[i].treatment ="";
		myExamSheet[i].type = 0;
		
	}
	
	for(i=0;i<DOCTORS;i=i+1)
	{
		examSheetDocCopy[i].pid=-1;
		examSheetDocCopy[i].age=-1;
		examSheetDocCopy[i].name=NULL;
		examSheetDocCopy[i].status=-1;
		examSheetDocCopy[i].room=-1;
		examSheetDocCopy[i].examiningDoctor=-1;
		examSheetDocCopy[i].servingNurse=-1;
		examSheetDocCopy[i].xRayRoom=-1;
		examSheetDocCopy[i].treatment ="";
		examSheetDocCopy[i].type =0;
		
	}
	
	for(i=0;i<XRAYTECH;i=i+1)
	{
		examSheetXrayCopy[i].pid=-1;
		examSheetXrayCopy[i].age=-1;
		examSheetXrayCopy[i].name=NULL;
		examSheetXrayCopy[i].status=-1;
		examSheetXrayCopy[i].room=-1;
		examSheetXrayCopy[i].examiningDoctor=-1;
		examSheetXrayCopy[i].servingNurse=-1;
		examSheetXrayCopy[i].xRayRoom=-1;
		examSheetXrayCopy[i].treatment ="";
		examSheetXrayCopy[i].type =0;
	}
	
	for(i=0;i<PATIENTS;i=i+1)
	{
		xRayWaitingRoomWallPocket[i].pid=-1;
		xRayWaitingRoomWallPocket[i].age=-1;
		xRayWaitingRoomWallPocket[i].name=NULL;
		xRayWaitingRoomWallPocket[i].status=-1;
		xRayWaitingRoomWallPocket[i].room=-1;
		xRayWaitingRoomWallPocket[i].examiningDoctor=-1;
		xRayWaitingRoomWallPocket[i].servingNurse=-1;
		xRayWaitingRoomWallPocket[i].xRayRoom=-1;
		xRayWaitingRoomWallPocket[i].treatment ="";
		xRayWaitingRoomWallPocket[i].type =0;
	}
	
	examSheetCashierCopy.pid=-1;
	examSheetCashierCopy.age=-1;
	examSheetCashierCopy.name=NULL;
	examSheetCashierCopy.status=-1;
	examSheetCashierCopy.room=-1;
	examSheetCashierCopy.examiningDoctor=-1;
	examSheetCashierCopy.servingNurse=-1;
	examSheetCashierCopy.xRayRoom=-1;
	examSheetCashierCopy.treatment ="";
	examSheetCashierCopy.type =0;
	
	for(i=0;i<NURSES;i=i+1)
	{
		roomList[i] = -1;
		nurseLineCV[i] = CreateCV("NLCV",sizeof("NLCV"));
		currentDoc[i]=-1;
		tempLock[i] = CreateLock("TL",sizeof("TL"));
		tempCV[i] = CreateCV("TCV",sizeof("TCV"));
		nurseStatus[i]=-1;
	}
	printlock= CreateLock("printlock",sizeof("printlock"));
	
	for(i=0;i<XRAYTECH;i=i+1)
	{
		patientWaitingForXrayCount[i] = 0;
		xRayInteractionLock[i] = CreateLock("XRIL",sizeof("XRIL"));
		xRayLineCV[i] = CreateCV("XRLCV",sizeof("XRLCV"));
		xRayInteractionCV[i] = CreateCV("XRICV",sizeof("XRICV"));
		xRayPatientList[i] = -1;
		xRayStatus[i] = -1;
		xRayInteractionPatCV[i] = CreateCV("XRIPCV",sizeof("XRIPCV"));
		xRayInteractionNurseCV[i] = CreateCV("XRINCV",sizeof("XRINCV"));
	}
	doctoroffice();
	for ( i=0;i<100000;i++)
	{
		Yield();
	}
}

void doctoroffice() 
{
	int i,j,k;
	Acquire(printlock);
	Printf((unsigned int)"\nSimulating WRN Patient interaction");
	Printf((unsigned int)"\nNumber of patients entering doctors office: [5]");
	Printf((unsigned int)"\nNumber of Nurses(2-5): [3]");
	Printf((unsigned int)"\nNumber of Exam Rooms(2-5): [2]");
	Printf((unsigned int)"\nNumber of Doctors(2-5): [2]");
	Printf((unsigned int)"\nNumber of Xray technicians(1-2): [2]");
	Release(printlock);
	Printf((unsigned int)"\n Fork WRN :"); 
	Fork(waitingRoomNurse);
	
	
	/*Patient cannot come into office until WRN starts.*/
	
	for(i=0 ; i<100 ; i=i+1) {
		Yield();
	}

	for(i=0 ; i<PATIENTS ; i=i+1) {
	   Printf1((unsigned int)"\n Fork Patient :",i); 
		Fork(patient);
	}
	
	for(j=0 ; j<NURSES ; j=j+1) {
		Printf1((unsigned int)"\n Fork Nurse :",j); 
		 Fork(nurse); 
	}
	for(j=0 ; j<DOCTORS ; j=j+1) {
		Printf1((unsigned int)"\n Fork Doctor :",j); 
		 Fork(doctor); 
	}
	
	for(j=0 ; j<XRAYTECH ; j=j+1) {
		Printf1((unsigned int)"\n Fork XRAYTECH :",j); 
		 Fork(xrayTechnician); 
	}
	
	Printf((unsigned int)"\n Fork Cashier"); 
	Fork(cashier);
	
	return;
}
 
