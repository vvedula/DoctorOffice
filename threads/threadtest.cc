
#include "system.h"
#include "synch.h"
#include "thread.h"
#include "stdio.h"
#include <iostream>
#include "string.h"
#include <cstdlib>

//Declaration of struct
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
};

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

Lock *wrnLineLock;
Lock *wrnLock;
Lock *infoLock;

Lock *waitingRoomLock;
Lock *taskLock;
Lock *examRoomLock[50];

Lock *docLineLock;
Lock *needNurseLock;
Lock *tempRoomLock[5];
Lock *tempLock[5];
Lock *newTempLock;

Lock *xRayLineLock;
Lock *xRayInteractionLock[2];
Lock *xRayWaitingRoomLock;

Lock *cashierLineLock;
Lock *cashierInteractionLock;

Lock *wallPocketLock;

Lock *familyLock[100];
Lock *leaveLock;

Condition *patientWaitingCV;
Condition *wrnWaitingCV;
Condition *nurseWaitingCV;
Condition *waitingRoomCV;
Condition *examRoomCV[50];
Condition *waitingRoomNurseCV;
Condition *nurseLineCV[5];
Condition *docLineCV[3];
Condition *tempCV[5];

Condition *xRayLineCV[2];
Condition *xRayInteractionCV[2];
Condition *xRayInteractionPatCV[2];
Condition *xRayInteractionNurseCV[2];
Condition *xRayWaitingRoomCV;
Condition *xRayWaitingRoomNurseCV;

Condition *cashierLineCV;
Condition *cashierInteractionCV;

Condition *familyCV[100];

//Functions
char randomChar();
void waitingRoomNurse();
void patient(int);
void nurse(int);
void doctor(int);
void xRayTechnician(int);
void cashier();
void parent(int);
void child(int);

char randomChar() {
	char randomCharacter;
	int randomNumber = random()%26;
	randomNumber += 97;
	randomCharacter = (char)randomNumber;
	return randomCharacter;
}

int patientPresent=0;
int no_of_patients=-1;
int no_of_nurses=-1;
int no_of_rooms=-1;
int no_of_doctors=-1;
int wrnStatus=-1;
int wrnTaskStatus=SERVE_NEITHER;
int patientTask=-1;
int nurseTask=-1;
int roomList[50];
int currentNurse=-1;
int currentNurse_dLL=-1;
int currentPatient=-1;
int currentPatient_erl=-1;
int currentPatient_dLL = -1;
int currentRoom=-1;
int currentDoc[5];
int currentRoom_erl=-1;
int currentSymptom[100];
int currentNurse_wrl=-1;
int patientWaitingCount=0;
int patientGettingForm=-1;
int patientGivingForm=-1;
int registeredPatientCount=0;
int nurseWaitingCount=0;
int examRoomStatus[50];
int docCount=0;
int docStatus[5];
int counter=0;
int nursePatCount=0;
int patList[5];
int leavingPatients=0;
int no_of_xRayTechnicians;
int patientWaitingForXrayCount[2];
int tempXrayRoom = -1;
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
int parentSignalled[100]={0};
int no_of_childPatients;
int no_of_parents;

struct ExamSheet *wrnExamSheet = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));
struct ExamSheet *patientExamSheet[100];
struct ExamSheet *wallPocket[5];
struct ExamSheet *xRayWaitingRoomWallPocket[100];


void waitingRoomNurse() {
	int patientList[50],p=0;
	for(int a=0;a<50;a++)
	{
		patientList[a]=-1;
	}
//	printf("\n\nEntered WRN");
	/* Local storage for patient id's to give to nurse */
	while(1)
	{	
		/*Acquire lock for interacting with the line */		
		wrnLineLock->Acquire();
		if(nurseWaitingCount == 0 && patientWaitingCount == 0)
		{
			 wrnStatus=FREE;
		}
		else if(patientWaitingCount > 0)
		{
			patientWaitingCV->Signal(wrnLineLock);
			patientWaitingCount--;
		}
		else if(nurseWaitingCount > 0)
		{
			nurseWaitingCV->Signal(wrnLineLock);
			nurseWaitingCount--;
		}
		wrnLock->Acquire();
		wrnLineLock->Release();
		wrnWaitingCV->Wait(wrnLock);			
		
		if(wrnTaskStatus == SERVE_PATIENT)
		{
			/*Check task to do with patient form. After receiving Signal from Patient */
			if(patientTask==GETFORM)
			{
					//Give the patient a new Blank Exam sheet
					wrnExamSheet->name = "BLANK";
					wrnExamSheet->age = -1;		//Age = -1 indicates a blank field
					if(patientExamSheet[patientGettingForm]->type == 0)
					{
						printf("Waiting Room nurse gives a form to Adult patient [%d]\n",patientGettingForm);
					}
					else
					{
						printf("Waiting Room nurse gives a form to Parent [%d]\n",patientGettingForm-no_of_patients);
					}
					/* Take the Form & Leave the Line. So that i can become free again */
					wrnWaitingCV->Signal(wrnLock);
					wrnLock->Release();
			}
			if(patientTask==GIVEFORM)
			{
				if(patientExamSheet[patientGivingForm]->type == 0)
				{
					printf("Waiting Room nurse accepts the form from Adult Patient [%d] with name [%s] and age [%d].\n",patientGivingForm,patientExamSheet[patientGivingForm]->name,patientExamSheet[patientGivingForm]->age);
				printf("Waiting Room nurse creates an examination sheet for [Adult] patient[%d] with name [%s] and age [%d].\n",patientGivingForm,patientExamSheet[patientGivingForm]->name,patientExamSheet[patientGivingForm]->age);
				printf("Waiting Room nurse tells the Adult Patient [%d] to wait in the waiting room for a nurse\n",patientGivingForm);
				}
				else
				{
				printf("\nWaiting Room nurse accepts the form from Parent [%d] with name [%s] and age [%d].\n",patientGivingForm-no_of_patients,patientExamSheet[patientGivingForm]->name,patientExamSheet[patientGivingForm]->age);
				printf("Waiting Room nurse creates an examination sheet for [Child] patient[%d] with name [%s] and age [%d].\n",patientGivingForm-no_of_patients,patientExamSheet[patientGivingForm]->name,patientExamSheet[patientGivingForm]->age);
				printf("Waiting Room nurse tells the Parent [%d] to wait in the waiting room for a nurse\n",patientGivingForm-no_of_patients);
				}
				registeredPatientCount++;
				patientList[p]=patientGivingForm;
				p++;
				wrnWaitingCV->Signal(wrnLock);
				wrnLock->Release();
			}
		}
		if(wrnTaskStatus == SERVE_NURSE)
		{
			if(nurseTask == FETCH_PATIENT)
			{
				for(int m=0;m<p;m++)
				{
					if(patientList[m]!=-1)
					{
						int patient=patientList[m];
						patientList[m]=-1;
						if(patientExamSheet[patient]->type == 0)
						{
							printf("Waiting Room nurse gives examination sheet of patient[%d] to Nurse[%d]\n",patient,currentNurse);
						}
						else
						{
							printf("Waiting Room nurse gives examination sheet of Child patient [%d] to Nurse[%d]\n",patient-no_of_patients,currentNurse);
						}
						wrnWaitingCV->Signal(wrnLock);
						break;
					}
				}
				wrnLock->Release();
			}
		}		
	}
}

