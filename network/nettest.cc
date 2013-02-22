// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include <sys/time.h>
#include<iostream>
#include <stdlib.h>

using namespace std;


// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

#define AVAILABLE 0
#define BUSY 1
#define MAXLOCKS 100
#define MAXCVS 100
#define MAXMVS 100

/*********************Code for Project 3 ***********************/
	
struct ServerLock
{
	int state;			// Available or busy
	int lockid;
	char lockName[25];
	int machineId;		// Equivalent to IPAddress
	int mailboxNum;		//Equivalent to Port Number
	bool exists;
	List *machine;		//Queue of Machine id's
	List *mailbox;		// Queue of Mailbox Numbers
	int lockUsedBy;		// To keep a track of the number of times lock is created so that lock won't be destroyed in between
}serverLock[100];

struct ServerCondition
{
	char cvName[25];
	int cvid;
	int lockUsed; 
	int machineId;		// Equivalent to IPAddress
	int mailboxNum;		//Equivalent to Port Number
	List *machine;
	List *mailbox;
	bool exists;
	int cvUsedBy;		// To keep a track of the number of times CV is created so that CV won't be destroyed in between
}serverCondition[100];

struct MonitorVariable
{
	int mvid;
	char mvName[25];	//Name of the Monitor Variable
	int  values[100];		//An array of values of size = max (recd from client)
	int max;				// Size of int array
	bool exists;
	int mvUsedBy;		// To keep a track of the number of times MV is created so that MV won't be destroyed in between
}mv[500];

struct RequestMsg
{
	char requestMsgString[100];
	int64_t timeStamp;
	int replyingServerMachine;
	int clientMac;
	int clientMail;
}*requestMsg,*currentMsg;

int lockIndex = 0;
int cvIndex = 0;
int mvIndex = 0;
int64_t LTR[5];
int clientMachineId= -1;
int clientMailBoxNum = -1;

List *readyMachineIdList;
List *readyMailboxNumList;
List *requestMsgQueue;
  
PacketHeader outPktHdr, inPktHdr;
MailHeader outMailHdr, inMailHdr;

void processMsg(char [],int,int,int);

