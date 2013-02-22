
#include "system.h"
#include "synch.h"
#include "thread.h"
#include "stdio.h"
#include "iostream.h"
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


Lock *wrnLineLock;
Lock *wrnLock;
Lock *infoLock;

Lock *waitingRoomLock;
Lock *taskLock;
Lock * tempTaskLock;
Lock *examRoomLock[50];

Lock *docLineLock;

Condition *patientWaitingCV;
Condition *wrnWaitingCV;
Condition *nurseWaitingCV;
Condition *waitingRoomCV;
Condition *examRoomCV[50];
Condition *waitingRoomNurseCV;
Condition *docLineCV;
Condition *nurseLineCV;

char randomChar();
void waitingRoomNurse();
void patient(int);
void nurse(int);
void doctor(int);

char randomChar() {
	char randomCharacter;
	int randomNumber = random()%26;
	randomNumber += 97;
	randomCharacter = (char)randomNumber;
	return randomCharacter;
}

int no_of_patients=-1;
int no_of_nurses=-1;
int no_of_rooms=-1;
int no_of_doctors=-1;
int wrnStatus=-1;
int wrnTaskStatus=SERVE_NEITHER;
int patientTask=-1;
int nurseTask=-1;
int roomList[50];
int localDocList[10];
int currentNurse=-1;
int currentNurse_dLL=-1;
int currentPatient=-1;
int currentPatient_erl=-1;
int currentPatient_dLL = -1;
int currentRoom=-1;
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
int doctorid=-1;
int docList[100];
int counter=0;
int nursePatCount=0;
int patList[100];
int localPatList[100];
int no_of_xrayTechnicians;

struct ExamSheet *wrnExamSheet = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));
struct ExamSheet *patientExamSheet[100];
struct ExamSheet *examSheetNurseCopy[5];

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
			 wrnTaskStatus = SERVE_NEITHER;
		}
		if(patientWaitingCount > 0)
		{
			patientWaitingCV->Signal(wrnLineLock);
			patientWaitingCount--;
			wrnTaskStatus=SERVE_PATIENT;
			if(patientWaitingCount == 0)
			{
				goto  xyz;
			}
		}
		if(nurseWaitingCount > 0 && wrnTaskStatus != SERVE_PATIENT)
		{
			nurseWaitingCV->Signal(wrnLineLock);
			nurseWaitingCount--;
			wrnTaskStatus=SERVE_NURSE;				
		}
		else if(nurseWaitingCount > 0 && patientWaitingCount == 0)
		{
			nurseWaitingCV->Signal(wrnLineLock);
			nurseWaitingCount--;
			wrnTaskStatus=SERVE_NURSE;				
		}
xyz:		wrnLock->Acquire();
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
					printf("\nWRN:Waiting Room nurse gives a form to Adult patient [%d]",patientGettingForm);
					/* Take the Form & Leave the Line. So that i can become free again */
					wrnWaitingCV->Signal(wrnLock);
					wrnLock->Release();
			}
			if(patientTask==GIVEFORM)
			{
				printf("\nWaiting Room nurse accepts the form from Adult Patient/Parent [%d] with name [%s] and age [%d].",patientGivingForm,patientExamSheet[patientGivingForm]->name,patientExamSheet[patientGivingForm]->age);
				printf("\nWaiting Room nurse creates an examination sheet for [Adult/Child] patient[%d] with name [%s] and age [%d].",patientGivingForm,patientExamSheet[patientGivingForm]->name,patientExamSheet[patientGivingForm]->age);
				printf("\nWaiting Room nurse tells the Adult Patient/Parent [%d] to wait in the waiting room for a nurse",patientGivingForm);
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
						printf("\nWaiting Room nurse gives examination sheet of patient[%d] to Nurse[%d]",patient,currentNurse);
						wrnWaitingCV->Signal(wrnLock);
						break;
					}
				}
				wrnLock->Release();
			}
		}		
	}
	printf("\n\nLeaving WRN");
}