void patient(int i) {
	printf("Adult Patient:%d has entered the Doctor's Office Waiting Room.\n",i);
	//patientExamSheet[i] = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));
		
	wrnLineLock->Acquire();
	if(wrnStatus == BUSY) {
		//If WRN is busy,patient enters line
		printf("Adult Patient:%d gets in line of the Waiting Room Nurse to get registration form.\n",i);
		patientWaitingCount++;
		patientWaitingCV->Wait(wrnLineLock);
	}
	else if(wrnStatus == FREE) {
		wrnStatus = BUSY;
	}

	/* Patient was waiting in line for wrn. When wrn signals him, he has to come out of line.*/
	//enteringPatientCnt++;
	wrnLineLock->Release();
	
	/* Interaction with wrn begins */
	wrnLock->Acquire();
	wrnTaskStatus=SERVE_PATIENT;
	patientTask = GETFORM;
	patientGettingForm=i;
	/* Tell the wrn that i have acquired the form */
	wrnWaitingCV->Signal(wrnLock);
	wrnWaitingCV->Wait(wrnLock);
	/* After Receiving Blank sheet from wrn */
	wrnLock->Release();

	/* After Getting out of Line. Patient Fills out the form */
	infoLock->Acquire();
	
	// Allocate random values of age & name to patient

	srand(time(0));
	patientExamSheet[i]->age = random()%100 + 1;
	char temp[10];		//Temporary array used to initialize name
	 for(int k1=0;k1<10;k1++) {
	        temp[k1] = randomChar();  
        }
        patientExamSheet[i]->name = temp;
        patientExamSheet[i]->pid = i;
	patientExamSheet[i]->room = -1;
	patientExamSheet[i]->status = -1;
        
        infoLock->Release();
        /* Give some delay for patient to enter his info */ 
        for(int k2=0;k2<10;k2++) {
        	currentThread->Yield();
        }
        /* Patient Re-enters the waiting Queue */
        wrnLineLock->Acquire();
        
	if(wrnStatus == BUSY) {
		printf("Adult Patient:%d gets in line of the Waiting Room Nurse to submit registration form.\n",i);
		patientWaitingCount++;
		patientWaitingCV->Wait(wrnLineLock);
	}
	else if(wrnStatus == FREE)	{
		wrnStatus = BUSY;
	}
	
	/* Patient was waiting in line for wrn. When wrn signals him, he has to come out of line.*/
	wrnLineLock->Release();
	
	/* Interacts with Wrn again */
	wrnLock->Acquire();
	wrnTaskStatus=SERVE_PATIENT;
	patientTask=GIVEFORM;
        patientGivingForm=i;

        /* Tell wrn that i have to submit the form*/
	printf("Adult patient [%d] submits the filled form to the Waiting Room Nurse.\n",i);
       	wrnWaitingCV->Signal(wrnLock);
	wrnWaitingCV->Wait(wrnLock);
		
	/*Now patient has to wait in waiting Room. He acquires Waiting for Nurse Lock & Waits on it */

	waitingRoomLock->Acquire();
	waitingRoomCnt++;
	wrnLock->Release();	
	waitingRoomCV->Wait(waitingRoomLock);
	
	/* Patient Nurse Interaction Begins */

	currentPatient = i;

	waitingRoomNurseCV->Signal(waitingRoomLock);
	waitingRoomNurseCV->Wait(waitingRoomLock);
	
	printf("Adult Patient [%d] is following Nurse [%d] to Examination Room [%d].\n",i,patientExamSheet[i]->servingNurse,patientExamSheet[i]->room);	

	waitingRoomNurseCV->Signal(waitingRoomLock);
	waitingRoomNurseCV->Wait(waitingRoomLock);
	
	examRoomLock[patientExamSheet[i]->room]->Acquire();
	waitingRoomLock->Release();
	
	examRoomCV[patientExamSheet[i]->room]->Signal(examRoomLock[patientExamSheet[i]->room]);
	examRoomCV[patientExamSheet[i]->room]->Wait(examRoomLock[patientExamSheet[i]->room]);
	
	srand(time(0));
	currentSymptom[i] = random()%3;	//Assign Random Symptoms to Patient
	if(currentSymptom[i]==0)
	{
		printf("Adult Patient [%d] says, “My symptoms are Nausea.\"\n",i);
	}
	else if(currentSymptom[i]==1)
	{
		printf("Adult Patient [%d] says, “My symptoms are Pain.\"\n",i);
	}
	else if(currentSymptom[i]==2)
	{
		printf("Adult Patient [%d] says, “My symptoms are I Hear Alien Voices.\"\n",i);
	}
	examRoomCV[patientExamSheet[i]->room]->Signal(examRoomLock[patientExamSheet[i]->room]);
	examRoomCV[patientExamSheet[i]->room]->Wait(examRoomLock[patientExamSheet[i]->room]);
	
	
	examRoomCV[patientExamSheet[i]->room]->Signal(examRoomLock[patientExamSheet[i]->room]);
	examRoomCV[patientExamSheet[i]->room]->Wait(examRoomLock[patientExamSheet[i]->room]);
	
	if(strcmp(patientExamSheet[i]->treatment,"XRAY") == 0)
	{
		printf("Adult Patient [%d] has been informed by Doctor [%d] that he needs an Xray.\n",i,patientExamSheet[i]->examiningDoctor);
	}
	else if(strcmp(patientExamSheet[i]->treatment,"SHOT") == 0)
	{
		printf("Adult Patient [%d] has been informed by Doctor [%d] that he will be administered a shot.\n",i,patientExamSheet[i]->examiningDoctor);
	}
	else if(strcmp(patientExamSheet[i]->treatment,"FINE") == 0)
	{
		printf("Adult Patient [%d] has been diagnosed by Doctor [%d].\n",i,patientExamSheet[i]->examiningDoctor);
	}
	
	examRoomCV[patientExamSheet[i]->room]->Signal(examRoomLock[patientExamSheet[i]->room]);
	examRoomCV[patientExamSheet[i]->room]->Wait(examRoomLock[patientExamSheet[i]->room]);
	
//	leavingPatients++;
	examRoomCV[patientExamSheet[i]->room]->Signal(examRoomLock[patientExamSheet[i]->room]);
	//printf("\nBefore pat %d waits on ERL %d",i,patientExamSheet[i]->room);
	examRoomCV[patientExamSheet[i]->room]->Wait(examRoomLock[patientExamSheet[i]->room]);
	/* Wait for Nurse to come see patient */
	
	if(strcmp(patientExamSheet[i]->treatment,"XRAY") == 0)
	{
//		newTempLock->Acquire();
		printf("Adult Patient [%d] waits for a Nurse to escort them to the Xray room.\n",i);

		examRoomCV[patientExamSheet[i]->room]->Signal(examRoomLock[patientExamSheet[i]->room]);
		examRoomCV[patientExamSheet[i]->room]->Wait(examRoomLock[patientExamSheet[i]->room]);
		
		xRayLineLock->Acquire();
		examRoomCV[patientExamSheet[i]->room]->Signal(examRoomLock[patientExamSheet[i]->room]);
		examRoomLock[patientExamSheet[i]->room]->Release();
//		newTempLock->Release();
		
		if(xRayStatus[patientExamSheet[i]->xRayRoom] == BUSY)
		{
			patientWaitingForXrayCount[patientExamSheet[i]->xRayRoom]++;
			xRayLineCV[patientExamSheet[i]->xRayRoom]->Wait(xRayLineLock);
		}
		else if(xRayStatus[patientExamSheet[i]->xRayRoom] == FREE)
		{
			xRayStatus[patientExamSheet[i]->xRayRoom] = BUSY;
		}
		xRayInteractionLock[patientExamSheet[i]->xRayRoom]->Acquire();
		xRayLineLock->Release();
		xRayPatientList[patientExamSheet[i]->xRayRoom] = i;

	//	printf("\nBefore Signal? %d ",i);
		xRayInteractionCV[patientExamSheet[i]->xRayRoom]->Signal(xRayInteractionLock[patientExamSheet[i]->xRayRoom]);
	//	printf("\nBefore wait? %d ",i);
		xRayInteractionPatCV[patientExamSheet[i]->xRayRoom]->Wait(xRayInteractionLock[patientExamSheet[i]->xRayRoom]);
	//	printf("\nAfter wait? %d ",i);			
					
	//	Added
		xRayWaitingRoomLock->Acquire();
	//	printf("\nAcquired lock on xray wRoom %d",i);	
		printf("Adult Patient [%d] waits for a Nurse to escort him/her to examination room. %d\n",i,patientExamSheet[i]->room);
		//printf("\n Patient % d --> Signal me",i);
		patientPresent++;
		xRayInteractionPatCV[patientExamSheet[i]->xRayRoom]->Signal(xRayInteractionLock[patientExamSheet[i]->xRayRoom]);
		xRayInteractionLock[patientExamSheet[i]->xRayRoom]->Release();
	//	printf("\nPatient %d Waits",i);
		xRayWaitingRoomCV->Wait(xRayWaitingRoomLock);
	//	printf("\nPatient %d Comes out",i);
	//	patientPresent--;
		
		printf("[Adult Patient] [%d] is following Nurse [%d] to Examination Room [%d].\n",i,patientExamSheet[i]->servingNurse,patientExamSheet[i]->room);
		xRayWaitingRoomNurseCV->Signal(xRayWaitingRoomLock);
		examRoomLock[patientExamSheet[i]->room]->Acquire();
		xRayWaitingRoomLock->Release();
		
		examRoomCV[patientExamSheet[i]->room]->Wait(examRoomLock[patientExamSheet[i]->room]);
	}
	else if(strcmp(patientExamSheet[i]->treatment,"SHOT") == 0)
	{
		printf("Adult Patient [%d] says, \"Yes I am ready for the shot\".\n",i);
		examRoomCV[patientExamSheet[i]->room]->Signal(examRoomLock[patientExamSheet[i]->room]);
		examRoomCV[patientExamSheet[i]->room]->Wait(examRoomLock[patientExamSheet[i]->room]);				
	}
	else if(strcmp(patientExamSheet[i]->treatment,"FINE") == 0)
	{
	}
	examRoomCV[patientExamSheet[i]->room]->Signal(examRoomLock[patientExamSheet[i]->room]);
	printf("Adult Patient [%d] enters the queue for Cashier\n",i);
	cashierLineLock->Acquire();
	examRoomLock[patientExamSheet[i]->room]->Release();
	if(cashierStatus == BUSY) {
		//If Cashier is busy,patient enters line
		patientCashierWaitingCount++;
		cashierLineCV->Wait(cashierLineLock);
	}
	else if(cashierStatus == FREE) {
		cashierStatus = BUSY;
	}
	cashierLineLock->Release();
	cashierInteractionLock->Acquire();
	printf("Adult Patient [%d] reaches the Cashier.\n",i);
	printf("Adult Patient [%d] hands over his examination sheet to the Cashier.\n",i);
	cashierLine[position]=i;
	position++;
	cashierInteractionCV->Signal(cashierInteractionLock);
	cashierInteractionCV->Wait(cashierInteractionLock);
	
	printf("Adult Patient [%d] pays the Cashier $.... \n",i);
	cashierInteractionCV->Signal(cashierInteractionLock);
	cashierInteractionCV->Wait(cashierInteractionLock);
	
	printf("Adult Patient [%d] receives a receipt from the Cashier.\n",i);
	printf("Adult Patient [%d] leaves the doctor's office.\n",i);

	cashierInteractionCV->Signal(cashierInteractionLock);
	leaveLock->Acquire();
	leavingPatients++;
	leaveLock->Release();
	cashierInteractionLock->Release();
}

int count=0;