void MailTest(int farAddr)
{
 //   char *data = "Hello there!";
 //   char *ack = "Got it!";
    char buffer[MaxMailSize];
    char newBuffer[MaxMailSize];
    readyMachineIdList = new List;
    readyMailboxNumList = new List; 
    requestMsgQueue = new List;
    requestMsg = (struct RequestMsg *)malloc(sizeof(struct RequestMsg));
  
    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = farAddr;		
    outMailHdr.to = 0;
    outMailHdr.from = 1;
   // outMailHdr.length = strlen(data) + 1;

	//Initializing lockUsedBy, CVUsedBy & MVUsedBy to 0
	for(int k=0;k<MAXLOCKS;k++)
	{
		serverLock[k].lockUsedBy=0;
	}
	for(int k=0;k<MAXCVS;k++)
	{
		serverCondition[k].cvUsedBy=0;
	}
	for(int k=0;k<MAXMVS;k++)
	{
		mv[k].mvUsedBy=0;
	}
	for(int k=0;k<numServers;k++)
	{
		LTR[k] = 0;
	}
	while(1)
	{
	//	printf("My Id is=>%d\n",serverMachineId);
		postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
		printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
		fflush(stdout);
		
		char *msgStr = strtok(buffer,":");
		char *sender = new char[10];
		int i=0;
		
		/* Check who the msg is from */
		while(msgStr != NULL)
		{
			if(i == 0)
			{
				strcpy(sender,msgStr);
			}
			if(i == 1)
			{
				strcpy(newBuffer,msgStr);
			}
			msgStr = strtok(NULL,":");
			i++;
		}
		i=0;
		
		/* If a timeStamp Msg is recd, update the LTR */
		if(strcmp(sender,"TimeStamp") == 0)
		{
			char *strNew = new char[50];
			char *str = strtok(newBuffer,":");
			int forwardedServerMacId,forwardedServerMailBox;
			int64_t forwardedTimeStamp;
			strNew = strtok(str,";");
			
			while(strNew != NULL)
			{
				if(i == 0)
				{
					/* TimeStamp */
					sscanf(strNew,"%llu",&forwardedTimeStamp);
				}
				if(i == 1)
				{
					/* Forwarding Server id */
					forwardedServerMacId = atoi(strNew);
				}
				if(i == 2)
				{
					/* Forwarding Server mailbox */
					forwardedServerMailBox = atoi(strNew);
				}
				strNew = strtok(NULL,";");
				i++;
			}
			i=0;
			LTR[forwardedServerMacId] = forwardedTimeStamp;		
			/******************************************NEW CODE ***************************************/
			//printf("After Receiving TS Msg\n");
			int64_t smallestTimeStamp = LTR[0];
			for(i=0;i<numServers;i++)
			{
				if(LTR[i] < smallestTimeStamp && LTR[i] != 0)
				{
				 	smallestTimeStamp = LTR[i];
				}
			}
			//printf("SmallestTimeStamp=>%llu\n",smallestTimeStamp);
			if(!requestMsgQueue->IsEmpty())
			{
				currentMsg = (struct RequestMsg*)requestMsgQueue->SortedRemove(&smallestTimeStamp);			
			//	printf("CurrentMsg=>%s\n",currentMsg->requestMsgString);
			//	printf("CurrentTimeStamp=>%llu\n",currentMsg->timeStamp);
			//	printf("CurrentClient=>%d %d \n",currentMsg->clientMac,currentMsg->clientMail);
		
				while(currentMsg->timeStamp <= smallestTimeStamp)
				{
					//Process currentMsg
					processMsg(currentMsg->requestMsgString,currentMsg->replyingServerMachine,currentMsg->clientMac,currentMsg->clientMail);
					//LTR[forwardingServerMachineId] = 0;
					//Check next msg
					if(!requestMsgQueue->IsEmpty())	
						currentMsg = (struct RequestMsg*)requestMsgQueue->SortedRemove(&smallestTimeStamp);
					else
						break;
				}
				//put msg back in the queue
				if(currentMsg->timeStamp > smallestTimeStamp)
					requestMsgQueue->SortedInsert((void*)currentMsg,forwardedTimeStamp);
			}
		}
		/* If the message if from client, we need to forward it to all other servers	*/
		 if(strcmp(sender,"Client") == 0)
		{
			clientMachineId = inPktHdr.from;
			clientMailBoxNum = inMailHdr.from;

			int64_t a, b, c, d;
			timeval t;
			gettimeofday(&t, NULL);
			a = t.tv_sec;
			b = a * 1000000;
			c = b + t.tv_usec;
			d = c * 10;
			d += 5;
		
			for(i=0;i<numServers;i++)
			{
				//Attach timestamp
				//Attach ForwardingServer machine Id & mailbox Number to it
				//Save client details
			//	if(i	!= serverMachineId)
				{
					char *forwardingMsg = new char[100];
					strcpy(forwardingMsg,"Server:");
					strcat(forwardingMsg,newBuffer);
					strcat(forwardingMsg,";");
					char timeString[25],serverMachineIdString[2],clientMachineIdString[2],clientMailBoxNumString[2];
	
					//itoa(d,10,timeString);
					//memcpy(timeString,&d,sizeof(d));
					sprintf(timeString,"%llu",d);
					strcat(forwardingMsg,timeString);
					strcat(forwardingMsg,";");
					sprintf(serverMachineIdString,"%d",serverMachineId);
					strcat(forwardingMsg,serverMachineIdString);
					strcat(forwardingMsg,";");
					strcat(forwardingMsg,"1");
					strcat(forwardingMsg,";");
					sprintf(clientMachineIdString,"%d",clientMachineId);
					strcat(forwardingMsg,clientMachineIdString);
					strcat(forwardingMsg,";");
					sprintf(clientMailBoxNumString,"%d",clientMailBoxNum);
					strcat(forwardingMsg,clientMailBoxNumString);
				
					//ForwardingMsg Type => Server:RequestType*Arguments;TimeStamp;ServerMachineId;ServerMailBoxNum;ClientMachine;ClientMailBox
					outMailHdr.length = strlen(forwardingMsg) + 1;
					outPktHdr.to = i;
					outMailHdr.to = 1;
					postOffice->Send(outPktHdr, outMailHdr, forwardingMsg);
				}
			}
		}
		
		/*If the message is from server, we need to do total ordering so that all servers process
		 client request in the same order*/
		int k1=0;
		if(strcmp(sender,"Server") == 0)
		{
			/********************** Step 1: Server receives Msg **************************/
			//Server forwards its timestamp to all other servers
			int64_t a, b, c, d;
			timeval t;
			gettimeofday(&t, NULL);
			a = t.tv_sec;
			b = a * 1000000;
			c = b + t.tv_usec;
			d = c * 10;
			d += 5;
			
			char *timeStampMsg = new char[50];
			char timeString[25],serverMachineIdString[2];
			
			strcpy(timeStampMsg,"TimeStamp:");
			sprintf(timeString,"%llu",d);
			strcat(timeStampMsg,timeString);
			strcat(timeStampMsg,";");
			sprintf(serverMachineIdString,"%d",serverMachineId);
			strcat(timeStampMsg,serverMachineIdString);
			strcat(timeStampMsg,";");
			strcat(timeStampMsg,"1");
			outMailHdr.length = strlen(timeStampMsg) + 1;
			for(int k=0;k<numServers;k++)
			{
				outPktHdr.to = k;
				outMailHdr.to = 1;
				if(k	!= serverMachineId)
				{
					postOffice->Send(outPktHdr, outMailHdr, timeStampMsg);
					//printf("Sending TimeStamp %llu to server %d\n",d,k);
				}
			}
			//Update its own table as well
			LTR[serverMachineId] = d;
	
			/************** Step 2: Extract TimeStamp & Forwarding Server Id ***************/
			char *strNew = new char[50];
			char *str = strtok(newBuffer,":");
			//char *requestMsg = new char[30];
			int forwardingServerMachineId;
			int forwardingServerMailBoxNum;
			int64_t recvTimeStamp;
			
			strNew = strtok(str,";");
			i=0;
			while(strNew != NULL)
			{
				if(i == 0)
				{
					//strcpy(requestMsg,strNew);
					strcpy(requestMsg->requestMsgString,strNew);
				}
				if(i == 1)
				{
					// Convert the timestamp back to int64_t & store it	
					sscanf(strNew,"%llu",&recvTimeStamp);
					requestMsg->timeStamp = recvTimeStamp;
					//printf("recvTimeStamp=>%llu\n",recvTimeStamp);
				}
				if(i ==2 )
				{
					// Forwarding Server Machine id
					forwardingServerMachineId = atoi(strNew);
					requestMsg->replyingServerMachine = forwardingServerMachineId;
					//printf("forwardingServerMachineId=>%d\n",forwardingServerMachineId);
				}
				if(i == 3)
				{
					// Forwarding Server MailBoxNum
					forwardingServerMailBoxNum = atoi(strNew);
					//printf("forwardingServerMailBoxNum=>%d\n",forwardingServerMailBoxNum);
				}
				if(i == 4)
				{
					clientMachineId = atoi(strNew);
					requestMsg->clientMac = clientMachineId;
				}
				if(i == 5)
				{
					clientMailBoxNum = atoi(strNew);
					requestMsg->clientMail = clientMailBoxNum;
				}
				strNew = strtok(NULL,";");
				i++;
			}

			/*********************** Step 3 : Put the request in pending Msg Queue **************/
			requestMsgQueue->SortedInsert((void*)requestMsg,recvTimeStamp);
			//printf("Added msg to msg Q\n");

			/*************** Step 4 :  Update LTR with timestamp of new msg for particular server ****/                          
			LTR[forwardingServerMachineId] = recvTimeStamp;
			//printf("LTR[0]=>%llu\n",LTR[0]);
			//printf("LTR[1]=>%llu\n",LTR[1]);
			//printf("LTR[1]=>%llu\n",LTR[2]);
			/*************** Step 5 : Scan LTR & Extract the smallest TimeStamp ******************/
	/*		i=0;
			while(LTR[i] == 0 && i<numServers)
				i++;*/
			int64_t smallestTimeStamp = LTR[0];
			for(i=0;i<numServers;i++)
			{
				if(LTR[i] < smallestTimeStamp && LTR[i] != 0)
				{
				 	smallestTimeStamp = LTR[i];
//				 	smallestTimeStampMachineId = i;
				}
			}
			//printf("SmallestTimeStamp=>%llu\n",smallestTimeStamp);
			/***************** Step 5b : Retrieve 1st Msg from Pending Msg Queue ******************/
			currentMsg = (struct RequestMsg*)requestMsgQueue->SortedRemove(&smallestTimeStamp);			
			//printf("CurrentMsg=>%s\n",currentMsg->requestMsgString);
			//printf("CurrentTimeStamp=>%llu\n",currentMsg->timeStamp);
			//printf("CurrentClient=>%d %d\n",currentMsg->clientMac,currentMsg->clientMail);
			
			/********** Step 6 : Process Msg having TimeStamp <= TimeStamp from Step 5 *************/			
			while(currentMsg->timeStamp <= smallestTimeStamp)
			{
				//Process currentMsg
				processMsg(currentMsg->requestMsgString,forwardingServerMachineId,currentMsg->clientMac,currentMsg->clientMail);
				//LTR[forwardingServerMachineId] = 0;
				//Check next msg
				if(!requestMsgQueue->IsEmpty())	/* Q not empty*/
					currentMsg = (struct RequestMsg*)requestMsgQueue->SortedRemove(&smallestTimeStamp);
				else
					break;
			}
			//put msg back in the queue
			if(currentMsg->timeStamp > smallestTimeStamp)
				requestMsgQueue->SortedInsert((void*)currentMsg,recvTimeStamp);
		}
	/*	outPktHdr.to = inPktHdr.from;
		outMailHdr.to = inMailHdr.from;
		outMailHdr.length = strlen(ack) + 1;
		bool success = postOffice->Send(outPktHdr, outMailHdr, ack); 

		if ( !success ) {
			printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
			interrupt->Halt();
    		}*/
	}
    // Send the first message
 /*  bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }*/

    // Wait for the first message from the other machine
    /*postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = strlen(ack) + 1;
    success = postOffice->Send(outPktHdr, outMailHdr, ack); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the ack from the other machine to the first message we sent.
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);
*/

    // Then we're done!
 //   interrupt->Halt();
}