void patient(int i) {
	printf("\nAdult Patient:%d has entered the Doctor's Office Waiting Room.",i);
	//patientExamSheet[i] = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));
		
	wrnLineLock->Acquire();
	if(wrnStatus == BUSY) {
		//If WRN is busy,patient enters line
//		printf("\nPatient%d:WRN is busy. I have to wait",i);
		printf("\nAdult Patient:%d gets in line of the Waiting Room Nurse to get registration form.",i);
		patientWaitingCount++;
		patientWaitingCV->Wait(wrnLineLock);
	}
	else if(wrnStatus == FREE) {
		wrnStatus = BUSY;
	}

	/* Patient was waiting in line for wrn. When wrn signals him, he has to come out of line.*/
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
		printf("\nAdult Patient:%d gets in line of the Waiting Room Nurse to submit registration form.",i);
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
	printf("\nAdult patient [%d] submits the filled form to the Waiting Room Nurse.",i);
       	wrnWaitingCV->Signal(wrnLock);
	wrnWaitingCV->Wait(wrnLock);
		
	/*Now patient has to wait in waiting Room. He acquires Waiting for Nurse Lock & Waits on it */

	waitingRoomLock->Acquire();
	wrnLock->Release();	
	waitingRoomCV->Wait(waitingRoomLock);
	
	/* Patient Nurse Interaction Begins */

	currentPatient = i;

	waitingRoomNurseCV->Signal(waitingRoomLock);
	waitingRoomNurseCV->Wait(waitingRoomLock);
	
	printf("\nAdult Patient/Parent [%d] is following Nurse [%d] to Examination Room [%d].",i,currentNurse_wrl,currentRoom);	

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
		printf("\nAdult Patient/Parent [%d] says, “My symptoms are Nausea.\"",i);
	}
	else if(currentSymptom[i]==1)
	{
		printf("\nAdult Patient/Parent [%d] says, “My symptoms are Pain.\"",i);
	}
	else if(currentSymptom[i]==2)
	{
		printf("\nAdult Patient/Parent [%d] says, “My symptoms are I Hear Alien Voices.\"",i);
	}
	examRoomCV[patientExamSheet[i]->room]->Signal(examRoomLock[patientExamSheet[i]->room]);
	examRoomCV[patientExamSheet[i]->room]->Wait(examRoomLock[patientExamSheet[i]->room]);
	
	
	//examRoomCV[patientExamSheet[i]->room]->Signal(examRoomLock[patientExamSheet[i]->room]);
	if(strcmp(patientExamSheet[i]->treatment,"XRAY") == 0)
	{
		printf("\nAdult Patient [%d] has been informed by Doctor [%d] that he needs an Xray.",i,patientExamSheet[i]->examiningDoctor);
	}
	else if(strcmp(patientExamSheet[i]->treatment,"SHOT") == 0)
	{
		printf("\nAdult Patient [%d] has been informed by Doctor [%d] that he will be administered a shot.",i,patientExamSheet[i]->examiningDoctor);
	}
	else if(strcmp(patientExamSheet[i]->treatment,"FINE") == 0)
	{
		printf("\nAdult Patient [%d] has been diagnosed by Doctor [%d].",i,patientExamSheet[i]->examiningDoctor);
	}
	
	examRoomCV[patientExamSheet[i]->room]->Signal(examRoomLock[patientExamSheet[i]->room]);
	examRoomLock[patientExamSheet[i]->room]->Release();
}

int count=0;