void nurse(int j)
{
	struct ExamSheet *myExamSheet=(struct ExamSheet *)malloc(sizeof(struct ExamSheet));
	int roomFound=0,roomAssigned=0;
	
	while(1)
	{	
		roomFound=0;
		roomAssigned=0;
		myExamSheet->status = FREE;
		
		for(int y=0;y<15;y++)
		{
			currentThread->Yield();
		}
		
		// Search for an Empty Room
		taskLock->Acquire();

		for(int k=0;k<no_of_rooms;k++)
		{
			if(examRoomStatus[k] == FREE)
			{
			//	printf("\n Nurse %d gets free room %d",j,k);
				examRoomStatus[k]=BUSY;
				roomList[j]=k;	//j-th nurse makes k-th room busy
				myExamSheet->room = k;
				roomFound=1;
				break;
			}
		}
		taskLock->Release();
		
		// If free Room Found & Patients Present, fetch one
		wrnLineLock->Acquire();
		if(roomFound == 1 && enteringPatientCnt <= (no_of_patients+no_of_parents))
		{
			enteringPatientCnt++;
			/* Talk to WRN after finding free room */
			if(wrnStatus == BUSY) {
			// Nurse Getting in Line
				nurseWaitingCount++;
				nurseWaitingCV->Wait(wrnLineLock);
			}
			else if(wrnStatus == FREE) {
				wrnStatus = BUSY;
			}
				
			/* Nurse was waiting in line for wrn. When wrn signals her, she has to come out of line.*/
			wrnLineLock->Release();
			wrnLock->Acquire();
			if(waitingRoomCnt > 0 )
			{
			//	printf("\n Nurse %d goes to Waiting Room",j);
				waitingRoomCnt--;
				currentNurse = j;
				wrnTaskStatus=SERVE_NURSE;
				nurseTask = FETCH_PATIENT;
				wrnWaitingCV->Signal(wrnLock);
				printf("Nurse [%d] tells Waiting Room Nurse to give a new examination sheet.\n",currentNurse);
				wrnWaitingCV->Wait(wrnLock);
			
				waitingRoomLock->Acquire();
			
				waitingRoomCV->Signal(waitingRoomLock);
				waitingRoomNurseCV->Wait(waitingRoomLock);
			
				if(patientExamSheet[currentPatient]->type == 0)
				{
					printf("Nurse [%d] escorts Adult Patient [%d] to the examination room [%d].\n",j,currentPatient,roomList[j]);
				}
				else
				{
					printf("Nurse [%d] escorts Parent [%d] to the examination room [%d].\n",j,currentPatient-no_of_patients,roomList[j]);
				}
				patientExamSheet[currentPatient]->room = roomList[j];
				patientExamSheet[currentPatient]->servingNurse = j;
				patientExamSheet[currentPatient]->room = roomList[j];
				*myExamSheet = *patientExamSheet[currentPatient];

				myExamSheet->room = roomList[j];
				
				waitingRoomNurseCV->Signal(waitingRoomLock);
				waitingRoomNurseCV->Wait(waitingRoomLock);
	
				examRoomLock[roomList[j]]->Acquire();
				roomAssigned=1;
				waitingRoomNurseCV->Signal(waitingRoomLock);
				waitingRoomLock->Release();
				wrnLock->Release();				
				
				examRoomCV[roomList[j]]->Wait(examRoomLock[roomList[j]]);
			
				if(patientExamSheet[myExamSheet->pid]->type == 0)
				{
					printf("Nurse:%d takes the temperature and blood pressure of Patient:%d\n",j,myExamSheet->pid);
					printf("Nurse:%d asks Patient:%d \"What Symptoms do you have?\"\n",j,myExamSheet->pid);
				}
				else
				{
					printf("Nurse:%d takes the temperature and blood pressure of Patient:%d\n",j,myExamSheet->pid-no_of_patients);
					printf("Nurse:%d asks Patient:%d \"What Symptoms do you have?\"\n",j,myExamSheet->pid-no_of_patients);
				}
				
				examRoomCV[roomList[j]]->Signal(examRoomLock[roomList[j]]);
				examRoomCV[roomList[j]]->Wait(examRoomLock[roomList[j]]);
				
				if(currentSymptom[myExamSheet->pid] == 0)
				{
					myExamSheet->symptom="Nausea";
					patientExamSheet[myExamSheet->pid]->symptom="Nausea";
				}
				else if(currentSymptom[myExamSheet->pid] == 1)
				{
					myExamSheet->symptom="Pain";
					patientExamSheet[myExamSheet->pid]->symptom="Pain";
				}
				else
				{
					myExamSheet->symptom="I hear alien voices";
					patientExamSheet[myExamSheet->pid]->symptom="I hear alien voices";
				}


				myExamSheet->status = WAITING_FOR_DOC;
				*patientExamSheet[myExamSheet->pid] = *myExamSheet;
				examRoomCV[roomList[j]]->Signal(examRoomLock[roomList[j]]);
				examRoomCV[roomList[j]]->Wait(examRoomLock[roomList[j]]);
				wallPocketLock->Acquire();				
				*wallPocket[roomList[j]]=*myExamSheet;	
				wallPocketLock->Release();
				//printf("Nurse %d releasing eRLock 1st loop",j);
				examRoomLock[roomList[j]]->Release();
			}
			else
			{
				wrnLock->Release();
			}
		}
		else
		{
			wrnLineLock->Release();
		}
		
		//Checks wallPockets
		wallPocketLock->Acquire();
		for(int t=0;t<no_of_rooms;t++)
		{
			if(wallPocket[t]->status==WAITING_FOR_DOC || wallPocket[t]->status == WAITING_FOR_DOC_AGAIN || wallPocket[t]->status == NEED_NURSE || 	wallPocket[t]->status == NEED_NURSE_AGAIN)
			{
				// Make Earlier Room Free
				taskLock->Acquire();
				if(roomAssigned == 0 && roomFound == 1)
				{
			//		printf("\n Room %d  %d made free 1 by %d",roomList[j],t,j);
					examRoomStatus[roomList[j]]= FREE;
				}
				taskLock->Release();
			//	printf("\n Nurse %d checks wallPockets",j);
				roomList[j]=t;
				*myExamSheet = *wallPocket[t];
				wallPocket[t]->status = OCCUPIED;
				myExamSheet->room=roomList[j];
				break;
			}
			else if(xRayWaitingRoomWallPocket[xRayRoomCnt_atNurse]->status == WAIT_FOR_NURSE && roomAssigned == 0 && roomFound==1)
			{
			//	printf("\n Nurse %d found patient in xray",j);
				*myExamSheet = *xRayWaitingRoomWallPocket[xRayRoomCnt_atNurse];
				myExamSheet->room = roomList[j];
				xRayWaitingRoomWallPocket[xRayRoomCnt_atNurse]->status = FREE;
				xRayRoomCnt_atNurse++;
				break;
			}
		}
		wallPocketLock->Release();
		
		docLineLock->Acquire();
		if(myExamSheet->status == WAITING_FOR_DOC || myExamSheet->status == WAITING_FOR_DOC_AGAIN)
		{
			if(docCount>0)
			{
			//	printf("\n Nurse %d checks for Doc",j);
				for(int x=0;x<no_of_doctors;x++)
				{
					if(docStatus[x]==FREE)
					{
						currentDoc[j]=x;
						docStatus[x]=BUSY;
						break;
					}
				}
				if(myExamSheet->type == 0)
				{
					printf("Nurse [%d] informs Doctor [%d] that Adult Patient [%d] is waiting in the examination room [%d].\n",j,currentDoc[j],myExamSheet->pid,roomList[j]);
				}
				else
				{
					printf("Nurse [%d] informs Doctor [%d] that Child Patient [%d] is waiting in the examination room [%d].\n",j,currentDoc[j],myExamSheet->pid-no_of_patients,roomList[j]);
				}
				myExamSheet->servingNurse=j;
				patientExamSheet[myExamSheet->pid]->servingNurse = j;
				patList[currentDoc[j]]=myExamSheet->pid;
			
				if(myExamSheet->type == 0)
				{
					printf("Nurse [%d] hands over to the Doctor [%d] the examination sheet of Adult Patient [%d].\n",j,currentDoc[j],myExamSheet->pid);
				}
				else
				{
					printf("Nurse [%d] hands over to the Doctor [%d] the examination sheet of Child Patient [%d].\n",j,currentDoc[j],myExamSheet->pid-no_of_patients);
				}
				if(myExamSheet->status == WAITING_FOR_DOC)
				{
					patientExamSheet[myExamSheet->pid]->status = FIRST_TIME;
				}
				else if(myExamSheet->status == WAITING_FOR_DOC_AGAIN)
				{
					//printf("\nNurse %d sets status for %d",j,myExamSheet->pid);
					patientExamSheet[myExamSheet->pid]->status = SECOND_TIME;
				}
				myExamSheet->status = OCCUPIED;
				docLineCV[currentDoc[j]]->Signal(docLineLock);
				docCount--;				
				nurseLineCV[j]->Wait(docLineLock);
			}
			else
			{
				wallPocketLock->Acquire();
				if(wallPocket[myExamSheet->room]->status == OCCUPIED)
				*wallPocket[myExamSheet->room] = *myExamSheet;	
				wallPocketLock->Release();
			}
		}
		docLineLock->Release();
		
		for(int t=0;t<15;t++)
		{
			currentThread->Yield();
		}
		
		if(roomList[j] != -1)
		{
			examRoomLock[roomList[j]]->Acquire();		
			//printf("\nNurse %d acquired lock on room :%d myExamSheet %d",j,roomList[j],myExamSheet->room);
			if(myExamSheet->status == NEED_NURSE)
			{
			//printf("\n\nNurse:%d has acquired lock on room:%d",j,myExamSheet->room);
				myExamSheet->status=OCCUPIED;
				myExamSheet->servingNurse = j;
				patientExamSheet[myExamSheet->pid]->servingNurse = j;
				patientExamSheet[myExamSheet->pid]->room = roomList[j];
			
				if(strcmp(myExamSheet->treatment,"XRAY") == 0)
				{
				//	printf("\nNurse %d finds pat %d in room %d",j,myExamSheet->pid,myExamSheet->room);
					examRoomCV[myExamSheet->room]->Signal(examRoomLock[myExamSheet->room]);
					examRoomCV[roomList[j]]->Wait(examRoomLock[roomList[j]]);
			
					xRayLineLock->Acquire();
					if(no_of_xRayTechnicians == 2)
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
					if(no_of_xRayTechnicians == 1)
					{
						tempXrayRoom = 0;
					}
					patientExamSheet[myExamSheet->pid]->xRayRoom = tempXrayRoom;
					xRayLineLock->Release();
					examRoomCV[myExamSheet->room]->Signal(examRoomLock[myExamSheet->room]);
					examRoomCV[myExamSheet->room]->Wait(examRoomLock[myExamSheet->room]);
					//Added
					taskLock->Acquire();
					//	printf("\n Room %d made free 2",roomList[j]);
						examRoomStatus[roomList[j]]= FREE;
					taskLock->Release();
					myExamSheet->status=FREE;
					examRoomLock[myExamSheet->room]->Release();
				}
				else if(strcmp(myExamSheet->treatment,"SHOT") == 0)
				{
					if(myExamSheet->type == 0)
					{
						printf("Nurse [%d] goes to supply cabinet to give to take medicine for Adult Patient [%d].\n",j,myExamSheet->pid);
						printf("Nurse [%d] asks Adult Patient [%d] \"Are you ready for the shot?\"\n",j,myExamSheet->pid);
					}
					else
					{
						printf("Nurse [%d] goes to supply cabinet to give to take medicine for Child Patient [%d].\n",j,myExamSheet->pid-no_of_patients);
						printf("Nurse [%d] asks Child Patient [%d] \"Are you ready for the shot?\"\n",j,myExamSheet->pid-no_of_patients);
					}
					examRoomCV[myExamSheet->room]->Signal(examRoomLock[myExamSheet->room]);
					examRoomCV[roomList[j]]->Wait(examRoomLock[roomList[j]]);
					if(patientExamSheet[myExamSheet->pid]->type == 0)
					{
						printf("Nurse [%d] tells Adult Patient [%d] \"Your shot is over.\"\n",j,myExamSheet->pid);
						printf("Nurse [%d] escorts Adult Patient [%d] to Cashier.\n",j,myExamSheet->pid);
					}
					else
					{
						printf("Nurse [%d] tells Child Patient [%d] \"Your shot is over.\"\n",j,myExamSheet->pid-no_of_patients);
						printf("Nurse [%d] escorts Parent [%d] to Cashier.\n",j,myExamSheet->pid-no_of_patients);
					}
					examRoomCV[myExamSheet->room]->Signal(examRoomLock[myExamSheet->room]);
					examRoomCV[roomList[j]]->Wait(examRoomLock[roomList[j]]);				
					//Added	
					taskLock->Acquire();
					//	printf("\n Room %d made free 3",roomList[j]);
						examRoomStatus[roomList[j]]= FREE;
					taskLock->Release();
					myExamSheet->status=FREE;
					examRoomLock[myExamSheet->room]->Release();
				}
				else if(strcmp(myExamSheet->treatment,"FINE") == 0)
				{
					if(myExamSheet->type == 0)
					{
						printf("Nurse [%d] escorts Adult Patient [%d] to Cashier.\n",j,myExamSheet->pid);
					}
					else
					{
						printf("Nurse [%d] escorts Parent [%d] to Cashier.\n",j,myExamSheet->pid-no_of_patients);
					}
					examRoomCV[myExamSheet->room]->Signal(examRoomLock[myExamSheet->room]);
					examRoomCV[roomList[j]]->Wait(examRoomLock[roomList[j]]);				
					//Added	
					taskLock->Acquire();
					//	printf("\n Room %d made free 4",roomList[j]);
						examRoomStatus[roomList[j]]= FREE;
					taskLock->Release();
					myExamSheet->status=FREE;
				//	printf("\nNurse %d releasing eRLock Fine loop",j);
					examRoomLock[myExamSheet->room]->Release();
				}
			}
			else if(myExamSheet->status == NEED_NURSE_AGAIN)
			{
				myExamSheet->servingNurse = j;
				patientExamSheet[myExamSheet->pid]->servingNurse = j;
				myExamSheet->status=WITH_NURSE;
			
				if(myExamSheet->type == 0)
				{
					printf("Nurse [%d] escorts Adult Patient [%d] to Cashier.\n",j,myExamSheet->pid);
				}
				else
				{
					printf("Nurse [%d] escorts Parent [%d] to Cashier.\n",j,myExamSheet->pid-no_of_patients);
				}
				examRoomCV[myExamSheet->room]->Signal(examRoomLock[myExamSheet->room]);
				examRoomCV[myExamSheet->room]->Wait(examRoomLock[myExamSheet->room]);
				//printf("\n After Wait");
				myExamSheet->status=FREE;
				//Added	
				taskLock->Acquire();
					//printf("\n Room %d made free 5",roomList[j]);
					examRoomStatus[roomList[j]]= FREE;
				taskLock->Release();
			//	printf("Nurse %d releasing eRLock NeedNurse loop",j);
				examRoomLock[myExamSheet->room]->Release();
			}
			else
			{
				//printf("\nNurse %d releasing lock %d",j,myExamSheet->room);
				examRoomLock[myExamSheet->room]->Release();
			}		
		}
		
		xRayWaitingRoomLock->Acquire();
		if(myExamSheet->status == WAIT_FOR_NURSE && patientPresent > 0)
		{
		//	printf("\n Nurse %d goes to Xray Waiting Room",j);
			myExamSheet->status = WITH_NURSE;
			myExamSheet->servingNurse = j;
			myExamSheet->room = roomList[j];
		//	printf("\nroomList[%d]=%d ",j,roomList[j]);
			if(myExamSheet->type == 0)
			{
				printf("Nurse [%d] escorts Adult Patient[%d] to the examination room [%d].\n",j,myExamSheet->pid,myExamSheet->room);
			}
			else
			{
				printf("Nurse [%d] escorts Parent[%d] to the examination room [%d].\n",j,myExamSheet->pid-no_of_patients,myExamSheet->room);
			}
			
			*patientExamSheet[myExamSheet->pid] = *myExamSheet;

		//	printf("\nSignal by Nurse :%d",j);
			
			xRayWaitingRoomCV->Signal(xRayWaitingRoomLock);
			xRayWaitingRoomNurseCV->Wait(xRayWaitingRoomLock);
			myExamSheet->status = WAITING_FOR_DOC_AGAIN;
			wallPocketLock->Acquire();
			*wallPocket[myExamSheet->room] = *myExamSheet;
			wallPocketLock->Release();
		}
		xRayWaitingRoomLock->Release();
		
		leaveLock->Acquire();
		if(leavingPatients==(no_of_patients+no_of_parents))
		{
			//printf("\n Leaving Pat%d",leavingPatients);
			leaveLock->Release();
			break;
		}
		leaveLock->Release();
	}
}

