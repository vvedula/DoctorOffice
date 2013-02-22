#include "syscall.h"

/* Initializing Total Entities in Doctors Office */
#define PATIENTS 15

#define FREE 0
#define BUSY 1
#define GETFORM 2
#define GIVEFORM 3

/* Details for Wrn */
int wrnES_pid;
int wrnES_name;
int wrnES_age;
int wrnES_symptom;
int wrnES_status;
int wrnES_room;
int wrnES_images;
int wrnES_treatment;
int wrnES_doctor;
int wrnES_nurse;
int wrnES_xRayRoom;
int wrnES_xRayResult;
int wrnES_type;

/* Details for Patient */
 int patientES_pid;
int patientES_name;
int patientES_age;
int patientES_symptom;
int patientES_status;
int patientES_room;
int patientES_images;
int patientES_treatment;
int patientES_doctor;
int patientES_nurse;
int patientES_xRayRoom;
int patientES_xRayResult;
int patientES_type;

/* Locks */
int printLock;
int infoLock;
int wrnLineLock;
int wrnLock;
int patientIndexLock;

/* CVs */
int patientWaitingCV;
int wrnWaitingCV;
int wrnWaitingPatientCV;
int wrnWaitingAgainPatientCV;

/* MVs*/
int wrnStatusMV;
int patientWaitingCountMV;
int patientTaskMV;
int globalPatientIndexMV;
int patientIndexMV;
int patientGettingFormMV;
int patientGivingFormMV;
int registeredPatientCountMV;
int wrnTaskStatusMV;

void createEntities()
{

	printLock = CreateLock("printLock",sizeof("printLock"));
	wrnLineLock = CreateLock("wrnLLock",sizeof("wrnLLock"));
	wrnLock = CreateLock("WrnLock",sizeof("WrnLock"));
	infoLock = CreateLock("InfoLock",sizeof("InfoLock"));
	patientIndexLock = CreateLock("PatientILock",sizeof("PatientILock"));

	patientWaitingCV = CreateCV("PWCV",sizeof("PWCV"));
	wrnWaitingCV = CreateCV("WWCV",sizeof("WWCV"));	
	wrnWaitingPatientCV = CreateCV("WWPCV",sizeof("WWPCV"));	
	wrnWaitingAgainPatientCV = CreateCV("WWAPCV",sizeof("WWAPCV"));	

	wrnStatusMV = CreateMV("wrnStatusMV",sizeof("wrnStatusMV"),1);
	wrnTaskStatusMV = CreateMV("wrnTaskStatusMV",sizeof("wrnTaskStatusMV"),1);
	globalPatientIndexMV = CreateMV("globalPatientIndexMV",sizeof("globalPatientIndexMV"),1);
	patientIndexMV = CreateMV("patientIndexMV",sizeof("patientIndexMV"),1);
	patientTaskMV = CreateMV("patientTaskMV",sizeof("patientTaskMV"),1);
	patientWaitingCountMV = CreateMV("patientWaitingCountMV",sizeof("patientWaitingCountMV"),1);
	patientGettingFormMV = CreateMV("patientGettingFormMV",sizeof("patientGettingFormMV"),1);
	patientGivingFormMV = CreateMV("patientGivingFormMV",sizeof("patientGivingFormMV"),1);
	registeredPatientCountMV = CreateMV("regPatientCountMV",sizeof("regPatientCountMV"),1);

	/* ExamSheet MV's  for WRN*/
	wrnES_pid = CreateMV("wrnES_pid",sizeof("wrnES_pid"),1);
	wrnES_name = CreateMV("wrnES_name",sizeof("wrnES_name"),1);
	wrnES_age = CreateMV("wrnES_age",sizeof("wrnES_age"),1);
	wrnES_symptom = CreateMV("wrnES_symptom",sizeof("wrnES_symptom"),1);
	wrnES_status = CreateMV("wrnES_status",sizeof("wrnES_status"),1);
	wrnES_room = CreateMV("wrnES_room",sizeof("wrnES_room"),1);
	wrnES_images= CreateMV("wrnES_images",sizeof("wrnES_images"),1);
	wrnES_treatment = CreateMV("wrnES_treatment",sizeof("wrnES_treatment"),1);
	wrnES_doctor = CreateMV("wrnES_doctor",sizeof("wrnES_doctor"),1);
	wrnES_nurse = CreateMV("wrnES_nurse",sizeof("wrnES_nurse"),1);
	wrnES_xRayRoom = CreateMV("wrnES_xRayRoom",sizeof("wrnES_xRayRoom"),1);
	wrnES_xRayResult = CreateMV("wrnES_xRayResult",sizeof("wrnES_xRayResult"),1);
	wrnES_type = CreateMV("wrnES_type",sizeof("wrnES_type"),1);

	/* ExamSheet MV's for Patients */
	patientES_pid = CreateMV("patientES_pid",sizeof("patientES_pid"),PATIENTS);
	patientES_name = CreateMV("patientES_name",sizeof("patientES_name"),PATIENTS);
	patientES_age = CreateMV("patientES_age",sizeof("patientES_age"),PATIENTS);
	patientES_symptom = CreateMV("patientES_symptom",sizeof("patientES_symptom"),PATIENTS);
	patientES_status = CreateMV("patientES_status",sizeof("patientES_status"),PATIENTS);
	patientES_room = CreateMV("patientES_room",sizeof("patientES_room"),PATIENTS);
	patientES_images = CreateMV("patientES_images",sizeof("patientES_images"),PATIENTS);
	patientES_treatment = CreateMV("patientES_treatment",sizeof("patientES_room"),PATIENTS);
	patientES_doctor = CreateMV("patientES_doctor",sizeof("patientES_doctor"),PATIENTS);
	patientES_nurse = CreateMV("patientES_nurse",sizeof("patientES_nurse"),PATIENTS);
	patientES_xRayRoom = CreateMV("patientES_xRayRoom",sizeof("patientES_xRayRoom"),PATIENTS);
	patientES_xRayResult = CreateMV("patientES_xRayResult",sizeof("patientES_xRayResult"),PATIENTS);
	patientES_type = CreateMV("patientES_type",sizeof("patientES_type"),PATIENTS);
}

void setEntities()
{
	SetMV(wrnStatusMV,0,FREE);
	SetMV(wrnTaskStatusMV,0,0);
	SetMV(globalPatientIndexMV,0,0);
	SetMV(patientIndexMV,0,0); 
	SetMV(patientTaskMV,0,-1);
	SetMV(patientWaitingCountMV,0,0);
	SetMV(patientGettingFormMV,0,-1);
	SetMV(patientGivingFormMV,0,-1);
	SetMV(registeredPatientCountMV,0,0);	
	
	SetMV(wrnES_pid,0,-1);
	SetMV(wrnES_name,0,-1);
	SetMV(wrnES_age,0,-1);
	SetMV(wrnES_symptom,0,-1);
	SetMV(wrnES_status,0,-1); 
	SetMV(wrnES_room,0,-1);
	SetMV(wrnES_images,0,-1);
	SetMV(wrnES_treatment,0,-1);
	SetMV(wrnES_doctor,0,-1);
	SetMV(wrnES_nurse,0,-1);
	SetMV(wrnES_xRayRoom,0,-1);
	SetMV(wrnES_xRayResult,0,-1);
	SetMV(wrnES_type,0,-1);
}