void nurse(int j)
{
	struct ExamSheet *myExamSheet=(struct ExamSheet *)malloc(sizeof(struct ExamSheet));
	while(1)
	{	
		for(int y=0;y<15;y++)
		{
			currentThread->Yield();
		}
		//printf("\n\n\nNurse:%d starts\n\n\n",j);
		taskLock->Acquire();
		for(int k=0;k<no_of_rooms;k++)
		{
			if(examRoomStatus[k] == FREE)
			{
				examRoomStatus[k]=BUSY;
				roomList[j]=k;	//j-th nurse makes k-th room busy
				break;
			}
		}
		taskLock->Release();
			
		if(roomList[j] != -1)
		{	
		//	printf("\n\n\nNurse:%d Inside if\n\n\n",j);
			/* Talk to WRN after finding free room */
			wrnLineLock->Acquire();
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
			/* Interaction with wrn begins */
			wrnLock->Acquire();
			if(registeredPatientCount > 0 )
			{
				currentNurse = j;
				wrnTaskStatus=SERVE_NURSE;
				nurseTask = FETCH_PATIENT;
				if(count>=no_of_patients)
				{
					break;
				}
				count++;
				wrnWaitingCV->Signal(wrnLock);
				printf("\nNurse [%d] tells Waiting Room Nurse to give a new examination sheet.",currentNurse);
				wrnWaitingCV->Wait(wrnLock);
			}		
			else
			{
				examRoomStatus[roomList[j]]=FREE;
				roomList[j]=-1;
			}
			/* Nurse now asks patient in waiting Room to follow her to Exam Room */
			
			if(registeredPatientCount > 0 )
			{
			/* Nurse Gets Exam Sheet from WRN */
				waitingRoomLock->Acquire();
				currentRoom = roomList[j];
				currentNurse_wrl = j;
			
				waitingRoomCV->Signal(waitingRoomLock);
				waitingRoomNurseCV->Wait(waitingRoomLock);
			
				printf("\nNurse [%d] escorts Adult Patient/Parent [%d] to the examination room [%d].",j,currentPatient,roomList[j]);
				patientExamSheet[currentPatient]->room = roomList[j];
				myExamSheet = patientExamSheet[currentPatient];
				
				waitingRoomNurseCV->Signal(waitingRoomLock);
				waitingRoomNurseCV->Wait(waitingRoomLock);
	
				examRoomLock[roomList[j]]->Acquire();
				waitingRoomNurseCV->Signal(waitingRoomLock);
				waitingRoomLock->Release();
				wrnLock->Release();				
				
				//printf("\n\nNext:CurrentRoomERL:%d",currentRoom_erl);			
				examRoomCV[roomList[j]]->Wait(examRoomLock[roomList[j]]);
			
				printf("\nNurse:%d takes the temperature and blood pressure of Patient:%d",j,myExamSheet->pid);
				printf("\nNurse:%d asks Patient:%d \"What Symptoms do you have?\"",j,myExamSheet->pid);
				
				examRoomCV[roomList[j]]->Signal(examRoomLock[roomList[j]]);
				examRoomCV[roomList[j]]->Wait(examRoomLock[roomList[j]]);
				
				if(currentSymptom[myExamSheet->pid] == 0)
				{
					myExamSheet->symptom="Nausea";
				}
				else if(currentSymptom[myExamSheet->pid] == 1)
				{
					myExamSheet->symptom="Pain";
				}
				else
				{
					myExamSheet->symptom="I hear alien voices";
				}

				myExamSheet->status = WAITING_FOR_DOC;
				patientExamSheet[myExamSheet->pid]=myExamSheet;
				//examRoomCV[roomList[j]]->Signal(examRoomLock[roomList[j]]);
				//examRoomCV[roomList[j]]->Wait(examRoomLock[roomList[j]]);
				
				examRoomLock[roomList[j]]->Release(); 
			}
			
		}
		if(myExamSheet->status == WAITING_FOR_DOC)
		{
			docLineLock->Acquire();
			if(docCount>0)
			{
				
				docLineCV->Signal(docLineLock);
				nurseLineCV->Wait(docLineLock);
			
				for(int x=0;x<no_of_doctors;x++)
				{
					if(docList[x]!=-1)
					{
						localDocList[j]=docList[x];
						docList[x]=-1;
						break;
					}
				}
				printf("\nNurse [%d] informs Doctor [%d] that Adult/Child Patient [%d] is waiting in the examination room [%d].",j,localDocList[j],myExamSheet->pid,roomList[j]);

				patList[nursePatCount++] = myExamSheet->pid;
				
				nurseLineCV->Signal(docLineLock);
				nurseLineCV->Wait(docLineLock);
		
				printf("\nNurse [%d] hands over to the Doctor [%d] the examination sheet of Adult/Child Patient [%d].",j,localDocList[j],myExamSheet->pid);
				examRoomStatus[roomList[j]]=FREE;
				roomList[j]=-1;
				myExamSheet->status=FREE;
				nurseLineCV->Signal(docLineLock);
				nurseLineCV->Wait(docLineLock);
				if(nursePatCount==no_of_patients)
				nursePatCount=0;
			}
			docLineLock->Release();
		}
	}
}