void doctor(int d)
{
	struct ExamSheet *examSheetDocCopy = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));
	
	while(1)
	{
		for(int y=0;y<15;y++)
		{
			currentThread->Yield();
		}

		docLineLock->Acquire();
		docCount++;
		docStatus[d]=FREE;

		docLineCV[d]->Wait(docLineLock);
		
		/* Doctor interacts with Nurse */
		
		*examSheetDocCopy=*patientExamSheet[patList[d]];
				

		nurseLineCV[examSheetDocCopy->servingNurse]->Signal(docLineLock);
			
		//printf("\n\nPatientId in Doctor:%d",examSheetDocCopy->pid);
	
		docLineLock->Release();
	
		/* Interaction with Patient Starts */		
		examRoomLock[examSheetDocCopy->room]->Acquire();

		if(examSheetDocCopy->status == SECOND_TIME)
		{
			tempRoomLock[examSheetDocCopy->room]->Acquire();
			if(examSheetDocCopy->type == 0)
			{
				printf("Doctor [%d] is examining the Xrays of [Adult] Patient [%d] in Examination room [%d].\n",d,examSheetDocCopy->pid,examSheetDocCopy->room);
			}
			else
			{
				printf("Doctor [%d] is examining the Xrays of [Child] Patient [%d] in Examination room [%d].\n",d,examSheetDocCopy->pid-no_of_patients,examSheetDocCopy->room);
			}
			printf("Doctor [%d] has left Examination Room [%d].\n",d,examSheetDocCopy->room);
			printf("Doctor [%d] is going to their office.\n",d);
			*patientExamSheet[examSheetDocCopy->pid]=*examSheetDocCopy;
			patientExamSheet[examSheetDocCopy->pid]->examiningDoctor=d;
			
			examRoomLock[examSheetDocCopy->room]->Release(); 	
			tempRoomLock[examSheetDocCopy->room]->Acquire();	
			wallPocketLock->Acquire();
			*wallPocket[examSheetDocCopy->room]=*examSheetDocCopy;
			wallPocket[examSheetDocCopy->room]->status = NEED_NURSE_AGAIN;
			wallPocketLock->Release();
			tempRoomLock[examSheetDocCopy->room]->Release();
		}
		else
		{
			if(examSheetDocCopy->type == 0)
			{
				printf("Doctor [%d] is reading the examination sheet of [Adult] Patient [%d] in Examination room [%d].\n",d,examSheetDocCopy->pid,examSheetDocCopy->room);		
			}
			else
			{
				printf("Doctor [%d] is reading the examination sheet of [Child] Patient [%d] in Examination room [%d].\n",d,examSheetDocCopy->pid-no_of_patients,examSheetDocCopy->room);	
			}		
	/*			Doctor randomly chooses one of the following:
				Patient needs a shot
				Patient needs an X-Ray
				Patient is fine.
	*/		
		
			int randomNumber;
			srand(time(0));
			randomNumber = random()%100;
		
			/* There is a 25% chance that a Patient will need an Xray */
			if(randomNumber >= 0 && randomNumber <= 25)
			{
				if(examSheetDocCopy->type == 0)
				{
					printf("Doctor [%d] notes down in the sheet that Xray is needed for Adult Patient [%d] in Examination room [%d].\n",d,examSheetDocCopy->pid,examSheetDocCopy->room);
				}
				else
				{
					printf("Doctor [%d] notes down in the sheet that Xray is needed for Child Patient [%d] in Examination room [%d].\n",d,examSheetDocCopy->pid-no_of_patients,examSheetDocCopy->room);
				}
				examSheetDocCopy->treatment="XRAY";
				patientExamSheet[examSheetDocCopy->pid]->treatment="XRAY";
			
				srand(time(0));
				int xrayRandomNumber = random()%100;
				if(xrayRandomNumber >= 0 && xrayRandomNumber <= 33)
				{
					examSheetDocCopy->no_of_images = 1;
					patientExamSheet[examSheetDocCopy->pid]->no_of_images=1;
				}
				if(xrayRandomNumber >= 34 && xrayRandomNumber <= 67)
				{
					examSheetDocCopy->no_of_images = 2;
					patientExamSheet[examSheetDocCopy->pid]->no_of_images=2;
				}
				if(xrayRandomNumber >= 68 && xrayRandomNumber <= 100)
				{
					examSheetDocCopy->no_of_images = 3;
					patientExamSheet[examSheetDocCopy->pid]->no_of_images=3;
				}
			}
		
			/* There is a separate 25% chance that a Patient will need a shot.*/
			if(randomNumber >= 75 && randomNumber <= 100)
			{
				if(examSheetDocCopy->type == 0)
				{			
					printf("Doctor [%d] notes down in the sheet that Adult Patient [%d] needs to be given a shot in Examination room [%d].\n",d,examSheetDocCopy->pid,examSheetDocCopy->room);
				}
				else
				{
					printf("Doctor [%d] notes down in the sheet that Child Patient [%d] needs to be given a shot in Examination room [%d].\n",d,examSheetDocCopy->pid-no_of_patients,examSheetDocCopy->room);
				}
				examSheetDocCopy->treatment="SHOT";
				patientExamSheet[examSheetDocCopy->pid]->treatment="SHOT";
			}
			if(randomNumber >25 && randomNumber <75)
			{
				if(examSheetDocCopy->type == 0)
				{
					printf("Doctor [%d] diagnoses Adult Patient [%d] to be fine and is leaving Examination Room [%d].\n",d,examSheetDocCopy->pid,examSheetDocCopy->room);
				}
				else
				{
					printf("Doctor [%d] diagnoses Child Patient [%d] to be fine and is leaving Examination Room [%d].\n",d,examSheetDocCopy->pid-no_of_patients,examSheetDocCopy->room);
				}
				examSheetDocCopy->treatment="FINE";
				patientExamSheet[examSheetDocCopy->pid]->treatment="FINE";
			}
			*patientExamSheet[examSheetDocCopy->pid]=*examSheetDocCopy;
			patientExamSheet[examSheetDocCopy->pid]->examiningDoctor=d;
		
			examRoomCV[examSheetDocCopy->room]->Signal(examRoomLock[examSheetDocCopy->room]);
			examRoomCV[examSheetDocCopy->room]->Wait(examRoomLock[examSheetDocCopy->room]);	
		
			tempRoomLock[examSheetDocCopy->room]->Acquire();
		
			printf("Doctor [%d] has left Examination Room [%d].\n",d,examSheetDocCopy->room);
			printf("Doctor [%d] is going to their office.\n",d);
		
			examRoomCV[examSheetDocCopy->room]->Signal(examRoomLock[examSheetDocCopy->room]);
		//	printf("\n\n Before Wait");
			examRoomCV[examSheetDocCopy->room]->Wait(examRoomLock[examSheetDocCopy->room]);
		//	printf("\n\n After Wait");

			examRoomLock[examSheetDocCopy->room]->Release(); 
			wallPocketLock->Acquire();
			examSheetDocCopy->status=NEED_NURSE;
			*wallPocket[examSheetDocCopy->room]=*examSheetDocCopy;
			wallPocketLock->Release();
			tempRoomLock[examSheetDocCopy->room]->Release();
		}
	}	
}