void processMsg(char newBuffer[50],int forwardingServerMachineId,int clientMachine,int clientMailBox)
{
		//printf("Message is from %s\n",sender);
		char *str = strtok(newBuffer,"*");
		char *requestParameters =  new char[25];
		char *requestType = new char[20];
		int i=0;		
		
		while(str != NULL)
		{
			if(i == 0)
			{
				fflush(stdout);
				strcpy(requestType,str);
			}
			else if(i == 1)
			{
				strcpy(requestParameters,str);
			}
			str = strtok(NULL,"*");
			i++;
		}
		/*********************** Create Lock ****************************/	
		if(strcmp(requestType,"CreateLock") == 0)
		{			
			char * lockName = requestParameters;
			char *ack = new char[3];
		
			if(lockIndex > MAXLOCKS)
			{
				printf("Exceeded lock limit:Cannot create more locks");
				fflush(stdout);
			}
			else if((sizeof(lockName) < 0 || sizeof(lockName) > 100) && atoi(lockName) < 0)
			{
				printf("\nCannot create Lock:Invalid Lock Name \n");
				fflush(stdout);
			}
			else
			{
				int j=0;
				for(j=0;j<MAXLOCKS;j++)
				{
					if(strcmp(serverLock[j].lockName,lockName) == 0 && serverLock[j].exists)
					{
						sprintf(ack,"%d",j);
						fflush(stdout);
						break;
					}
				}
				if(j == MAXLOCKS)	/* Lock does not exist. Create it */
				{
					serverLock[lockIndex].state = AVAILABLE;
					serverLock[lockIndex].lockid = lockIndex;
					strcpy(serverLock[lockIndex].lockName,lockName);
					serverLock[lockIndex].machineId = clientMachine;
					serverLock[lockIndex].mailboxNum = clientMailBox;
					serverLock[lockIndex].lockUsedBy++;
					serverLock[lockIndex].exists = true;
					serverLock[lockIndex].machine = new List;
					serverLock[lockIndex].mailbox = new List;
					sprintf(ack,"%d",lockIndex);
					lockIndex++ ;
					printf("Created Lock=>%s with id=>%d\n",lockName,lockIndex-1);
					//printf("Count for Lock:%s=>%d\n",lockName,serverLock[lockIndex-1].lockUsedBy);
					fflush(stdout);
				}
				else
				{
					serverLock[j].lockUsedBy++;
					printf("Lock=>%s exists. Returning its id=>%d\n",lockName,j);
					//printf("Count for Lock:%s=>%d\n",lockName,serverLock[j].lockUsedBy);
					fflush(stdout);
				}
				outMailHdr.length = strlen(ack) + 1;
				outPktHdr.to = clientMachine;
				outMailHdr.to = clientMailBox;
				//printf("machineid=>%d,mailboxNum=>%d\n",outPktHdr.to,outMailHdr.to);
				if(serverMachineId == forwardingServerMachineId)
					postOffice->Send(outPktHdr, outMailHdr, ack);
				fflush(stdout);
			}
		}
		/******************** Destroy Lock ****************************/
		else if(strcmp(requestType,"DestroyLock") == 0)
		{
			char * lockName = requestParameters;
			int lockid = atoi(lockName);
			char *ack = new char[25];
		
			if(lockid < 0 || lockid > 100)
			{
				printf("\nCannot delete Lock:Invalid Lock Name \n");
				fflush(stdout);
			}
			else
			{
				int lockSelected = -1;
				// Search for the lock
				for(i=0;i<MAXLOCKS;i++)
				{
					if(lockid == serverLock[i].lockid && serverLock[i].exists)
					{
						serverLock[i].lockUsedBy--;
						if(serverLock[i].state == 0 && serverLock[i].lockUsedBy == 0)
						{
							serverLock[i].exists = false;
							lockSelected = i;
							sprintf(ack,"%d",i);
							fflush(stdout);
							break;
						}
						else
						{
							strcpy(ack,"Destroyed lock copy");
							fflush(stdout);
							break;
							/*	outMailHdr.length = strlen(ack) + 1;
								outPktHdr.to = inPktHdr.from;
								outMailHdr.to = inMailHdr.from;
								postOffice->Send(outPktHdr, outMailHdr, ack);
								fflush(stdout);
								break;*/					
						}
					}
				}
				if(i == MAXLOCKS)
				{
					printf("Lock %s Does not Exist\n",lockName);
					strcpy(ack,"-1");
					fflush(stdout);
				}
				//printf("Deleted Lock=>%s with id=>%d\n",lockName,lockSelected);
				fflush(stdout);
				outMailHdr.length = strlen(ack) + 1;
				outPktHdr.to = clientMachine;
				outMailHdr.to = clientMailBox;
				if(serverMachineId == forwardingServerMachineId)
					postOffice->Send(outPktHdr, outMailHdr, ack);
				fflush(stdout);
			}
		}
			/******************** Acquire Lock *****************************/
			else if(strcmp(requestType,"Acquire") == 0)
			{
				char * lockName = requestParameters;
				int lockid = atoi(lockName);
				char *ack = new char[3];
			
				if(lockid < 0 || lockid > 100)
				{
					printf("\nCannot Acquire Lock:Invalid Lock Name \n");
					fflush(stdout);
				}
				else
				{
					// Search for the lock
					for(i=0;i<MAXLOCKS;i++)
					{
						if(lockid == serverLock[i].lockid && serverLock[i].exists)
						{
							/* Lock has been found 
								if(lock is Available)
									Let the calling process have it
								else
									Delay the reply (Q it)
							*/
							if(serverLock[i].state == 0)	/* Lock Available & no one is waiting for lock*/
							{
								printf("Lock=>%d available\n",i);
								fflush(stdout);
								serverLock[i].state = 1;	/* Lock has been acquired */
								serverLock[i].machineId = clientMachine;
								serverLock[i].mailboxNum = clientMailBox;
								sprintf(ack,"%d",i);
								fflush(stdout);
							
								/*Send Ack */
								outMailHdr.length = strlen(ack) + 1;
			
								outPktHdr.to = clientMachine;
								outMailHdr.to = clientMailBox;
								//printf("Sending Reply for Lock=>%d to %d, box %d\n",i,outPktHdr.to, outMailHdr.to);
								fflush(stdout);
								if(serverMachineId == forwardingServerMachineId)
									postOffice->Send(outPktHdr, outMailHdr, ack);
								fflush(stdout);
							}
							else
							{
								/* Lock is busy. Queue the request */
								printf("Lock=>%d is busy. Queue the request for %d %d\n",lockid,inPktHdr.from,inMailHdr.from);
								serverLock[i].machine->Append((void*)clientMachine);
								serverLock[i].mailbox->Append((void*)clientMailBox);

		/********************************** CHECK THIS PART ***************************************/							
								/* Add fetching from readyList here */
								/*if(!readyMachineIdList->IsEmpty())
								{
									printf("Delete front of ReadyList => %d\n",lockid);
									fflush(stdout);						

									serverLock[i].machineId = (int)readyMachineIdList->Remove();
									serverLock[i].mailboxNum = (int)readyMailboxNumList->Remove();
							
									sprintf(ack,"%d",i);
									fflush(stdout);
									outMailHdr.length = strlen(ack) + 1;
									outPktHdr.to = serverLock[i].machineId;
									outMailHdr.to = serverLock[i].mailboxNum;
								//	printf("Send reply to new lockowner=>%d\n",i);
									if(serverMachineId == forwardingServerMachineId)
										postOffice->Send(outPktHdr, outMailHdr, ack);
									fflush(stdout);
								}*/
		/********************************** TILL HERE ***************************************/														
							}
							break;
						}
					}
					if(i == MAXLOCKS)
					{
						printf("Lock %s Does not Exist\n",lockName);
						strcpy(ack,"-1");
						/*Send Ack */
						outMailHdr.length = strlen(ack) + 1;
			
						outPktHdr.to = clientMachine;
						outMailHdr.to = clientMailBoxNum;
						if(serverMachineId == forwardingServerMachineId)
							postOffice->Send(outPktHdr, outMailHdr, ack);
						fflush(stdout);
					}
				} 
			}
			/************************** Release Lock *****************************/
			else if(strcmp(requestType,"Release") == 0)
			{

				char * lockName = requestParameters;
				int lockid = atoi(lockName);
				char *ack = new char[3];
			
				if(lockid < 0 || lockid > 100)
				{
					printf("\nCannot Release Lock:Invalid Lock Name \n");
					fflush(stdout);
				}
				else
				{
					if(serverLock[lockid].exists)
					{
						//printf("Owner of %d=>%d %d\n",lockid,serverLock[lockid].machineId,serverLock[lockid].mailboxNum);
						//printf("Owner info=>%d %d\n",inPktHdr.from,inMailHdr.from);
						fflush(stdout);
						/* Check if requesting process is lock Owner */
						if(serverLock[lockid].machineId ==clientMachine && serverLock[lockid].mailboxNum == clientMailBox)
						{
							printf("Releasing Lock => %d\n",lockid);
							fflush(stdout);
							/* Send Release successful Ack to client */
							sprintf(ack,"%d",lockid);
							outMailHdr.length = strlen(ack) + 1;	
							outPktHdr.to = clientMachine;
							outMailHdr.to = clientMailBoxNum;
						
							if(serverMachineId == forwardingServerMachineId)
								postOffice->Send(outPktHdr, outMailHdr, ack);
							fflush(stdout);								

							/* Check if waiting Q is empty or not */
							if(!serverLock[lockid].machine->IsEmpty())	/* Q not empty*/
							{
								printf("Delete front of Q => %d\n",lockid);
								fflush(stdout);						

								serverLock[lockid].machineId = (int)serverLock[lockid].machine->Remove();
								serverLock[lockid].mailboxNum = (int)serverLock[lockid].mailbox->Remove();
							
								sprintf(ack,"%d",lockid);
								fflush(stdout);
								outMailHdr.length = strlen(ack) + 1;
								outPktHdr.to = serverLock[lockid].machineId;
								outMailHdr.to = serverLock[lockid].mailboxNum;
								if(serverMachineId == forwardingServerMachineId)
									postOffice->Send(outPktHdr, outMailHdr, ack);
								fflush(stdout);
							}
							else if(!readyMachineIdList->IsEmpty())
							{
							
			/********************************** CHECK THIS PART***************************************/							
									printf("Delete front of ReadyList => %d\n",lockid);
									fflush(stdout);						

									serverLock[lockid].machineId = (int)readyMachineIdList->Remove();
									serverLock[lockid].mailboxNum = (int)readyMailboxNumList->Remove();
							
									sprintf(ack,"%d",lockid);
									fflush(stdout);
									outMailHdr.length = strlen(ack) + 1;
									outPktHdr.to = serverLock[lockid].machineId;
									outMailHdr.to = serverLock[lockid].mailboxNum;
									serverLock[lockid].state = 0;
							//		printf("Send reply to new lockowner=>%d %d\n",outPktHdr.to,outMailHdr.to);
									if(serverMachineId == forwardingServerMachineId)
										postOffice->Send(outPktHdr, outMailHdr, ack);
									fflush(stdout);
							}
							else 
							{
								/* If Q is empty free the lock */
								serverLock[lockid].state = 0;
							}
				/********************************** TILL HERE***************************************/														
						}
						else
						{
								strcpy(ack,"Not lock Owner");
								outMailHdr.length = strlen(ack) + 1;	
								outPktHdr.to =clientMachine;
								outMailHdr.to = clientMailBox;

								if(serverMachineId == forwardingServerMachineId)
									postOffice->Send(outPktHdr, outMailHdr, ack);
								fflush(stdout);													
						}
					}
					else
					{
						printf("Lock %s Does not Exist\n",lockName);
						strcpy(ack,"-1");
						/*Send Ack */
						outMailHdr.length = strlen(ack) + 1;
			
						outPktHdr.to = inPktHdr.from;
						outMailHdr.to = inMailHdr.from;
						if(serverMachineId == forwardingServerMachineId)
							postOffice->Send(outPktHdr, outMailHdr, ack);
						fflush(stdout);
					}
				}		
			}
			/************************** Create CV *************************/
			if(strcmp(requestType,"CreateCV") == 0)
			{			
				char * cvName = requestParameters;
				char *ack = new char[3];
			
				if(cvIndex > MAXCVS)
				{
					printf("Exceeded cv limit:Cannot create more cv's");
					fflush(stdout);
				}
				else if((sizeof(cvName) < 0 || sizeof(cvName) > 100) && atoi(cvName) < 0)
				{
					printf("\nCannot create CV:Invalid CV Name \n");
					fflush(stdout);
				}
				else
				{
					int j=0;
					for(j=0;j<MAXCVS;j++)
					{
						if(strcmp(serverCondition[j].cvName,cvName) == 0 && serverCondition[j].exists)
						{
							sprintf(ack,"%d",j);
							break;
						}
					}
					if(j == MAXCVS)	/* Lock does not exist. Create it */
					{
						serverCondition[cvIndex].cvid = cvIndex;
						strcpy(serverCondition[cvIndex].cvName,cvName);
						serverCondition[cvIndex].machineId = clientMachine;
						serverCondition[cvIndex].mailboxNum = clientMailBox;
						serverCondition[cvIndex].exists = true;
						serverCondition[cvIndex].machine = new List;
						serverCondition[cvIndex].mailbox = new List;
						serverCondition[cvIndex].cvUsedBy++;
						sprintf(ack,"%d",cvIndex);
						cvIndex++ ;
						printf("CV =>%s created\n",cvName);
						fflush(stdout);
					}
					else
					{
					//	printf("CV=>%s exists. Returning its id=>%d\n",cvName,j);
						serverCondition[j].cvUsedBy++;
						printf("CV => %s exists. Returning its id",cvName);
						//printf("Count for cv:%s is =>%d\n",cvName,serverCondition[j].cvUsedBy);
						fflush(stdout);
					}
					outMailHdr.length = strlen(ack) + 1;
			
					outPktHdr.to = clientMachine;
					outMailHdr.to = clientMailBox;
					if(serverMachineId == forwardingServerMachineId)
						postOffice->Send(outPktHdr, outMailHdr, ack);
					fflush(stdout);
				}
			}
			/********************* Destroy CV **********************/
			else if(strcmp(requestType,"DestroyCV") == 0)
			{
				char * cvName = requestParameters;
				int cvid = atoi(cvName);
				char *ack = new char[25];
			
				if(cvid < 0 || cvid > 100)
				{
					printf("\nCannot delete cv:Invalid cv Name \n");
					fflush(stdout);
				}
				else
				{
					int cvSelected = -1;
		
					for(i=0;i<MAXCVS;i++)
					{
						if(cvid == serverCondition[i].cvid && serverCondition[i].exists)
						{
							serverCondition[i].cvUsedBy--;
							if(serverCondition[i].cvUsedBy == 0)
							{
								serverCondition[i].exists = false;
								cvSelected = i;
								sprintf(ack,"%d",i);
								fflush(stdout);
								break;
							}
							else
							{
								strcpy(ack,"Destroyed CV Copy");
								fflush(stdout);
								break;
							}
						}
					}
					if(i == MAXCVS)
					{
						//printf("cv %s Does not Exist\n",cvName);
						strcpy(ack,"-1");
						fflush(stdout);
					}
					//printf("Deleted cv=>%s with id=>%d\n",cvName,cvSelected);
					fflush(stdout);
					outMailHdr.length = strlen(ack) + 1;
			
					outPktHdr.to = clientMachine;
					outMailHdr.to = clientMailBox;
					if(serverMachineId == forwardingServerMachineId)
						postOffice->Send(outPktHdr, outMailHdr, ack);
					fflush(stdout);
				}
			}		
			/************************** Wait RPC **************************/
			else if(strcmp(requestType,"Wait") == 0)
			{
				char *currentStr = strtok(requestParameters,"|");
				char *cvidString =  new char[3];
				char *lockidString = new char[3];
				char *ack = new char[10];
				int k=0;
		
				while(currentStr != NULL)
				{
					if(k == 0)
					{
						fflush(stdout);
						strcpy(cvidString,currentStr);
					}
					else if(k == 1)
					{
						strcpy(lockidString,currentStr);
					}
					currentStr = strtok(NULL,"|");
					k++;
				}
			
				/* Now cvidString contains cvid string & lockidString contains lockid string */
				int cvid = atoi(cvidString);
				int lockid = atoi(lockidString);
				int p = 0;
			
				/* Check if the conditionLock & waitingLock are same */
				for(p=0;p<MAXLOCKS;p++)
				{
					if(serverLock[p].lockid == lockid && serverLock[p].exists)
					{
						/* Lockid matches*/
						if(serverLock[p].machineId == clientMachine && serverLock[p].mailboxNum == clientMailBox)
						{
							/* Current client is the owner */
							//serverLock[p].state = 0; 
							serverCondition[cvid].machine->Append((void*)clientMachine);
							serverCondition[cvid].mailbox->Append((void*)clientMailBox);
						
							/* Add fetching from readyList here */
				/********************************** CHECK THIS PART***************************************/							
								if(!readyMachineIdList->IsEmpty())	/* Q not empty*/
								{
									printf("Delete front of ReadyList => %d\n",lockid);
									fflush(stdout);						

									serverLock[lockid].machineId = (int)readyMachineIdList->Remove();
									serverLock[lockid].mailboxNum = (int)readyMailboxNumList->Remove();
							
									sprintf(ack,"%d",lockid);
									fflush(stdout);
									outMailHdr.length = strlen(ack) + 1;
									outPktHdr.to = serverLock[lockid].machineId;
									outMailHdr.to = serverLock[lockid].mailboxNum;
									serverLock[lockid].state = 0;
							//		printf("Send reply to new lockowner=>%d\n",lockid);
									if(serverMachineId == forwardingServerMachineId)
										postOffice->Send(outPktHdr, outMailHdr, ack);
									fflush(stdout);
								}
								else if(!serverLock[lockid].machine->IsEmpty())
								{
					
									/* Check if waiting Q is empty or not */
										printf("Delete front of Q => %d\n",lockid);
										fflush(stdout);						

										serverLock[lockid].machineId = (int)serverLock[lockid].machine->Remove();
										serverLock[lockid].mailboxNum = (int)serverLock[lockid].mailbox->Remove();
							
										sprintf(ack,"%d",lockid);
										fflush(stdout);
										outMailHdr.length = strlen(ack) + 1;
										outPktHdr.to = serverLock[lockid].machineId;
										outMailHdr.to = serverLock[lockid].mailboxNum;
										if(serverMachineId == forwardingServerMachineId)
											postOffice->Send(outPktHdr, outMailHdr, ack);
										fflush(stdout);
								}
								else
								{
									/* If Q is empty free the lock */
							//		printf("Nobody waiting, free the lock\n");
									fflush(stdout);
									serverLock[lockid].state = 0;
								}
				/********************************** TILL HERE***************************************/														
							break;					
						}
					}
				}
				if(p == MAXLOCKS)
				{
					/* Lock does not exist */
					strcpy(ack,"-1");
					outMailHdr.length = strlen(ack) + 1;
		
					outPktHdr.to = clientMachine;
					outMailHdr.to = clientMailBox;
					if(serverMachineId == forwardingServerMachineId)
						postOffice->Send(outPktHdr, outMailHdr, ack);
					fflush(stdout);
				}
			}
			/********************** Signal RPC *****************************/
			else if(strcmp(requestType,"Signal") == 0)
			{
				char *currentStr = strtok(requestParameters,"|");
				char *cvidString =  new char[3];
				char *lockidString = new char[3];
				char *ack = new char[10];
				int k=0;
		
				while(currentStr != NULL)
				{
					if(k == 0)
					{
						fflush(stdout);
						strcpy(cvidString,currentStr);
					}
					else if(k == 1)
					{
						strcpy(lockidString,currentStr);
					}
					currentStr = strtok(NULL,"|");
					k++;
				}

				/* Now cvidString contains cvid string & lockidString contains lockid string */
				int cvid = atoi(cvidString);
				int lockid = atoi(lockidString);
				int p = 0;
			
				for(p=0;p<MAXLOCKS;p++)
				{
					if(serverLock[p].lockid == lockid && serverLock[p].exists)
					{
						/* Lockid matches*/
						if(serverLock[p].machineId == clientMachine && serverLock[p].mailboxNum == clientMailBox)
						{
							/* Current client is the owner */
							int machineId = (int)serverCondition[cvid].machine->Remove();
							int mailboxNum = (int)serverCondition[cvid].mailbox->Remove();

							readyMachineIdList->Append((void*)machineId);
							readyMailboxNumList->Append((void*)mailboxNum);
						
						//	printf("Appending %d %d to readyList\n",machineId,mailboxNum);
							sprintf(ack,"%d",lockid);
							outMailHdr.length = strlen(ack) + 1;
		
							outPktHdr.to = clientMachine;
							outMailHdr.to = clientMailBox;
							if(serverMachineId == forwardingServerMachineId)
								postOffice->Send(outPktHdr, outMailHdr, ack);
							fflush(stdout);						
							break;					
						}
					}
				}
				if(p == MAXLOCKS)
				{
					/* Lock does not exist */
					strcpy(ack,"-1");
					outMailHdr.length = strlen(ack) + 1;
		
					outPktHdr.to = clientMachine;
					outMailHdr.to = clientMailBox;
					if(serverMachineId == forwardingServerMachineId)
						postOffice->Send(outPktHdr, outMailHdr, ack);
					fflush(stdout);
				}		
			}
			/********************** Broadcast RPC *****************************/
			else if(strcmp(requestType,"Broadcast") == 0)
			{
				char *currentStr = strtok(requestParameters,"|");
				char *cvidString =  new char[3];
				char *lockidString = new char[3];
				char *ack = new char[10];
				int k=0;
		
				while(currentStr != NULL)
				{
					if(k == 0)
					{
						fflush(stdout);
						strcpy(cvidString,currentStr);
					}
					else if(k == 1)
					{
						strcpy(lockidString,currentStr);
					}
					currentStr = strtok(NULL,"|");
					k++;
				}

				/* Now cvidString contains cvid string & lockidString contains lockid string */
				int cvid = atoi(cvidString);
				int lockid = atoi(lockidString);
				int p = 0;
			
				for(p=0;p<lockIndex;p++)
				{
					if(serverLock[p].lockid == lockid && serverLock[p].exists)
					{
						/* Lockid matches*/
						if(serverLock[p].machineId == clientMachine && serverLock[p].mailboxNum == clientMailBox)
						{
							int broadcastCount=0;
						
							/* Current client is the owner */
							while(!serverCondition[cvid].machine->IsEmpty())
							{
								broadcastCount++;		
								int machineId = (int)serverCondition[cvid].machine->Remove();
								int mailboxNum = (int)serverCondition[cvid].mailbox->Remove();

								readyMachineIdList->Append((void*)machineId);
								readyMailboxNumList->Append((void*)mailboxNum);							
							}
						
							printf("BroadcastCount=>%d",broadcastCount);
							fflush(stdout);
							sprintf(ack,"%d",broadcastCount);
							outMailHdr.length = strlen(ack) + 1;
		
							outPktHdr.to = clientMachine;
							outMailHdr.to = clientMailBox;
							if(serverMachineId == forwardingServerMachineId)
								postOffice->Send(outPktHdr, outMailHdr, ack);
							fflush(stdout);						
							break;					
						}
					}
				}
				if(p == lockIndex)
				{
					/* Lock does not exist */
					strcpy(ack,"-1");
					outMailHdr.length = strlen(ack) + 1;
		
					outPktHdr.to = clientMachine;
					outMailHdr.to = clientMailBox;
					if(serverMachineId == forwardingServerMachineId)
						postOffice->Send(outPktHdr, outMailHdr, ack);
					fflush(stdout);
				}		
			}
			/********************* Create MV RPC ***********************/
			else if(strcmp(requestType,"CreateMV") == 0)
			{
				char *currentStr = strtok(requestParameters,"|");
				char *mvName = new char[25];
				char *sizeString =  new char[3];
				char *mvLengthString = new char[3];
				char *ack = new char[10];
				int k=0;
		
				while(currentStr != NULL)
				{
					if(k == 0)
					{
						strcpy(mvName,currentStr);	
					}
					else if(k == 1)
					{
						strcpy(sizeString,currentStr);				
					}
					else if(k==2)
					{
						strcpy(mvLengthString,currentStr);
					}
					currentStr = strtok(NULL,"|");
					k++;
				}

				/* Now cvidString contains cvid string & lockidString contains lockid string */
				int size = atoi(sizeString);
				int mvLength = atoi(mvLengthString);
			
				int j=0;
				for(j=0;j<MAXMVS;j++)
				{
					if(strcmp(mv[j].mvName,mvName) == 0 && mv[j].exists)
					{
						sprintf(ack,"%d",j);
						break;
					}
				}
				if(j == MAXMVS)	 /*MV does not exist. Create it */
				{
						mv[mvIndex].mvid = mvIndex;
						strcpy(mv[mvIndex].mvName,mvName);
						mv[mvIndex].exists = true;
						mv[mvIndex].max = mvLength;
						mv[mvIndex].mvUsedBy++;
						for(int v=0;v<mvLength;v++)
						{
							mv[mvIndex].values[v] = 0;		/* Default values of MV=0*/
						}
						sprintf(ack,"%d",mvIndex);
						mvIndex++ ;
						printf("Created MV=>%s with id=>%d\n",mvName,mvIndex-1);
						fflush(stdout);
				}
				else
				{
					mv[j].mvUsedBy++;
					printf("MV=>%s exists. Returning its id=>%d\n",mvName,j);
					fflush(stdout);
				}
				outMailHdr.length = strlen(ack) + 1;
			
				outPktHdr.to = clientMachine;
				outMailHdr.to = clientMailBox;
				if(serverMachineId == forwardingServerMachineId)
					postOffice->Send(outPktHdr, outMailHdr, ack);
				fflush(stdout);
			}
			/**************************** Destroy MV RPC ***********************/
			else if(strcmp(requestType,"DestroyMV") == 0)
			{
				char * mvName = requestParameters;
				int mvid = atoi(mvName);
				char *ack = new char[20];
			
				if(mvid < 0 || mvid > 500)
				{
					printf("\nCannot delete Lock:Invalid Lock Name \n");
					fflush(stdout);
				}
				else
				{
					int mvSelected = -1;
					// Search for the lock
					for(i=0;i<MAXMVS;i++)
					{
						if(mvid == mv[i].mvid && mv[i].exists)
						{
								mv[i].mvUsedBy--;
								if(mv[i].mvUsedBy == 0)
								{
									printf("MV=>%d Destroyed\n",mvid);
									mv[i].exists = false;
									mvSelected = i;
									sprintf(ack,"%d",i);
									fflush(stdout);
									break;
								}
								else
								{
									strcat(ack,"Destroyed MV Copy");
									fflush(stdout);
									break;
								}
						}
					}
					if(i == MAXMVS)
					{
						printf("MV %s Does not Exist\n",mvName);
						strcpy(ack,"-1");
						fflush(stdout);
					}
					fflush(stdout);
					outMailHdr.length = strlen(ack) + 1;
			
					outPktHdr.to = clientMachine;
					outMailHdr.to = clientMailBox;
					if(serverMachineId == forwardingServerMachineId)
						postOffice->Send(outPktHdr, outMailHdr, ack);
					fflush(stdout);
				}
			}
			/**************************** Set MV RPC ************************/
			else if(strcmp(requestType,"SetMV") == 0)
			{
				char *currentStr = strtok(requestParameters,"|");
				char *mvidString = new char[10];
				char *mvValueIndexString =  new char[3];
				char *mvValueString = new char[3];
				char *ack = new char[10];
				int k=0;
		
				while(currentStr != NULL)
				{
					if(k == 0)
					{
						strcpy(mvidString,currentStr);	
					}
					else if(k == 1)
					{
						strcpy(mvValueIndexString,currentStr);
					}
					else if(k==2)
					{
						strcpy(mvValueString,currentStr);
					}
					currentStr = strtok(NULL,"|");
					k++;
				}

				/* Now cvidString contains cvid string & lockidString contains lockid string */
				int mvid = atoi(mvidString);
				int mvValueIndex = atoi(mvValueIndexString);
				int mvValue = atoi(mvValueString);

			//	printf("mvid=>%d mvValueIndex=>%d,mvValue=>%d\n",mvid,mvValueIndex,mvValue);
				for(int j=0;j<MAXMVS;j++)
				{
					if(mv[j].mvid == mvid && mv[j].exists)
					{
						if(mvValueIndex < mv[j].max && mvValueIndex >= 0)
						{
						//	printf("Value Before set mv[%d] to %d\n",j,mv[j].values[mvValueIndex]);
							mv[j].values[mvValueIndex] = mvValue;
						//	printf("Value set mv[%d] to %d\n",j,mv[j].values[mvValueIndex]);
							strcpy(ack,"1");
							fflush(stdout);
						}
						else
						{
							printf("MV Index out of bounds\n");
							strcpy(ack,"-1");
							fflush(stdout);
						}
			/*			for(int p=0;p<mv[j].max;p++)
						{
						../network/nettest.cc:742:	printf("%d ",mv[j].values[p]);
							fflush(stdout);
						}*/
						outMailHdr.length = strlen(ack) + 1;
			
						outPktHdr.to = clientMachine;
						outMailHdr.to = clientMailBox;
						if(serverMachineId == forwardingServerMachineId)
							postOffice->Send(outPktHdr, outMailHdr, ack);
						fflush(stdout);
						break;
					}
				}			
			}
			/************************** Get MV RPC ****************************/
			else if(strcmp(requestType,"GetMV") == 0)
			{
				char *currentStr = strtok(requestParameters,"|");
				char *mvidString = new char[10];
				char *mvValueIndexString =  new char[3];
				char *ack = new char[10];
				int k=0;
		
				while(currentStr != NULL)
				{
					if(k == 0)
					{
						strcpy(mvidString,currentStr);	
					}
					else if(k == 1)
					{
						strcpy(mvValueIndexString,currentStr);
					}
					currentStr = strtok(NULL,"|");
					k++;
				}

				/* Now cvidString contains cvid string & lockidString contains lockid string */
				int mvid = atoi(mvidString);
				int mvValueIndex = atoi(mvValueIndexString);
		
				for(int j=0;j<MAXMVS;j++)
				{
					if(mv[j].mvid == mvid && mv[j].exists)
					{
						if(mvValueIndex < mv[j].max && mvValueIndex >= 0)
						{					
							int mvValue = mv[j].values[mvValueIndex] ;
							sprintf(ack,"%d",mvValue);
							fflush(stdout);
						}
						else
						{
							printf("MV Index out of bounds\n");
							strcpy(ack,"-1");
							fflush(stdout);
						}
			/*			for(int p=0;p<mv[j].max;p++)
						{
							printf("%d ",mv[j].values[p]);
							fflush(stdout);
						}*/
						outMailHdr.length = strlen(ack) + 1;
			
						outPktHdr.to = clientMachine;
						outMailHdr.to = clientMailBox;
						if(serverMachineId == forwardingServerMachineId)
							postOffice->Send(outPktHdr, outMailHdr, ack);
						fflush(stdout);
						break;
					}
				}			
			}
}