void doctor(int d)
{
	struct ExamSheet *examSheetDocCopy = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));
	while(1)
	{
		docLineLock->Acquire();
		docCount++;
		docLineCV->Wait(docLineLock);
		
		/* Doctor interacts with Nurse */
		docList[counter++] = d;	
		
		nurseLineCV->Signal(docLineLock);
		nurseLineCV->Wait(docLineLock);

		tempTaskLock->Acquire();
		for(int y=0;y<no_of_patients;y++)
		{
			if(patList[y]!=-1)
			{
				localPatList[d]=patList[y];
				patList[y]=-1;
				break;
			}
		}
		
		for(int y=0;y<no_of_patients;y++)
		{
			if(patientExamSheet[y]->pid == localPatList[d])
			{
				examSheetDocCopy = patientExamSheet[y];
				break;
			}
		}
		tempTaskLock->Release();
		
		nurseLineCV->Signal(docLineLock);
		nurseLineCV->Wait(docLineLock);
		//printf("\nDoctor [%d] is reading the examination sheet of [Adult/Child] Patient [%d] in Examination room [%d].",d,examSheetDocCopy->pid,examSheetDocCopy->room);
		if(counter==no_of_doctors)
			counter=0;
		nurseLineCV->Signal(docLineLock);
		docLineLock->Release();
		
		
		/* Interaction with Patient Starts */		
		examRoomLock[examSheetDocCopy->room]->Acquire();

		printf("\nDoctor [%d] is reading the examination sheet of [Adult/Child] Patient [%d] in Examination room [%d].",d,examSheetDocCopy->pid,examSheetDocCopy->room);		
		
		/* 
			Doctor randomly chooses one of the following:
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
			printf("\nDoctor [%d] notes down in the sheet that Xray is needed for Adult Patient [%d] in Examination room [%d].",d,examSheetDocCopy->pid,examSheetDocCopy->room);
			examSheetDocCopy->treatment="XRAY";
			
			srand(time(0));
			int xrayRandomNumber = random()%100;
			if(xrayRandomNumber >= 0 && xrayRandomNumber <= 33)
			{
				examSheetDocCopy->no_of_images = 1;
			}
			if(xrayRandomNumber >= 34 && xrayRandomNumber <= 67)
			{
				examSheetDocCopy->no_of_images = 2;
			}
			if(xrayRandomNumber >= 68 && xrayRandomNumber <= 100)
			{
				examSheetDocCopy->no_of_images = 3;
			}
		}
		
		/* There is a separate 25% chance that a Patient will need a shot.*/
		if(randomNumber >= 75 && randomNumber <= 100)
		{
			printf("\nDoctor [%d] notes down in the sheet that Adult Patient [%d] needs to be given a shot in Examination room [%d].",d,examSheetDocCopy->pid,examSheetDocCopy->room);
			examSheetDocCopy->treatment="SHOT";
		}
		if(randomNumber >25 && randomNumber <75)
		{
			printf("\nDoctor [%d] diagnoses Adult Patient [%d] to be fine and is leaving Examination Room [%d].",d,examSheetDocCopy->pid,examSheetDocCopy->room);
			examSheetDocCopy->treatment="FINE";
		}
		printf("\nDoctor [%d] has left Examination Room [%d].",d,examSheetDocCopy->room);
		printf("\nDoctor [%d] is going to their office.",d);
		
		patientExamSheet[examSheetDocCopy->pid]=examSheetDocCopy;
		patientExamSheet[examSheetDocCopy->pid]->examiningDoctor=d;
		examRoomCV[examSheetDocCopy->room]->Signal(examRoomLock[examSheetDocCopy->room]);
		examRoomCV[examSheetDocCopy->room]->Wait(examRoomLock[examSheetDocCopy->room]);
		
		examRoomLock[examSheetDocCopy->room]->Release();
		
	}	
}

void xrayTechnician(int xr)
{
	while(1)
	{
	
	}
}

void interaction_1() {
	/* Initialization*/
	wrnLineLock = new Lock("WRN");
	wrnLock = new Lock("WL");	
	infoLock = new Lock("IL");
	waitingRoomLock = new Lock("WRL");
	taskLock = new Lock("TL");
	docLineLock = new Lock("DLL");
	tempTaskLock = new Lock("TLL");
	
	patientWaitingCV = new Condition("PWCV");
	wrnWaitingCV = new Condition("WWCV");	
	nurseWaitingCV = new Condition("NWCV");	
	waitingRoomCV = new Condition("WRCV");
	waitingRoomNurseCV = new Condition("WRNCV");
	docLineCV = new Condition("DLCV");
	nurseLineCV = new Condition("NLCV");
	
	for(int i=0;i<no_of_rooms;i++)
	{
		examRoomStatus[i] = FREE;
		examRoomLock[i] = new Lock("ERL");
		examRoomCV[i] = new Condition("ERCV");
	}
	
	for(int i=0;i<3;i++)
	{
		docList[i]=-1;
	}
	
	for(int i=0;i<no_of_patients;i++)
	{
		patientExamSheet[i] = (struct ExamSheet *)malloc(sizeof(struct ExamSheet));
		patientExamSheet[i]->pid=-1;
		patientExamSheet[i]->age=-1;
		patientExamSheet[i]->name=NULL;
		patientExamSheet[i]->status=-1;
		patientExamSheet[i]->room=-1;
		currentSymptom[i]=-1;
		patList[i]=-1;
		localPatList[i]=-1;
	}
	
	
	for(int i=0;i<no_of_nurses;i++)
	{
		roomList[i] = -1;
		localDocList[i]=-1;
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
	
/*	for(int xr=0 ; xr<no_of_xrayTechnicians ; xr++) {
		Thread *xrayThread= new Thread("New XRay Technician");
		xrayThread->Fork((VoidFunctionPtr)xrayTechnician,i);
	}*/
}

void Problem2() {
	printf("\nSimulating WRN Patient interaction");
	printf("\nEnter Number of patients entering doctors office:");
	scanf("%d",&no_of_patients);
	printf("\nEnter Number of Nurses:");
	scanf("%d",&no_of_nurses);
	printf("\nEnter Number of Exam Rooms:");
	scanf("%d",&no_of_rooms);
	printf("\nEnter Number of Doctors:");
	scanf("%d",&no_of_doctors);
	printf("\nEnter Number of XRay Technicians:");
	scanf("%d",&no_of_xrayTechnicians);
	interaction_1();
}