void xrayTechnician(int x)
{
	struct ExamSheet *examSheetXrayCopy = (struct ExamSheet *)malloc(sizeof(struct ExamSheet)); 

	while(1)
	{
		xRayLineLock->Acquire();
		
		if(patientWaitingForXrayCount[x] > 0)
		{
			xRayLineCV[x]->Signal(xRayLineLock);
			patientWaitingForXrayCount[x]--;
		}
		else
		{
			xRayStatus[x] = FREE;
		}
		xRayInteractionLock[x]->Acquire();
		xRayLineLock->Release();
		xRayInteractionCV[x]->Wait(xRayInteractionLock[x]);
		
		*examSheetXrayCopy = *patientExamSheet[xRayPatientList[x]];
	//	patientExamSheet[xRayPatientList[x]]->xRayRoom = x;
	//	xRayInteractionNurseCV[x]->Signal(xRayInteractionLock[x]);
	//	xRayInteractionPatCV[x]->Wait(xRayInteractionLock[x]);
		
		if(examSheetXrayCopy->type == 0)
		{
			printf("Xray technician [%d] asks Adult Patient [%d] to get on the table.\n",x,examSheetXrayCopy->pid);
		}
		else
		{
			printf("Xray technician [%d] asks Child Patient [%d] to get on the table.\n",x,examSheetXrayCopy->pid-no_of_patients);
		}
		for(int t=0;t<examSheetXrayCopy->no_of_images;t++)
		{
			if(examSheetXrayCopy->type == 0)
			{
				printf("Xray Technician [%d] asks Adult Patient [%d] to move.\n",x,examSheetXrayCopy->pid);
				printf("Xray Technician [%d] takes an Xray Image of Adult Patient [%d].\n",x,examSheetXrayCopy->pid);
			}
			else
			{
				printf("Xray Technician [%d] asks Child Patient [%d] to move.\n",x,examSheetXrayCopy->pid-no_of_patients);
				printf("Xray Technician [%d] takes an Xray Image of Child Patient [%d].\n",x,examSheetXrayCopy->pid-no_of_patients);			
			}
			int randomResult;
			srand(time(0));
			randomResult = random()%3;
			if(randomResult == 0)
			{
				if(examSheetXrayCopy->type == 0)
				{
					printf("Xray Technician [%d] records [nothing] on Adult Patient [%d]'s examination sheet.\n",x,examSheetXrayCopy->pid);
				}
				else
				{
					printf("Xray Technician [%d] records [nothing] on Child Patient [%d]'s examination sheet.\n",x,examSheetXrayCopy->pid-no_of_patients);
				}
				examSheetXrayCopy->xRayResult = "nothing";
			}
			if(randomResult == 1)
			{
				if(examSheetXrayCopy->type == 0)
				{
					printf("Xray Technician [%d] records [break] on Adult Patient [%d]'s examination sheet.\n",x,examSheetXrayCopy->pid);
				}
				else
				{
					printf("Xray Technician [%d] records [break] on Child Patient [%d]'s examination sheet.\n",x,examSheetXrayCopy->pid-no_of_patients);
				}
				examSheetXrayCopy->xRayResult = "break";
			}
			if(randomResult == 2)
			{
				if(examSheetXrayCopy->type == 0)
				{
					printf("Xray Technician [%d] records [compound fracture] on Adult Patient [%d]'s examination sheet.\n",x,examSheetXrayCopy->pid);
				}
				else
				{
					printf("Xray Technician [%d] records [compound fracture] on Child Patient [%d]'s examination sheet.\n",x,examSheetXrayCopy->pid-no_of_patients);
				}
				examSheetXrayCopy->xRayResult = "compound fracture";
			}
						
		}
		patientExamSheet[examSheetXrayCopy->pid]->xRayResult = examSheetXrayCopy->xRayResult;
		if(examSheetXrayCopy->type == 0)
		{
			printf("X-ray Technician [%d] tells [Adult Patient] [%d] to wait in Xray waiting room.\n",x,examSheetXrayCopy->pid);
			printf("X-ray Technician [%d] puts [Adult Patient] [%d] in Xray waiting room wall pocket.\n",x,examSheetXrayCopy->pid);
		}
		else
		{
			printf("X-ray Technician [%d] tells [Child] [%d] to wait in Xray waiting room.\n",x,examSheetXrayCopy->pid-no_of_patients);
			printf("X-ray Technician [%d] puts [Child] [%d] in Xray waiting room wall pocket.\n",x,examSheetXrayCopy->pid-no_of_patients);		
		}
		wallPocketLock->Acquire();
		//printf("\n Changed status of XrayWaitingRoom for %d ",examSheetXrayCopy->pid);
		examSheetXrayCopy->status = WAIT_FOR_NURSE;
		*xRayWaitingRoomWallPocket[xRayRoomCnt_atXray] = *examSheetXrayCopy;
		xRayRoomCnt_atXray++;
		wallPocketLock->Release();
		xRayInteractionPatCV[x]->Signal(xRayInteractionLock[x]);	
		xRayInteractionPatCV[x]->Wait(xRayInteractionLock[x]);
		
		xRayInteractionLock[x]->Release();
	}
}

void cashier()
{
	int amount;
	struct ExamSheet *examSheetCashierCopy = (struct ExamSheet *)malloc(sizeof(struct ExamSheet)); 
	
	while(1)
	{
		for(int y=0;y<15;y++)
		{
			currentThread->Yield();
		}
	
		cashierLineLock->Acquire();
		if(patientCashierWaitingCount == 0)
		{
			 cashierStatus=FREE;
		}
		else
		{
			cashierLineCV->Signal(cashierLineLock);
			patientCashierWaitingCount--;
		}
		
		cashierInteractionLock->Acquire();
		cashierLineLock->Release();
		cashierInteractionCV->Wait(cashierInteractionLock);	
		
		*examSheetCashierCopy = *patientExamSheet[cashierLine[myPosition]];
		myPosition++;
		if(examSheetCashierCopy->type == 0)
		{
			printf("Cashier receives the examination sheet from Adult Patient [%d].\n",examSheetCashierCopy->pid);
		}
		else
		{
			printf("Cashier receives the examination sheet for Child Patient [%d] from Parent [%d].\n",examSheetCashierCopy->pid-no_of_patients,examSheetCashierCopy->pid-no_of_patients);
		}
		
		srand(time(0));
		if(strcmp(examSheetCashierCopy->treatment,"XRAY"))
		{
			amount = random()%1000;
		
		}
		else if(strcmp(examSheetCashierCopy->treatment,"SHOT"))
		{
			amount = random()%500;
		}
		else
		{
			amount = random()%100;
		}
		
		if(examSheetCashierCopy->type == 0)
		{
			printf("Cashier reads the examination sheet of Adult Patient [%d] and asks him to pay $[%d]\n",examSheetCashierCopy->pid,amount);
		}
		else
		{
			printf("Cashier reads the examination sheet of Child Patient [%d] and asks Parent [%d] to pay $[%d]\n",examSheetCashierCopy->pid-no_of_patients,examSheetCashierCopy->pid-no_of_patients,amount);
		}
		cashierInteractionCV->Signal(cashierInteractionLock);	
		cashierInteractionCV->Wait(cashierInteractionLock);	

		if(examSheetCashierCopy->type == 0)
		{
			printf("Cashier accepts $[%d] from Adult Patient [%d].\n",amount,examSheetCashierCopy->pid);
			printf("Cashier gives a receipt of $[%d] to Adult Patient [%d].\n",amount,examSheetCashierCopy->pid);
		}
		else
		{
			printf("Cashier accepts $[%d] from Parent [%d].\n",amount,examSheetCashierCopy->pid-no_of_patients);
			printf("Cashier gives a receipt of $[%d] to Parent [%d].\n",amount,examSheetCashierCopy->pid-no_of_patients);		
		}
		cashierInteractionCV->Signal(cashierInteractionLock);	
		//printf("\nCashier b4 wait");
		cashierInteractionCV->Wait(cashierInteractionLock);		
		//printf("\nCashier after wait");
		cashierInteractionLock->Release();
	}
}

void parent(int P)
{	
	familyLock[P]->Acquire();	
	printf("Parent [%d] has entered the Doctor's Office Waiting Room with Child Patient [%d].\n",P-no_of_patients,P-no_of_patients);
	wrnLineLock->Acquire();
	if(wrnStatus == BUSY) {
		//If WRN is busy,patient enters line
		printf("Parent [%d] gets in line of the Waiting Room Nurse to get registration form.\n",P-no_of_patients);
		patientWaitingCount++;
		patientWaitingCV->Wait(wrnLineLock);
	}
	else if(wrnStatus == FREE) {
		wrnStatus = BUSY;
	}

	/* Patient was waiting in line for wrn. When wrn signals him, he has to come out of line.*/
	//enteringPatientCnt++;
	wrnLineLock->Release();
	
	/* Interaction with wrn begins */
	wrnLock->Acquire();
	wrnTaskStatus=SERVE_PATIENT;
	patientTask = GETFORM;
	patientGettingForm=P;
	/* Tell the wrn that i have acquired the form */
	wrnWaitingCV->Signal(wrnLock);
	wrnWaitingCV->Wait(wrnLock);
	/* After Receiving Blank sheet from wrn */
	wrnLock->Release();

	/* After Getting out of Line. Patient Fills out the form */
	infoLock->Acquire();
	
	// Allocate random values of age & name to patient

	srand(time(0));
	patientExamSheet[P]->age = random()%100 + 1;
	char temp[10];		//Temporary array used to initialize name
	 for(int k1=0;k1<10;k1++) {
	        temp[k1] = randomChar();  
        }
        patientExamSheet[P]->name = temp;
        patientExamSheet[P]->pid = P;
	patientExamSheet[P]->room = -1;
	patientExamSheet[P]->status = -1;
        
        infoLock->Release();
        /* Give some delay for patient to enter his info */ 
        for(int k2=0;k2<10;k2++) {
        	currentThread->Yield();
        }
        /* Patient Re-enters the waiting Queue */
        wrnLineLock->Acquire();
        
	if(wrnStatus == BUSY) {
		printf("Parent [%d] gets in line of the Waiting Room Nurse to submit registration form.\n",P-no_of_patients);
		patientWaitingCount++;
		patientWaitingCV->Wait(wrnLineLock);
	}
	else if(wrnStatus == FREE)	{
		wrnStatus = BUSY;
	}
	
	/* Patient was waiting in line for wrn. When wrn signals him, he has to come out of line.*/
	wrnLineLock->Release();
	
	/* Interacts with Wrn again */
	wrnLock->Acquire();
	wrnTaskStatus=SERVE_PATIENT;
	patientTask=GIVEFORM;
        patientGivingForm=P;

        /* Tell wrn that i have to submit the form*/
	printf("Parent [%d] submits the filled form to the Waiting Room Nurse.\n",P-no_of_patients);
       	wrnWaitingCV->Signal(wrnLock);
	wrnWaitingCV->Wait(wrnLock);
		
	/*Now patient has to wait in waiting Room. He acquires Waiting for Nurse Lock & Waits on it */

	waitingRoomLock->Acquire();
	waitingRoomCnt++;
	wrnLock->Release();	
	waitingRoomCV->Wait(waitingRoomLock);
	/* Patient Nurse Interaction Begins */

	currentPatient = P;

	waitingRoomNurseCV->Signal(waitingRoomLock);
	waitingRoomNurseCV->Wait(waitingRoomLock);
	
	printf("Parent [%d] asks Child Patient [%d] to follow him to Examination Room [%d].\n",P-no_of_patients,P-no_of_patients,patientExamSheet[P]->room);	
	printf("Parent [%d] is following Nurse [%d] to Examination Room [%d].\n",P-no_of_patients,patientExamSheet[P]->servingNurse,patientExamSheet[P]->room);	
	parentSignalled[P]=1;
	familyCV[P]->Signal(familyLock[P]);
	familyCV[P]->Wait(familyLock[P]);
	
	waitingRoomNurseCV->Signal(waitingRoomLock);
	waitingRoomNurseCV->Wait(waitingRoomLock);
	
	examRoomLock[patientExamSheet[P]->room]->Acquire();
	waitingRoomLock->Release();
	
	examRoomCV[patientExamSheet[P]->room]->Signal(examRoomLock[patientExamSheet[P]->room]);
	examRoomCV[patientExamSheet[P]->room]->Wait(examRoomLock[patientExamSheet[P]->room]);
	
	srand(time(0));
	currentSymptom[P] = random()%3;	//Assign Random Symptoms to Patient
	if(currentSymptom[P]==0)
	{
		printf("Parent:%d asks Child Patient:%d \"What Symptoms do you have?\"\n",P-no_of_patients,P-no_of_patients);
		familyCV[P]->Signal(familyLock[P]);
		familyCV[P]->Wait(familyLock[P]);
//		printf("\nAdult Patient [%d] says, “My symptoms are Nausea.\"",P);
	}
	else if(currentSymptom[P]==1)
	{
		printf("Parent:%d asks Child Patient:%d \"What Symptoms do you have?\"\n",P-no_of_patients,P-no_of_patients);
		familyCV[P]->Signal(familyLock[P]);
		familyCV[P]->Wait(familyLock[P]);
//		printf("\nAdult Patient [%d] says, “My symptoms are Pain.\"",P);
	}
	else if(currentSymptom[P]==2)
	{
		printf("Parent:%d asks Child Patient:%d \"What Symptoms do you have?\"\n",P-no_of_patients,P-no_of_patients);
		familyCV[P]->Signal(familyLock[P]);
		familyCV[P]->Wait(familyLock[P]);
//		printf("\nAdult Patient [%d] says, “My symptoms are I Hear Alien Voices.\"",P);
	}
	examRoomCV[patientExamSheet[P]->room]->Signal(examRoomLock[patientExamSheet[P]->room]);
	examRoomCV[patientExamSheet[P]->room]->Wait(examRoomLock[patientExamSheet[P]->room]);
	
	
	examRoomCV[patientExamSheet[P]->room]->Signal(examRoomLock[patientExamSheet[P]->room]);
	examRoomCV[patientExamSheet[P]->room]->Wait(examRoomLock[patientExamSheet[P]->room]);
	
	if(strcmp(patientExamSheet[P]->treatment,"XRAY") == 0)
	{
	//	printf("\nAdult Patient [%d] has been informed by Doctor [%d] that he needs an Xray.",P,patientExamSheet[P]->examiningDoctor);
	}
	else if(strcmp(patientExamSheet[P]->treatment,"SHOT") == 0)
	{
	//	printf("\nAdult Patient [%d] has been informed by Doctor [%d] that he will be administered a shot.",P,patientExamSheet[P]->examiningDoctor);
	}
	else if(strcmp(patientExamSheet[P]->treatment,"FINE") == 0)
	{
	//	printf("\nAdult Patient [%d] has been diagnosed by Doctor [%d].",P,patientExamSheet[P]->examiningDoctor);
	}
	familyCV[P]->Signal(familyLock[P]);
	familyCV[P]->Wait(familyLock[P]);

	examRoomCV[patientExamSheet[P]->room]->Signal(examRoomLock[patientExamSheet[P]->room]);
	examRoomCV[patientExamSheet[P]->room]->Wait(examRoomLock[patientExamSheet[P]->room]);
	
//	leavingPatients++;
	examRoomCV[patientExamSheet[P]->room]->Signal(examRoomLock[patientExamSheet[P]->room]);
	//printf("\nBefore pat %d waits on ERL %d",P,patientExamSheet[P]->room);
	examRoomCV[patientExamSheet[P]->room]->Wait(examRoomLock[patientExamSheet[P]->room]);
	/* Wait for Nurse to come see patient */
	
	if(strcmp(patientExamSheet[P]->treatment,"XRAY") == 0)
	{
		printf("Parent [%d] waits for a Nurse to escort them to the Xray room.\n",P-no_of_patients);

		examRoomCV[patientExamSheet[P]->room]->Signal(examRoomLock[patientExamSheet[P]->room]);
		examRoomCV[patientExamSheet[P]->room]->Wait(examRoomLock[patientExamSheet[P]->room]);
		
		xRayLineLock->Acquire();
		examRoomCV[patientExamSheet[P]->room]->Signal(examRoomLock[patientExamSheet[P]->room]);
		examRoomLock[patientExamSheet[P]->room]->Release();
		
		if(xRayStatus[patientExamSheet[P]->xRayRoom] == BUSY)
		{
			patientWaitingForXrayCount[patientExamSheet[P]->xRayRoom]++;
			xRayLineCV[patientExamSheet[P]->xRayRoom]->Wait(xRayLineLock);
		}
		else if(xRayStatus[patientExamSheet[P]->xRayRoom] == FREE)
		{
			xRayStatus[patientExamSheet[P]->xRayRoom] = BUSY;
		}
		xRayInteractionLock[patientExamSheet[P]->xRayRoom]->Acquire();
		xRayLineLock->Release();
		xRayPatientList[patientExamSheet[P]->xRayRoom] = P;

		xRayInteractionCV[patientExamSheet[P]->xRayRoom]->Signal(xRayInteractionLock[patientExamSheet[P]->xRayRoom]);
		xRayInteractionPatCV[patientExamSheet[P]->xRayRoom]->Wait(xRayInteractionLock[patientExamSheet[P]->xRayRoom]);		
					
	//	Added
		xRayWaitingRoomLock->Acquire();
		printf("Parent [%d] waits for a Nurse to escort him/her to examination room.\n",P-no_of_patients);

		patientPresent++;
		xRayInteractionPatCV[patientExamSheet[P]->xRayRoom]->Signal(xRayInteractionLock[patientExamSheet[P]->xRayRoom]);
		xRayInteractionLock[patientExamSheet[P]->xRayRoom]->Release();

		xRayWaitingRoomCV->Wait(xRayWaitingRoomLock);
		printf("Parent [%d] asks Child [%d] to follow him/her\n",P-no_of_patients,P-no_of_patients);
		printf("Parent [%d] is following Nurse [%d] to Examination Room [%d].\n",P-no_of_patients,patientExamSheet[P]->servingNurse,patientExamSheet[P]->room);
	
		familyCV[P]->Signal(familyLock[P]);	
		familyCV[P]->Wait(familyLock[P]);

		xRayWaitingRoomNurseCV->Signal(xRayWaitingRoomLock);
		examRoomLock[patientExamSheet[P]->room]->Acquire();
		xRayWaitingRoomLock->Release();
		
		examRoomCV[patientExamSheet[P]->room]->Wait(examRoomLock[patientExamSheet[P]->room]);
	}
	else if(strcmp(patientExamSheet[P]->treatment,"SHOT") == 0)
	{
		printf("Parent [%d] asks Child Patient [%d] \"Are you ready for the shot?\"\n",P-no_of_patients,P-no_of_patients);

		familyCV[P]->Signal(familyLock[P]);	
		familyCV[P]->Wait(familyLock[P]);

//		printf("\nAdult Patient [%d] says, \"Yes I am ready for the shot\".",P);
		examRoomCV[patientExamSheet[P]->room]->Signal(examRoomLock[patientExamSheet[P]->room]);
		examRoomCV[patientExamSheet[P]->room]->Wait(examRoomLock[patientExamSheet[P]->room]);				
	}
	else if(strcmp(patientExamSheet[P]->treatment,"FINE") == 0)
	{
	}
	examRoomCV[patientExamSheet[P]->room]->Signal(examRoomLock[patientExamSheet[P]->room]);
	printf("Parent [%d] asks Child Patient [%d] to follow him to Cashier\n",P-no_of_patients,P-no_of_patients);

	familyCV[P]->Signal(familyLock[P]);	
	familyCV[P]->Wait(familyLock[P]);

	printf("Parent [%d] enters the queue for Cashier\n",P-no_of_patients);
	cashierLineLock->Acquire();
	examRoomLock[patientExamSheet[P]->room]->Release();
	if(cashierStatus == BUSY) {
		//If Cashier is busy,patient enters line
		patientCashierWaitingCount++;
		cashierLineCV->Wait(cashierLineLock);
	}
	else if(cashierStatus == FREE) {
		cashierStatus = BUSY;
	}
	cashierLineLock->Release();
	cashierInteractionLock->Acquire();
	printf("Parent [%d] reaches the Cashier.\n",P-no_of_patients);
	printf("Parent [%d] hands over his examination sheet to the Cashier.\n",P-no_of_patients);
	cashierLine[position]=P;
	position++;
	cashierInteractionCV->Signal(cashierInteractionLock);
	cashierInteractionCV->Wait(cashierInteractionLock);
	
	printf("\nParent [%d] pays the Cashier $.... \n",P-no_of_patients);
	cashierInteractionCV->Signal(cashierInteractionLock);
	cashierInteractionCV->Wait(cashierInteractionLock);
	
	printf("Parent [%d] receives a receipt from the Cashier.\n",P-no_of_patients);
	
	printf("Parent [%d] asks Child Patient [%d] to leave doctor's office.\n",P-no_of_patients,P-no_of_patients);
	printf("Parent [%d] leaves the doctor's office.\n",P-no_of_patients);
	familyCV[P]->Signal(familyLock[P]);	
	familyCV[P]->Wait(familyLock[P]);

	cashierInteractionCV->Signal(cashierInteractionLock);
	//printf("\nParent %d signalled cashier",P-no_of_patients);
	//printf("\nParent %d trying to acquire leave lock",P-no_of_patients);
	leaveLock->Acquire();
	//printf("\nParent %d leave lock acquired",P-no_of_patients);
	leavingPatients++;
	//printf("\nParent %d increased count",P-no_of_patients);
	leaveLock->Release();
	cashierInteractionLock->Release();
	
	//printf("\nParent %d Signals child",P-no_of_patients);
	familyCV[P]->Signal(familyLock[P]);	
	familyLock[P]->Release();
	//printf("\nParent %d Release family lock",P-no_of_patients);
}


void child(int c)
{
	familyLock[c]->Acquire();

	printf("Child Patient [%d] has entered the Doctor's Office Waiting Room with Parent [%d].\n",c-no_of_patients,c-no_of_patients);
	// If parent has not signalled
	if(parentSignalled[c] == 0)
	{
		familyCV[c]->Wait(familyLock[c]);		
	}
	printf("Child [%d] is following Parent [%d] to Examination Room [%d].\n",c-no_of_patients,patientExamSheet[c]->servingNurse,patientExamSheet[c]->room);

	familyCV[c]->Signal(familyLock[c]);	
	familyCV[c]->Wait(familyLock[c]);
	
	if(currentSymptom[c]==0)
	{
-		printf("Child Patient [%d] says, “My symptoms are Nausea.\"\n",c-no_of_patients);
		familyCV[c]->Signal(familyLock[c]);	
		familyCV[c]->Wait(familyLock[c]);
	}
	else if(currentSymptom[c]==1)
	{
		printf("Child Patient [%d] says, “My symptoms are Pain.\"\n",c-no_of_patients);
		familyCV[c]->Signal(familyLock[c]);	
		familyCV[c]->Wait(familyLock[c]);
	}
	else if(currentSymptom[c]==2)
	{
		printf("Child Patient [%d] says, “My symptoms are I Hear Alien Voices.\"\n",c-no_of_patients);
		familyCV[c]->Signal(familyLock[c]);	
		familyCV[c]->Wait(familyLock[c]);
	}
	if(strcmp(patientExamSheet[c]->treatment,"XRAY") == 0)
	{
		printf("Child Patient [%d] has been informed by Doctor [%d] that he needs an Xray.\n",c-no_of_patients,patientExamSheet[c]->examiningDoctor);
	}
	else if(strcmp(patientExamSheet[c]->treatment,"SHOT") == 0)
	{
		printf("Child Patient [%d] has been informed by Doctor [%d] that he will be administered a shot.\n",c-no_of_patients,patientExamSheet[c]->examiningDoctor);
	}
	else if(strcmp(patientExamSheet[c]->treatment,"FINE") == 0)
	{
		printf("Child Patient [%d] has been diagnosed by Doctor [%d].\n",c-no_of_patients,patientExamSheet[c]->examiningDoctor);
	}

	familyCV[c]->Signal(familyLock[c]);	
	familyCV[c]->Wait(familyLock[c]);

	if(strcmp(patientExamSheet[c]->treatment,"XRAY") == 0)
	{
		printf("Child Patient [%d] is following Parent [%d] to Examination Room [%d].\n",c-no_of_patients,c-no_of_patients,patientExamSheet[c]->room);

		familyCV[c]->Signal(familyLock[c]);	
		familyCV[c]->Wait(familyLock[c]);

	}
	else if(strcmp(patientExamSheet[c]->treatment,"SHOT") == 0)	
	{
		printf("Child Patient [%d] says, \"Yes I am ready for the shot\".\n",c-no_of_patients);

		familyCV[c]->Signal(familyLock[c]);	
		familyCV[c]->Wait(familyLock[c]);

	}
	else if(strcmp(patientExamSheet[c]->treatment,"FINE") == 0)
	{
	
	}
	
	printf("Child Patient [%d] follows Parent [%d] to Cashier\n",c-no_of_patients,c-no_of_patients);

	familyCV[c]->Signal(familyLock[c]);	
	familyCV[c]->Wait(familyLock[c]);

	printf("Child Patient [%d] leaves the doctor's office.\n",c-no_of_patients);
	
	familyCV[c]->Signal(familyLock[c]);	
	familyCV[c]->Wait(familyLock[c]);
	
	//printf("\nChild %d Signal received",c-no_of_patients);
	familyLock[c]->Release();
}

void interaction_1() {
	/* Initialization*/
	wrnLineLock = new Lock("WRN");
	wrnLock = new Lock("WL");	
	infoLock = new Lock("IL");
	waitingRoomLock = new Lock("WRL");
	taskLock = new Lock("TL");
	docLineLock = new Lock("DLL");
	needNurseLock = new Lock("NNL");
	newTempLock = new Lock("NTL");
	xRayWaitingRoomLock = new Lock("XWRL");
	cashierLineLock = new Lock("CLL");
	cashierInteractionLock = new Lock("CIL");
	wallPocketLock = new Lock("WPL");
	xRayLineLock = new Lock("XRLL");	
	leaveLock = new Lock("LL");
		
	patientWaitingCV = new Condition("PWCV");
	wrnWaitingCV = new Condition("WWCV");	
	nurseWaitingCV = new Condition("NWCV");	
	waitingRoomCV = new Condition("WRCV");
	waitingRoomNurseCV = new Condition("WRNCV");
	xRayWaitingRoomCV = new Condition("XWRCV");
	cashierLineCV = new Condition("CLCV");
	cashierInteractionCV = new Condition("CICV");
	xRayWaitingRoomNurseCV = new Condition("XWRNV");
	
	for(int i=0;i<no_of_rooms;i++)
	{
		examRoomStatus[i] = FREE;
		examRoomLock[i] = new Lock("ERL");
		examRoomCV[i] = new Condition("ERCV");
		tempRoomLock[i] = new Lock("TRL");
	}
	
	for(int i=0;i<no_of_doctors;i++)
	{
		docStatus[i]=-1;
		patList[i]=-1;
		docLineCV[i]=new Condition("DLCV");
	}
	
	for(int i=0;i<no_of_patients;i++)
	{
		patientExamSheet[i] = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));
		xRayWaitingRoomWallPocket[i] = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));
		patientExamSheet[i]->pid=-1;
		patientExamSheet[i]->age=-1;
		patientExamSheet[i]->name=NULL;
		patientExamSheet[i]->status=-1;
		patientExamSheet[i]->room=-1;
		patientExamSheet[i]->examiningDoctor=-1;
		patientExamSheet[i]->servingNurse=-1;
		patientExamSheet[i]->xRayRoom=-1;
		currentSymptom[i]=-1;
		patientExamSheet[i]->treatment ="";
		// Type = 0 indicates Adult Patient
		patientExamSheet[i]->type = 0;
	}
	
	for(int i = no_of_patients; i <  (no_of_patients+no_of_parents) ; i++)
	{
		patientExamSheet[i] = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));
		xRayWaitingRoomWallPocket[i] = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));
		patientExamSheet[i]->pid=-1;
		patientExamSheet[i]->age=-1;
		patientExamSheet[i]->name=NULL;
		patientExamSheet[i]->status=-1;
		patientExamSheet[i]->room=-1;
		patientExamSheet[i]->examiningDoctor=-1;
		patientExamSheet[i]->servingNurse=-1;
		patientExamSheet[i]->xRayRoom=-1;
		currentSymptom[i]=-1;
		patientExamSheet[i]->treatment ="";
		// Type = 1 indicates Child Patient
		patientExamSheet[i]->type=1;
	}
	
	for (int i=0;i<no_of_rooms;i++)
	{
		wallPocket[i] = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));
		wallPocket[i]->pid=-1;
		wallPocket[i]->age=-1;
		wallPocket[i]->name=NULL;
		wallPocket[i]->status=-1;
		wallPocket[i]->xRayRoom=-1;
	}
	
	for(int i=0;i<no_of_nurses;i++)
	{
		roomList[i] = -1;
		nurseLineCV[i] = new Condition("NLCV");
		currentDoc[i]=-1;
		tempLock[i] = new Lock("TL");
		tempCV[i] = new Condition("TCV");
		nurseStatus[i]=-1;
	}
	
	for(int i=0;i<no_of_xRayTechnicians;i++)
	{
		patientWaitingForXrayCount[i] = 0;
		//xRayLineLock[i] = new Lock("XRLL");
		xRayInteractionLock[i] = new Lock("XRIL");
		xRayLineCV[i] = new Condition("XRLCV");
		xRayInteractionCV[i] = new Condition("XRICV");
		xRayPatientList[i] = -1;
		xRayStatus[i] = -1;
		xRayInteractionPatCV[i] = new Condition("XRIPCV");
		xRayInteractionNurseCV[i] = new Condition("XRINCV");
	}
	
	for(int i=no_of_patients ; i<(no_of_parents+no_of_patients) ; i++)
	{
	 	familyLock[i] = new Lock("FL");
	 	familyCV[i] = new Condition("FCV");
	}
	/* Fork WRN Thread */
	Thread *wrnThread= new Thread("New WRN");
	wrnThread->Fork((VoidFunctionPtr)waitingRoomNurse,0);
	
	
	/*Patient cannot come into office until WRN starts.*/
	for(int i=0 ; i<100 ; i++) {
		currentThread->Yield();
	}

	/* Fork patient Threads */	
	for(int i=0 ; i<no_of_patients ; i++) {
		Thread *patientThread= new Thread("New Patient");
		patientThread->Fork((VoidFunctionPtr)patient,i);
	}
	
	for(int j=0 ; j<no_of_nurses ; j++) {
		Thread *nurseThread= new Thread("New Nurse");
		nurseThread->Fork((VoidFunctionPtr)nurse,j);
	}
	
	for(int d=0 ; d<no_of_doctors ; d++) {
		Thread *doctorThread= new Thread("New Doctor");
		doctorThread->Fork((VoidFunctionPtr)doctor,d);
	}
	
	for(int x=0 ; x<no_of_xRayTechnicians ; x++) {
		Thread *xrayThread= new Thread("New XRay Technician");
		xrayThread->Fork((VoidFunctionPtr)xrayTechnician,x);
	}

	Thread *cashierThread= new Thread("New Cashier");
	cashierThread->Fork((VoidFunctionPtr)cashier,0);

	for(int P=no_of_patients ; P<(no_of_parents+no_of_patients) ; P++) {
		Thread *parentThread= new Thread("New Parent");
		parentThread->Fork((VoidFunctionPtr)parent,P);
	}
	
	for(int c=no_of_patients ; c<(no_of_parents+no_of_patients) ; c++) {
		Thread *childThread= new Thread("New Child");
		childThread->Fork((VoidFunctionPtr)child,c);
	}
	
}

void Problem2() {
	printf("Simulating WRN Patient interaction\n");

enterPatientsAgain:
	printf("Enter Number of patients entering doctors office:\n");
	scanf("%d",&no_of_patients);
	if(no_of_patients < 1 || no_of_patients > 100)
		goto enterPatientsAgain;

enterNursesAgain;
	printf("Enter Number of Nurses(2-5):\n");
	scanf("%d",&no_of_nurses);
	if(no_of_nurses < 1 || no_of_nurses > 100)
		goto enterNursesAgain;

enterExamRoomsAgain:	
	printf("Enter Number of Exam Rooms(2-5):\n");
	scanf("%d",&no_of_rooms);
	if(no_of_rooms < 2 || no_of_rooms >5)
		goto enterRoomsAgain;
	
enterDoctorsAgain:	
	printf("Enter Number of Doctors(2-3):\n");
	scanf("%d",&no_of_doctors);
	if(no_of_doctors < 2 || no_of_doctors > 3)
		goto enterDoctorsAgain;
		
enterXrayTech:
	printf("Enter Number of XRay Technicians(1-2):\n");
	scanf("%d",&no_of_xRayTechnicians);
	if(no_of_xRayTechnicians < 1 || no_of_xRayTechnicians > 2)
		goto enterXrayTech;

enterParentsAgain:
	printf("Enter Number of Parents/Child Patients:\n");
	scanf("%d",&no_of_parents);
	if(no_of_parents < 1 || no_of_parents > 100)
		goto enterParentsAgain;
		
	no_of_childPatients = no_of_parents;
	interaction_1();
}
