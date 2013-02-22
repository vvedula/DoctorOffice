//ptablearr is the array of processes in the process table. It has 

// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from uhser
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>
#include "noff.h"

using namespace std;

void kernelThread(unsigned int);

/*********************** Copyin Function *****************************/
int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

/*********************** Copyout Function *****************************/
int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

/*********************** Create System Call *****************************/
void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	DEBUG('s',"%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

/*********************** Open System Call *****************************/
int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	DEBUG('s',"%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	DEBUG('s',"%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

/*********************** Write System Call *****************************/
void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	DEBUG('s',"%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    DEBUG('s',"%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	DEBUG('s',"%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    DEBUG('s',"%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

/*********************** Read System Call *****************************/
int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	DEBUG('s',"%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	DEBUG('s',"%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    DEBUG('s',"%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    DEBUG('s',"%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

/*********************** Close System Call *****************************/
void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      DEBUG('s',"%s","Tried to close an unopen file\n");
    }
}

/******************** Yield System Call *******************/
void Yield_Syscall()
{
	DEBUG('a',"Inside yield syscall\n");
	currentThread->Yield();
}

/******************** Create Lock System Call *******************/
int CreateLock_Syscall(int vaddr,int size)
{
	#ifdef NETWORK
	//printf("% d inside CreateLock Syscall\n",currentThread->space->procid);
	PacketHeader outPktHdr, inPktHdr;	// PacketHeader	=~	IP Address
	MailHeader outMailHdr, inMailHdr;	// MailHeader	=~	Port Number
	char *lockName = new char[10];
	char *data = new char[50];
	char *ack = new char[3];

	if( (copyin(vaddr,size,lockName)) == -1 )
	{
		printf("%s","Bad pointer passed to CreateLock\n");
		delete lockName;
		return -1;
	}
	/*if(atoi(lockName[0]) <0 || atoi(lockName[0]) >0)
	{
		printf("Cannot Create Lock:Lock Name %s is invalid",lockName);
		delete lockName;
		return -1;
	}*/
	
	/* Now lockName contains name of lock passed to CreateLock */
	
	/* Set the Packet & Mail Headers for the server instance */
	outPktHdr.to = rand()%numServers;
	outMailHdr.to = 1;		// Server port
	outMailHdr.from = currentThread->space->procid;		// Client port
		
	strcpy(data,"Client:CreateLock*");
	strcat(data,lockName);
	
	outMailHdr.length = strlen(data)+1;
	
	/* Data sent to server is CreateLock*LockName*/
	//printf("%d Sending data %s to Server %d\n",currentThread->space->procid,data,outPktHdr.to);
	bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	fflush(stdout);
	
	if ( !success )
	{
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	
	postOffice->Receive(outMailHdr.from, &inPktHdr, &inMailHdr, ack);
	
	fflush(stdout);
	int index = atoi(ack);
	//printf("%d Lock created=>%d\n",currentThread->space->procid,index);
	fflush(stdout);
	return index;
	 #endif
	 /* Add a return statement to avoid getting "control reaches end of non-void function" warning */
	 return 0;
	
	#ifdef USER_PROG
	osLock->Acquire();
	if(size <= 0)
	{
		printf("\nCannot create Lock:Lock size < 0\n");
		DEBUG('s',"Cannot create Lock:Lock size < 0\n");
		osLock->Release();
		return -1;
	}
	else if(size > 100)
	{
		printf("\nCannot create Lock:Lock size > Limit\n");
		DEBUG('s',"Cannot create Lock:Lock size > Limit\n");
		osLock->Release();
		return -1;
	}
	if(vaddr < 0)
	{
		printf("\nCannot create Lock:Invalid virtual address passed\n");
		DEBUG('s',"Cannot create Lock:Invalid virtual address passed\n");
		osLock->Release();
		return -1;
	}
	char* mybuf;
	mybuf = new char[size+1];
	
	/*The copyin( ) function copies the specified number of bytes from user space to kernel space*/
	
	if(copyin(vaddr,size,mybuf) == -1) 
	{
		printf("\nCannot Create Lock:Invalid virtual address\n");
		DEBUG('a',"Cannot Create Lock:Invalid Virtual Address\n");
		osLock->Release();
		return -1;
	}
	for(int i=0;i<totalLocks;i++)
	{
		if(kernelLock[i].exists == false)
		{
			kernelLock[i].exists = true;
			kernelLock[i].addrSpace=currentThread->space;
			kernelLock[i].count = 0;
			kernelLock[i].lock=new Lock(mybuf);
			osLock->Release();
			return i;
		}
	}
	osLock->Release();
	printf("\nCannot Create Lock:Max limit for locks exceeded\n"); 
	DEBUG('a',"Cannot Create Lock:Max limit for locks exceeded\n");
	return -1;
	 #endif
	 /* Add a return statement to avoid getting "control reaches end of non-void function" warning */
	 return 0;
}

/******************** Destroy Lock System Call *******************/
void DestroyLock_Syscall(unsigned int lockid)
{
	#ifdef NETWORK
	//printf("%d inside DestroyLock Syscall\n",currentThread->space->procid);
	PacketHeader outPktHdr, inPktHdr;	// PacketHeader	=~	IP Address
	MailHeader outMailHdr, inMailHdr;	// MailHeader	=~	Port Number
	char *data = new char[50];
	char lockidString[3];
	char *ack = new char[25];

	if(lockid < 0 || lockid > 100 )
	{
		printf("%s","Cannot Destroy Lock:Incorrect id\n");
		return;
	}
	/* Now lockName contains name of lock passed to CreateLock */
	
	/* Set the Packet & Mail Headers for the server instance */
	outPktHdr.to = 0;		
	outPktHdr.from = 1;
	outMailHdr.to = 1;		// Server port
	outMailHdr.from = currentThread->space->procid;		// Client port
	
	strcpy(data,"Client:DestroyLock*");
	
	sprintf(lockidString,"%d",lockid);
	strcat(data,lockidString);
//	printf("Sending data %s to Destroy\n",data);
	fflush(stdout);
	
	outMailHdr.length = strlen(data)+1;
	
	/* Data sent to server is DestroyLock*LockName*/
	bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	fflush(stdout);
	
	if ( !success )
	{
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	
	postOffice->Receive(outMailHdr.from, &inPktHdr, &inMailHdr, ack);	
	fflush(stdout);

	if(strcmp(ack,"Destroyed lock copy") == 0)
	{
	//	printf("%d  Lock=>%d  copy has been destroyed\n",currentThread->space->procid,lockid);
		fflush(stdout);
	}
	int index = atoi(ack);
	if(index == -1 && strcmp(ack,"Destroyed lock copy")!= 0)
	{
		printf("Cannot Destroy:Lock=>%d does not Exist\n",lockid);
		fflush(stdout);
	}
	else if(index != -1 && strcmp(ack,"Destroyed lock copy")!=  0)
	{
	//	printf("%d Final Lock=>%d  copy has been destroyed\n",currentThread->space->procid,lockid);
		fflush(stdout);
	}
	 #endif
	 
	#ifdef USER_PROG
	osLock->Acquire();
	if(lockid < 0)
	{
		printf("Cannot Destroy Lock:Lock id < 0\n");
		DEBUG('a',"%s","Cannot Destroy Lock:Lock id < 0\n");
		osLock->Release();
		return;
	}
	else if(lockid > 100 )
	{
		printf("Cannot Destroy Lock: Lock id > 100\n");
		DEBUG('a',"%s","Cannot Destroy Lock: Lock id > Limit\n");
		osLock->Release();
		return;
	}	
	if(kernelLock[lockid].addrSpace != currentThread->space )
	{
		printf("\nCannot Destroy Lock:Lock is not within Address space of currentThread\n");
		DEBUG('a',"Cannot Destroy Lock:Lock is not within Address space of currentThread\n");
		osLock->Release();
		return;
	}
	if(kernelLock[lockid].exists == false)
	{
		printf("\nCannot Destroy Lock:Lock does not exist\n");
		DEBUG('a',"Cannot Destroy Lock:Lock does not exist\n");
		osLock->Release();
		return;
	} 
	if(kernelLock[lockid].count > 0)
	{
		kernelLock[lockid].toBeDestroyed = true;
		printf("\nCannot destroy Lock: Lock acquired by some other thread\n");
		DEBUG('s',"Cannot destroy Lock: Lock acquired by some other thread\n");
		osLock->Release();
		return;
	}    
	if(kernelLock[lockid].count == 0)
	{
		printf("\nDestroying Lock...\n");	
		kernelLock[lockid].exists= false;
		osLock->Release();
		return;
    	}
	if(kernelLock[lockid].count == 0 &&  kernelLock[lockid].toBeDestroyed == true)
	{
		printf("\nDestroying Lock...\n");	
		kernelLock[lockid].exists= false;
		osLock->Release();
		return;
	}
	#endif
}

/******************** Acquire Lock System Call *******************/
void Acquire_Syscall( int lockid)
{
	#ifdef NETWORK
	//printf("%d Inside Acquire Syscall\n",currentThread->space->procid);
	PacketHeader outPktHdr, inPktHdr;	// PacketHeader	=~	IP Address
	MailHeader outMailHdr, inMailHdr;	// MailHeader	=~	Port Number
	char *data = new char[50];
	char lockidString[3];
	char *ack = new char[3];
	
	if(lockid < 0 || lockid > 100 )
	{
		printf("%s","Cannot Acquire Lock:Incorrect id\n");
		return;
	}
	/* Now lockName contains name of lock passed to CreateLock */
	
	/* Set the Packet & Mail Headers for the server instance */
	outPktHdr.to = 0;	
	outMailHdr.to = 1;		// Server port
	outMailHdr.from = currentThread->space->procid;		// Client port
		
	strcpy(data,"Client:Acquire*");
	
	sprintf(lockidString,"%d",lockid);
	strcat(data,lockidString);
	
	outMailHdr.length = strlen(data)+1;
	
	/* Data sent to server is DestroyLock*LockName*/
	//printf("%d Sending request to server for Acquire=>%d\n",currentThread->space->procid,lockid);
	bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	fflush(stdout);
	
	if ( !success )
	{
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	postOffice->Receive(outMailHdr.from, &inPktHdr, &inMailHdr, ack);
	fflush(stdout);
	int index = atoi(ack);
	if(index != -1)
	{
	//	printf("Lock=>%d has been Acquired by %d\n",lockid,currentThread->space->procid);
	}
	else
	{
		printf("Lock could not be Acquired\n");
	}
	#endif
	#ifdef USER_PROG
	osLock->Acquire();	  
	if(lockid < 0 )
	{
		printf("Cannot Acquire Lock:Lock id < 0 \n");
		DEBUG('s',"%s","Cannot Acquire Lock:Lock id < 0 \n");
		osLock->Release();
		return;
	}
	if(lockid > 100)
	{
		printf("\nCannot Acquire Lock:Lock id > Limit \n");
		DEBUG('s',"%s","Cannot Acquire Lock:Lock id > Limit\n");
		osLock->Release();
		return;
	}
	if(kernelLock[lockid].exists == false)
	{
		printf("Cannot Acquire Lock:Lock does not exist\n");
		DEBUG('s',"%s","Cannot Acquire Lock:Lock id > Limit\n");
		osLock->Release();
		return;
	}		
	if(kernelLock[lockid].addrSpace != currentThread->space)
	{
		printf("\nCannot Acquire Lock:Lock is not within Address space of currentThread\n");
		DEBUG('s',"%s","Cannot Destroy Lock:Lock is not within Address space of currentThread\n");
		osLock->Release();
		return;
	}
        kernelLock[lockid].count++;
	osLock->Release();
	kernelLock[lockid].lock->Acquire();
	return;
	#endif
}

/******************** Release Lock System Call *******************/
void Release_Syscall(unsigned int lockid)
{
	#ifdef NETWORK
	//printf("%d inside Release Syscall\n",currentThread->space->procid);
	PacketHeader outPktHdr, inPktHdr;	// PacketHeader	=~	IP Address
	MailHeader outMailHdr, inMailHdr;	// MailHeader	=~	Port Number
	char *data = new char[50];
	char lockidString[3];
	char *ack = new char[20];
	
	if(lockid < 0 || lockid > 100 )
	{
		printf("%s","Cannot Release Lock:Incorrect id\n");
		return;
	}
	/* Now lockName contains name of lock passed to CreateLock */
	
	/* Set the Packet & Mail Headers for the server instance */
	outPktHdr.to = 0;
	outMailHdr.to = 1;		// Server port
	outMailHdr.from = currentThread->space->procid;		// Client port
	
	strcpy(data,"Client:Release*");
	
	sprintf(lockidString,"%d",lockid);
	strcat(data,lockidString);
	
	outMailHdr.length = strlen(data)+1;
	
	/* Data sent to server is DestroyLock*LockName*/
	//printf("%d Send request to server for release=>%d\n",currentThread->space->procid,lockid);
	bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	fflush(stdout);
	
	if ( !success )
	{
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	postOffice->Receive(outMailHdr.from, &inPktHdr, &inMailHdr, ack);
	fflush(stdout);
	int index = atoi(ack);
	if(index != -1)
	{
	//	printf("Received reply from Server %d\n",inPktHdr.from);
	//	printf("Lock=>%d has been Released by %d\n",lockid,currentThread->space->procid);
	}
	else
	{
	}
	return;
	#endif	
	#ifdef USER_PROG
	osLock->Acquire();
	if(lockid < 0 )
	{
		printf("\nCannot Release Lock:Lock id < 0\n");
		DEBUG('s',"%s","Cannot Release Lock:Lock id < 0\n");
		osLock->Release();
		return;
	}
	if(lockid > 100)
	{
		printf("\nCannot Release Lock:Lock id > Limit\n");
		DEBUG('s',"%s","Cannot Release Lock:Lock id > Limit\n");
		osLock->Release();
		return;
	}
	if(kernelLock[lockid].exists == false)
	{
		printf("\nCannot Release Lock:Lock Does not exist\n");
		DEBUG('s',"%s","Cannot Release Lock:Lock Does not exist\n");
		osLock->Release();
		return;
	}	
	if(kernelLock[lockid].addrSpace != currentThread->space)
	{
		printf("\nCannot Release Lock:Lock is not within Address space of currentThread\n");
		DEBUG('s',"%s","Cannot Release Lock:Lock is not within Address space of currentThread\n");
		osLock->Release();
		return;
	}
	kernelLock[lockid].count--;
	osLock->Release();
	/*printf("\nReleasing Lock...");*/
	kernelLock[lockid].lock->Release();
	return;
	#endif
}

/****************** Create CV System Call **************************/
int CreateCV_Syscall(int vaddr,int size)
{
	#ifdef NETWORK
	//printf("%d Inside CreateCV Syscall\n",currentThread->space->procid);
	PacketHeader outPktHdr, inPktHdr;	// PacketHeader	=~	IP Address
	MailHeader outMailHdr, inMailHdr;	// MailHeader	=~	Port Number
	char *cvName = new char[10];
	char *data = new char[50];
	char *ack = new char[3];

	if( (copyin(vaddr,size,cvName)) == -1 )
	{
		printf("%s","Bad pointer passed to CreateCV\n");
		delete cvName;
		return -1;
	}
	cvName[size]='\0';
	/* Now lockName contains name of lock passed to CreateLock */
	
	/* Set the Packet & Mail Headers for the server instance */
	outPktHdr.to = 0;
	outMailHdr.to = 1;		// Server port
	outMailHdr.from = currentThread->space->procid;		// Client port
		
	strcpy(data,"Client:CreateCV*");
	strcat(data,cvName);
	
	outMailHdr.length = strlen(data)+1;
	
	/* Data sent to server is CreateCV*cvName*/
	//printf("%d Sending data %s to CreateCV\n",currentThread->space->procid,cvName);
	fflush(stdout);
	bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	fflush(stdout);
	
	if ( !success )
	{
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	
	postOffice->Receive(outMailHdr.from, &inPktHdr, &inMailHdr, ack);
	
	fflush(stdout);
	int index = atoi(ack);
	//printf("CV=>%d created by %d\n",index,currentThread->space->procid);
	fflush(stdout);
	return index;
	 #endif
	return 0;

	#ifdef USER_PROG
	cvLock->Acquire();  
	if(size <= 0 )
	{
		printf("\nCannot Create CV: Size < 0\n");
		DEBUG('s',"\nCannot Create CV: Size < 0\n");
		cvLock->Release();
		return -1;
	}
	
	else if(size >100)
	{
		printf("\nCannot Create CV: Size > Limit\n");
		DEBUG('s',"\nCannot Create CV: Size > Limit\n");
 		cvLock->Release();
		return -1;

	}
	if(vaddr < 0)
	{
		printf("\nCannot Create CV: Invalid Virtual Address\n");
		DEBUG('s',"\nCannot Create CV: Invalid Virtual Address\n");
		cvLock->Release();
		return -1;
	}
   
	char* mybuf;
	mybuf = new char[size];
  
	if(copyin(vaddr, size, mybuf) == -1) 
	{
		printf("\nCannot Create CV: Invalid Virtual Address\n");
		DEBUG('s',"\nCannot Create CV: Invalid Virtual Address\n");
		cvLock->Release();
		return -1;
	}
	
	for(int i=0;i<totalCVs;i++)
	{
		if(CVList[i].exists == false)
		{
			CVList[i].exists = true;				
			CVList[i].addrSpace = currentThread->space;
			CVList[i].count = 0;
			CVList[i].condition=new Condition(mybuf);
			cvLock->Release();	
			return i;
		}
	}
	cvLock->Release();
	printf("\nCannot Create CV:Max limit for CVs exceeded\n"); 
	DEBUG('a',"Cannot Create CV:Max limit for CVs exceeded\n");
	return -1;
	#endif
	return 0;
}

/***************************Destroy CV System Call*********************/
void DestroyCV_Syscall(unsigned int CVid)
{
	#ifdef NETWORK
	//printf("%d inside DestroyCv Syscall\n",currentThread->space->procid);
	PacketHeader outPktHdr, inPktHdr;	// PacketHeader	=~	IP Address
	MailHeader outMailHdr, inMailHdr;	// MailHeader	=~	Port Number
	char *data = new char[50];
	char cvidString[3];
	char *ack = new char[20];

	if(CVid < 0 || CVid > 100 )
	{
		printf("%s","Cannot Destroy CV:Incorrect id\n");
		return;
	}
	/* Now lockName contains name of lock passed to CreateLock */
	
	/* Set the Packet & Mail Headers for the server instance */
	outPktHdr.to = 0;		
	outPktHdr.from = 1;
	outMailHdr.to = 1;		// Server port
	outMailHdr.from = currentThread->space->procid;		// Client port
	
	strcpy(data,"Client:DestroyCV*");
	
	sprintf(cvidString,"%d",CVid);
	strcat(data,cvidString);
//	printf("Sending data %s to Destroy\n",data);
	fflush(stdout);
	
	outMailHdr.length = strlen(data)+1;
	
	/* Data sent to server is DestroyCV*CvId*/
	bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	fflush(stdout);
	
	if ( !success )
	{
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	
	postOffice->Receive(outMailHdr.from, &inPktHdr, &inMailHdr, ack);
	
	if(strcmp(ack,"Destroyed CV Copy") == 0)
	{
	//	printf("CV=>%d  copy has been destroyed by %d\n",CVid,currentThread->space->procid);
		fflush(stdout);
	}
	int index = atoi(ack);
	if(index == -1 && strcmp(ack,"Destroyed CV Copy")!= 0)
	{
		printf("Cannot Destroy:CV=>%d does not Exist\n",CVid);
		fflush(stdout);
	}
	else if(index != -1 && strcmp(ack,"Destroyed CV Copy")!=  0)
	{
	//	printf("Final CV=>%d  copy has been destroyed by %d\n",CVid,currentThread->space->procid);
		fflush(stdout);
	}
	 #endif

	#ifdef USER_PROG
	cvLock->Acquire();

	if(CVid < 0)
	{	
		printf("\nCannot Destroy CV: CV id < 0\n");
		DEBUG('s',"\nCannot Destroy CV: CV id < 0\n");
		cvLock->Release();
		return;
	}

	else if(CVid > 100)
	{
		printf("\nCannot Destroy CV: CV id > Limit\n");
		DEBUG('s',"\nCannot Destroy CV: CV id > Limit\n");
		cvLock->Release();
		return;
	}
	if(CVList[CVid].addrSpace != currentThread->space)
	{
		printf("\nCannot Destroy CV:CV is not within Address space of currentThread\n");
		DEBUG('s',"\nCannot Destroy CV:CV is not within Address space of currentThread\n");
		cvLock->Release();
		return;
	}
	if(CVList[CVid].exists == false)
	{
		printf("\nCannot Destroy CV: CV does not exist\n");	
		DEBUG('s',"\nCannot Destroy CV: CV does not exist\n");
		cvLock->Release();
		return;
	}
	if(CVList[CVid].count > 0)
	{
		CVList[CVid].toBeDestroyed = true;
		printf("Cannot Destroy CV: CV is being used\n");
		DEBUG('s',"Cannot Destroy CV: CV is being used\n");
		cvLock->Release();
		return;
	}
	if(CVList[CVid].count==0)
	{        
		printf("\nDestroying CV... \n");
		DEBUG('s',"Destroying CV...\n");
		CVList[CVid].exists = false;
		cvLock->Release();
		return;
	}
	if(CVList[CVid].count == 0 && CVList[CVid].toBeDestroyed == true)
	{        
		printf("\nDestroying CV... \n");
		DEBUG('s',"Destroying CV...\n");
		CVList[CVid].exists = false;
		cvLock->Release();
		return;
	}
	#endif
}

/*******************************Signal System Call *******************************/
void Signal_Syscall(unsigned int CVid, int lockid)
{
	#ifdef NETWORK
	//printf("%d inside Signal Syscall\n",currentThread->space->procid);
	PacketHeader outPktHdr, inPktHdr;	// PacketHeader	=~	IP Address
	MailHeader outMailHdr, inMailHdr;	// MailHeader	=~	Port Number

	char *data = new char[50];
	char cvidString[3],lockidString[3];
	char *ack = new char[3];

	if(lockid < 0 || lockid > 100)
	{
		printf("Cannot Wait:Incorrect Lockid\n");
		return ;
	}
	if(CVid < 0 || CVid > 100)
	{
		printf("Cannot Wait:Incorrect cvid\n");
		return ;
	}
		
	/* Set the Packet & Mail Headers for the server instance */
	outPktHdr.to = 0;		
	outPktHdr.from = 1;
	outMailHdr.to = 1;		// Server port
	outMailHdr.from = currentThread->space->procid;		// Client port
	
	strcpy(data,"Client:Signal*");
	
	sprintf(cvidString,"%d",CVid);
	strcat(data,cvidString);
	strcat(data,"|");
	sprintf(lockidString,"%d",lockid);
	strcat(data,lockidString);
	
	//printf("%d Sending data %s to Signal\n",currentThread->space->procid,data);
	fflush(stdout);
	
	outMailHdr.length = strlen(data)+1;
	
	/* Data sent to server is Wait*CvId*/
	bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	fflush(stdout);
	
	if ( !success )
	{
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	
	postOffice->Receive(outMailHdr.from, &inPktHdr, &inMailHdr, ack);
	fflush(stdout);
	int index = atoi(ack);
	if(index == -1)
	{
		printf("Lock=>%d does not exist for%d\n",lockid,currentThread->space->procid);
	}

	return;
	#endif
	
	#ifdef USER_PROG
	osLock->Acquire();

	if(lockid < 0)
	{
		printf("\nCannot Signal:Lock id < 0\n");
		DEBUG('s',"Cannot Signal:Lock id < 0\n");
		osLock->Release();
		return;
	}
	else if (lockid > 100)
	{
		printf("\nCannot Signal:Lock id > Limit\n");
		DEBUG('s',"Cannot Signal:Lock id > Limit\n");
		osLock->Release();
		return;	
	}
	if(kernelLock[lockid].exists == false)
	{
		printf("\nCannot Signal:Lock does not exist\n");
		DEBUG('s',"Cannot Signal:Lock does not exist\n");
		osLock->Release();
		return;
    	} 
	if(kernelLock[lockid].addrSpace != currentThread->space || kernelLock[lockid].exists == false)
	{
		printf("\nCannot Signal:Lock belongs to different process\n");
		DEBUG('s',"Cannot Signal:Lock belongs to different process\n");
		osLock->Release();
		return;
	}	

	cvLock->Acquire();
	if(CVid < 0)
	{
		printf("\nCannot Signal:CV id < 0\n");
		DEBUG('s',"Cannot Signal:CV id < 0\n");
		cvLock->Release();
		return;
	}
	else if(CVid > 100)
	{
		printf("\nCannot Signal:CV id > Limit\n");
		DEBUG('s',"Cannot Signal:CV id > Limit\n");
		cvLock->Release();
		return;	
	}
	if(CVList[CVid].addrSpace != currentThread->space || CVList[CVid].exists == false)
	{
		printf("\nCannot Signal:CV belongs to different process\n");
		DEBUG('s',"Cannot Signal:CV belongs to different process \n");
		cvLock->Release();
		return;
	}
	if(CVList[CVid].exists == false)
	{
		printf("\nCannot Signal: CV does not exist\n");
		DEBUG('s',"Cannot Signal:CV does not exist\n");
		cvLock->Release();
		return;
	}
	cvLock->Release();
	osLock->Release();
	CVList[CVid].count--;
	CVList[CVid].condition->Signal(kernelLock[lockid].lock);
	return;
	#endif
}

/*****************************Wait System Call ****************************/
void Wait_Syscall(unsigned int CVid, int lockid)
{
	#ifdef NETWORK
	//printf("%d inside Wait Syscall\n",currentThread->space->procid);
	PacketHeader outPktHdr, inPktHdr;	// PacketHeader	=~	IP Address
	MailHeader outMailHdr, inMailHdr;	// MailHeader	=~	Port Number

	char *data = new char[50];
	char cvidString[3],lockidString[3];
	char *ack = new char[3];

	if(lockid < 0 || lockid > 100)
	{
		printf("Cannot Wait:Incorrect Lockid\n");
		return ;
	}
	if(CVid < 0 || CVid > 100)
	{
		printf("Cannot Wait:Incorrect cvid\n");
		return ;
	}	
	/* Set the Packet & Mail Headers for the server instance */
	outPktHdr.to = 0;		
	outPktHdr.from = 1;
	outMailHdr.to = 1;		// Server port
	outMailHdr.from = currentThread->space->procid;		// Client port
	
	strcpy(data,"Client:Wait*");
	
	sprintf(cvidString,"%d",CVid);
	strcat(data,cvidString);
	strcat(data,"|");
	sprintf(lockidString,"%d",lockid);
	strcat(data,lockidString);
	
	//printf("%d Sending data %s to Wait\n",currentThread->space->procid,data);
	fflush(stdout);
	
	outMailHdr.length = strlen(data)+1;
	
	/* Data sent to server is Wait*CvId*/
	bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	fflush(stdout);
	
	if ( !success )
	{
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	
	postOffice->Receive(outMailHdr.from, &inPktHdr, &inMailHdr, ack);
	fflush(stdout);
	int index = atoi(ack);
	if(index == -1)
	{
		printf("Lock=>%d does not exist %d\n",lockid,currentThread->space->procid);
	}
	else
	{
	//	printf("%d out of Wait\n",currentThread->space->procid);
	}

	return;
	#endif
	
	#ifdef USER_PROG
	osLock->Acquire();

	if(lockid < 0)
	{
		printf("\nCannot Wait: lock id < 0\n");
		DEBUG('s',"Cannot Wait: lock id < 0\n");
		osLock->Release();
		return;
	}
	else if(lockid > 100)
	{
		printf("\nCannot Wait: lock id > Limit\n");
		DEBUG('s',"Cannot Wait: lock id > Limit\n");
		osLock->Release();
		return;	
	}
	
	if(kernelLock[lockid].exists == false)
	{
		printf("\nCannot Wait: Lock does not exist\n");
		DEBUG('s',"Cannot Wait: Lock does not exist\n");
		osLock->Release();
		return;
	}
	if(kernelLock[lockid].addrSpace != currentThread->space)
	{
		printf("\nCannot Wait:Address space is different\n");
		DEBUG('s',"Cannot Wait: Address space is different\n");
		osLock->Release();
		return;
	}
	
	cvLock->Acquire();

	if(lockid < 0)//illegal condition number
	{
		printf("\nCannot Wait: CV id < 0\n");
		DEBUG('s',"Cannot Wait: CV id < 0\n");
		cvLock->Release();
		return;
	}
	else if(lockid > 100)
	{
		printf("\nCannot Wait: CV id > Limit\n");
		DEBUG('s',"Cannot Wait: CV id > Limit\n");
		cvLock->Release();
		return;	
	}
	if(CVList[CVid].exists == false)
	{
		printf("\nCannot Wait:CV does not exist\n");
		DEBUG('s',"Cannot Wait:CV does not exist\n");
		cvLock->Release();
		return;
	}
	if(CVList[CVid].addrSpace != currentThread->space)
	{
		printf("\nCannot Wait:CV has different Address Space\n");
		DEBUG('s',"Cannot Wait:CV has different Address Space\n");
		cvLock->Release();
		return;
	}
	CVList[CVid].count++;
	cvLock->Release();
	osLock->Release();
	CVList[CVid].condition->Wait(kernelLock[lockid].lock);
	return;
	#endif
}

/********************** BroadCast System Call *****************************/
void Broadcast_Syscall( int CVid,int lockid)
{
	#ifdef NETWORK
	//printf("%d inside Broadcast Syscall\n",currentThread->space->procid);
	PacketHeader outPktHdr, inPktHdr;	// PacketHeader	=~	IP Address
	MailHeader outMailHdr, inMailHdr;	// MailHeader	=~	Port Number

	char *data = new char[50];
	char cvidString[3],lockidString[3];
	char *ack = new char[3];

	if(lockid < 0 || lockid > 100)
	{
		printf("Cannot Broadcast:Incorrect Lockid\n");
		return ;
	}
	if(CVid < 0 || CVid > 100)
	{
		printf("Cannot Broadcast:Incorrect cvid\n");
		return ;
	}
		
	/* Set the Packet & Mail Headers for the server instance */
	outPktHdr.to = 0;		
	outPktHdr.from = 1;
	outMailHdr.to = 1;		// Server port
	outMailHdr.from = currentThread->space->procid;		// Client port
	
	strcpy(data,"Client:Broadcast*");
	
	sprintf(cvidString,"%d",CVid);
	strcat(data,cvidString);
	strcat(data,"|");
	sprintf(lockidString,"%d",lockid);
	strcat(data,lockidString);
	
	//printf("%d Sending data %s to Broadcast\n",currentThread->space->procid,data);
	fflush(stdout);
	
	outMailHdr.length = strlen(data)+1;
	
	/* Data sent to server is Broadcast*CvId*/
	bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	fflush(stdout);
	
	if ( !success )
	{
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	
	postOffice->Receive(outMailHdr.from, &inPktHdr, &inMailHdr, ack);
	fflush(stdout);
	int index = atoi(ack);
	if(index == -1)
	{
		printf("Lock=>%d does not exist\n",lockid);
	}
	else
	{
	//	printf("Broadcast by %d to clients successfull\n",currentThread->space->procid);
		fflush(stdout);
	}

	return;
	#endif
	
	#ifdef USER_PROG
	osLock->Acquire();

	if(lockid < 0)
	{
		printf("\nCannot Broadcast: lockid < 0\n");
		DEBUG('s',"Cannot Broadcast: lockid < 0\n");
		osLock->Release();
		return;
	}
	else if(lockid > 100)
	{
		printf("\nCannot Broadcast: lockid > Limit\n");
		DEBUG('s',"Cannot Broadcast: lockid > Limit\n");
		osLock->Release();
		return;	
	}
	if(kernelLock[lockid].exists == false)
	{
		printf("\nCannot Broadcast: Lock does not exist\n");
		DEBUG('s',"Cannot Broadcast: Lock does not exist\n");
		osLock->Release();
		return;
	}
	if(kernelLock[lockid].addrSpace != currentThread->space )
	{
		printf("\nCannot Broadcast:Lock has different Address Space\n");
		DEBUG('s',"Cannot Broadcast:Lock has different Address Space\n");
		osLock->Release();
		return;
	}
	
	cvLock->Acquire();

	if(CVid < 0)
	{
		printf("\nCannot Broadcast: CVid < 0\n");
		DEBUG('s',"Cannot Broadcast: CVid < 0\n");
		cvLock->Release();
		return;
	}
	else if(CVid > 100)
	{
		printf("\nCannot Broadcast: CVid > Limit\n");
		DEBUG('s',"Cannot Broadcast: CVid > Limit\n");
		cvLock->Release();
		return;
	}
	if(CVList[CVid].exists == false)
	{
		printf("\nCannot Broadcast: CV does not exist\n");
		DEBUG('s',"Cannot Broadcast:CV does not exist\n");
		cvLock->Release();
		return;
	}
	if(CVList[CVid].addrSpace != currentThread->space)
	{
		printf("\nCannot Broadcast:CV has different Address Space\n");
		DEBUG('s',"Cannot Broadcast:CV has different Address Space\n");
		osLock->Release();
		return;
	}

	//printf("\nBroadcasting...\n");
	
	int numBroadcasts = CVList[CVid].condition->Broadcast(kernelLock[lockid].lock);

	CVList[CVid].count = CVList[CVid].count - numBroadcasts;
	cvLock->Release();

	kernelLock[lockid].count = kernelLock[lockid].count - numBroadcasts;
	osLock->Release();

	return;
	#endif
}

/*********************** Create MV  RPC **************************************/
int CreateMV_Syscall(int vaddr,int size, int mvSize)
{
	#ifdef NETWORK
	//printf("%d inside CreateMV Syscall\n",currentThread->space->procid);
	PacketHeader outPktHdr, inPktHdr;	// PacketHeader	=~	IP Address
	MailHeader outMailHdr, inMailHdr;	// MailHeader	=~	Port Number
	char *mvName = new char[25];
	char *data = new char[50];
	char *ack = new char[3];
	char *sizeString = new char[3];
	char *mvLengthString = new char[3];

	if( (copyin(vaddr,size,mvName)) == -1 )
	{
		printf("%s","Bad pointer passed to CreateLock\n");
		delete mvName;
		return -1;
	}
	mvName[size]='\0';
	if(size < 0 || size > 100)
	{
		printf("Invalid Monitor Variable Name Size\n");
		return -1;
	}
	if(mvSize < 0 || mvSize > 100)
	{
		printf("Invalid Monitor Variable Array Length\n");
		return -1;		
	}
	
	sprintf(sizeString,"%d",size);
	sprintf(mvLengthString,"%d",mvSize);
	
	/* Set the Packet & Mail Headers for the server instance */
	outPktHdr.to = 0;		
	outPktHdr.from = 1;
	outMailHdr.to = 1;		// Server port
	outMailHdr.from = currentThread->space->procid;		// Client port
	
	strcpy(data,"Client:CreateMV*");
	strcat(data,mvName);
	strcat(data,"|");
	strcat(data,sizeString);
	strcat(data,"|");
	strcat(data,mvLengthString);

	//printf("%d Sending %s to CreateMV\n",currentThread->space->procid,data);
	fflush(stdout);
	
	outMailHdr.length = strlen(data)+1;
	
	/* Data sent to server is Broadcast*CvId*/
	bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	fflush(stdout);
	
	if ( !success )
	{
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	
	postOffice->Receive(outMailHdr.from, &inPktHdr, &inMailHdr, ack);
	fflush(stdout);
	int index = atoi(ack);
	if(index == -1)
	{
		printf("Monitor Variable was not created\n");
	}
	else
	{
		printf("Monitor variable=>%s created by %d with id=>%d\n",mvName,currentThread->space->procid,index);
		fflush(stdout);
	}
	//printf("\nindex=>%d",index);
	//return index;
	return index;
	#endif
}
/************************** Destroy MV RPC *******************************/
void DestroyMV_Syscall(int mvid)
{
	#ifdef NETWORK
	//printf("%d inside DestroyMV Syscall\n",currentThread->space->procid);
	PacketHeader outPktHdr, inPktHdr;	// PacketHeader	=~	IP Address
	MailHeader outMailHdr, inMailHdr;	// MailHeader	=~	Port Number
	char *data = new char[50];
	char mvidString[3];
	char *ack = new char[20];

	if(mvid < 0 || mvid > 500 )
	{
		printf("%s","Cannot Destroy MV:Incorrect id\n");
		return;
	}
	
	/* Set the Packet & Mail Headers for the server instance */
	outPktHdr.to = 0;		
	outPktHdr.from = 1;
	outMailHdr.to = 1;		// Server port
	outMailHdr.from = currentThread->space->procid;		// Client port
	
	strcpy(data,"Client:DestroyMV*");
	
	sprintf(mvidString,"%d",mvid);
	strcat(data,mvidString);
	//printf("%d Sending data %s to DestroyMV\n",currentThread->space->procid,data);
	fflush(stdout);
	
	outMailHdr.length = strlen(data)+1;
	
	/* Data sent to server is DestroyLock*LockName*/
	bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	fflush(stdout);
	
	if ( !success )
	{
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	
	postOffice->Receive(outMailHdr.from, &inPktHdr, &inMailHdr, ack);	
	fflush(stdout);

	if(strcmp(ack,"Destroyed MV Copy") == 0)
	{
	//	printf("MV=>%d  copy has been destroyed by %d\n",mvid,currentThread->space->procid);
		fflush(stdout);
	}
	int index = atoi(ack);
	if(index == -1 && strcmp(ack,"Destroyed MV Copy")!= 0)
	{
		printf("Cannot Destroy:MV=>%d does not Exist\n",mvid);
		fflush(stdout);
	}
	else if(index != -1 && strcmp(ack,"Destroyed MV Copy")!=  0)
	{
	//	printf("Final MV=>%d  copy has been destroyed by %d\n",mvid,currentThread->space->procid);
		fflush(stdout);
	}
	#endif
}

/*************************** Set MV RPC **********************************/
void SetMV_Syscall(int mvid,int mvValueIndex,int mvValue)
{
	#ifdef NETWORK
	//printf("%d inside SetMV Syscall\n",currentThread->space->procid);
	PacketHeader outPktHdr, inPktHdr;	// PacketHeader	=~	IP Address
	MailHeader outMailHdr, inMailHdr;	// MailHeader	=~	Port Number
	
	char *data = new char[50];
	char mvidString[3];
	char mvValueIndexString[3];
	char mvValueString[3];
	char *ack = new char[20];

	if(mvid < 0 || mvid > 500 )
	{
		printf("%s","Cannot Set MV:Incorrect id\n");
		return;
	}
		/* Set the Packet & Mail Headers for the server instance */
	outPktHdr.to = 0;		
	outPktHdr.from = 1;
	outMailHdr.to = 1;		// Server port
	outMailHdr.from = currentThread->space->procid;		// Client port
	
	strcpy(data,"Client:SetMV*");
	
	sprintf(mvidString,"%d",mvid);
	strcat(data,mvidString);
	strcat(data,"|");
	sprintf(mvValueIndexString,"%d",mvValueIndex);
	strcat(data,mvValueIndexString);
	strcat(data,"|");
	sprintf(mvValueString,"%d",mvValue);
	strcat(data,mvValueString);
	//printf("%d Sending data %s to SetMV\n",currentThread->space->procid,data);
	fflush(stdout);
	
	outMailHdr.length = strlen(data)+1;
	
	bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	fflush(stdout);
	
	if ( !success )
	{
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	
	postOffice->Receive(outMailHdr.from, &inPktHdr, &inMailHdr, ack);	
	fflush(stdout);

	int index = atoi(ack);
	if(index != -1)
	{
	//	printf("MV has been Set successfully by %d\n",currentThread->space->procid);
	}
	else
	{
		printf("Error:MV=>%d index=>%d value=>%d could not be set\n",mvid,mvValueIndex,mvValue);
	}
	#endif
}

/*************************** Get MV RPC **********************************/
int GetMV_Syscall(int mvid,int mvValueIndex)
{
	#ifdef NETWORK
	//printf("%d inside GetMV Syscall\n",currentThread->space->procid);
	PacketHeader outPktHdr, inPktHdr;	// PacketHeader	=~	IP Address
	MailHeader outMailHdr, inMailHdr;	// MailHeader	=~	Port Number
	
	char *data = new char[50];
	char mvidString[3];
	char mvValueIndexString[3];
	char *ack = new char[20];

	if(mvid < 0 || mvid > 500 )
	{
		printf("%s","Cannot Set MV:Incorrect id\n");
		return -1;
	}
		/* Set the Packet & Mail Headers for the server instance */
	outPktHdr.to = 0;		
	outPktHdr.from = 1;
	outMailHdr.to = 1;		// Server port
	outMailHdr.from = currentThread->space->procid;		// Client port
	strcpy(data,"Client:GetMV*");
	
	sprintf(mvidString,"%d",mvid);
	strcat(data,mvidString);
	strcat(data,"|");
	sprintf(mvValueIndexString,"%d",mvValueIndex);
	strcat(data,mvValueIndexString);
	
	//printf("%d Sending data %s to GetMV\n",currentThread->space->procid,data);
	fflush(stdout);
	
	outMailHdr.length = strlen(data)+1;
	
	bool success = postOffice->Send(outPktHdr, outMailHdr, data); 
	fflush(stdout);
	
	if ( !success )
	{
		printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
	
	postOffice->Receive(outMailHdr.from, &inPktHdr, &inMailHdr, ack);	
	fflush(stdout);

	int value = atoi(ack);
	if(value != -1)
	{
	//	printf("MV=>%d has been Received successfully by %d\n",value,currentThread->space->procid);
	}
	else
	{
		printf("Error:MV value could not be fetched\n");
	}
	return value;
	#endif
	return 0;
}

/************************* Fork System Call *********************************/
void Fork_Syscall(unsigned int vaddr)
{
	int processid,threadid,j=0;
	
	Thread*t;
	t = new Thread ("Forked thread");
	
	processTableLock->Acquire();
	for (int i=0; i<MAX_PROCESSES; i++)
	{
		if (currentThread->space == processTable[i].processAddrSpace)
		{
			processid=i;
			processTable[i].totalThreads++;
			for (j=0;j<MAX_THREADS;j++)
			{
				/* Find the 1st free slot for new thread in the Thread Array */
				if(processTable[i].threadTable[j] == false)
				{
					processTable[i].threadTable[j] = true;
					threadid=j;
					t->space = currentThread->space;
					break;
				}
			}
			if(j== MAX_THREADS)
			{
				printf("\nCannot Fork Thread: Number of Threads > Limit\n");
				DEBUG('s',"\nCannot Fork Thread: Number of Threads > Limit\n");
				currentThread->Finish();
			}
			break;
		}
	}
	processTableLock->Release();
	t->Fork((VoidFunctionPtr)kernelThread,vaddr);
}

/******************* Fork system Call redirects to this function *********************/
void kernelThread(unsigned int vaddr)
{
	/* Set all registers to initial values to run user program function */
	unsigned int m;
	
	/* Load vaddr into PCReg */
	machine->WriteRegister(PCReg,vaddr);
	/* Load vaddr + 4 into NextPCReg */
	machine->WriteRegister(NextPCReg,vaddr+4);
	/* Allocate 8 contiguous pages for thread */
	m = currentThread->space->nextalloc();
	/* Make Stack Pointer point to last page */
	machine->WriteRegister(StackReg,m*PageSize-16);	
	currentThread->space->RestoreState();
	/* Now switch from kernel mode to user mode */
	machine->Run();
}


/************************** Exec System Call ****************************/
SpaceId  Exec_Syscall(unsigned int vaddr, int length)
{
	int procid = -1;
	char *filename = new char[length+1];
	
	if(copyin(vaddr,length, filename) == -1) 
	{
		//printf("\nCannot run exec:Invalid Virtual Address\n");
		DEBUG('s',"Cannot run exec:Invalid Virtual Address\n");
		return -1;
	}
	else
	{
		/* Open the file as a new process */
		OpenFile *executable = fileSystem->Open(filename);
		AddrSpace *space;

		if (executable == NULL) 
		{
			DEBUG('s',"Unable to open file %s\n", filename);
			return -1;
		}
		/* Allocate address space for that process */
     		space = new AddrSpace(executable);
     		/* Create a thread within the new process */
     		Thread *t;
		t= new Thread ("exec_thread");
		t->space= space;

		/* Update the process table */
		processTableLock->Acquire();    
		//printf("\nIn Exec:TotalProcesses=>%d",totalProcesses);
		totalProcesses++;
		for (int proc=0;proc<MAX_PROCESSES;proc++)
		{
			if(processTable[proc].processPresent == false)
			{
				processTable[proc].processPresent = true;
				processTable[proc].processAddrSpace = space;
				processTable[proc].totalThreads = 0;
	    			procid = proc;
	    			break;
			}
		}
		if(procid == MAX_PROCESSES || procid == -1)
		{
			//printf("\nCannot exec: Number of Processes > Limit\n");
			currentThread->Finish();
		}
		processTableLock->Release();													
	    
		t->Fork((VoidFunctionPtr)exec_thread,0);
		return procid;
	}
}

/****************Exec System Call redirects to exec_thread function************/
void exec_thread(int)
{
	currentThread->space->InitRegisters();		
	currentThread->space->RestoreState();		
	DEBUG('s',"Inside exec system call\n");
	machine->Run();			
}

/************************** Exit System Call ******************************/
void Exit_Syscall(int status)
{
	//currentThread->Finish();
		processTableLock->Acquire();
		//printf("\nIn Exit:TotalProcesses=>%d",totalProcesses);
		for(int i=0;i<MAX_PROCESSES;i++)
		{
		//	printf("\nThread is inside for");
			if(processTable[i].processAddrSpace == currentThread->space)
			{
			//	printf("\nIn Exit:currentThread->space=>%x",currentThread->space);
			//	printf("\nProcess=>%d",i);
				if(processTable[i].totalThreads == 0)
				{
					/*Last executing Thread in the last process */
					if(totalProcesses == 1)
					{
					//	printf("\nExiting the last thread of last process ");
						DEBUG('s'," Exiting the last thread of last process ");
						delete currentThread->space;
						processTable[i].processPresent = false;
						processTableLock->Release();
						interrupt->Halt();
					}
					/* Not the last Process */
					else
					{
					//	printf("\nExiting the last thread of a process.... deleting process addressspace");
						DEBUG('s',"Exiting the last thread of a process.... deleting process addressspace ");
						
						// Make all ipt entries of that process invalid & clear physPages used by it
					//	freePhysPageLock->Acquire();
						pageTableLock->Acquire();
						iptLock->Acquire();
						for(unsigned int j=0;j<sizeof(currentThread->space->pageTable);j++)/* Pages in page table of that process */
						{
							if(currentThread->space->pageTable[j].valid == TRUE/* PageTable entry is valid*/)
							{
								//Make ipt entry invalid
								if(ipt[currentThread->space->pageTable[j].physicalPage].use == FALSE && ipt[currentThread->space->pageTable[j].physicalPage].valid == TRUE)
									ipt[currentThread->space->pageTable[j].physicalPage].valid = FALSE;
								currentThread->space->pageTable[j].virtualPage = -1;
								bitMapObject->Clear(currentThread->space->pageTable[j].physicalPage);
								currentThread->space->pageTable[j].physicalPage = -1;
								currentThread->space->pageTable[j].valid = FALSE;
							}
						}
						iptLock->Release();
						pageTableLock->Release();
					//	freePhysPageLock->Release();
					//	delete currentThread->space;
						processTable[i].processPresent = false;
						totalProcesses--;
					}
				}
				/* Not the last thread in a process */
				else
				{
					DEBUG('s',"Finishing a thread  ");
					processTable[i].totalThreads--;
					//currentThread->Finish();
				}
			}
		}
		processTableLock->Release();
		currentThread->Finish();
}

/**************************Printf System Call *************************/
void Printf_Syscall(unsigned int add)
{
	char* mybuf;
	mybuf = new char[200];

	//read in the string at the virtual address, copy it to a buffer(mybuf),and returns the number of bytes read in addr
	int addr = copyin(add, 200, mybuf);

	if(addr == -1) //i.e a wrong address was passed
	{
		return ;
	}
	else
	{
		printf("%s",mybuf);
		return;
	}
}

/**************************Printf1 System Call *************************/
void Printf1_Syscall(unsigned int add,int arg)
{
	char* mybuf;
	mybuf = new char[200];

	int addr = copyin(add, 200, mybuf);
	if(addr == -1) //i.e a wrong address was passed
	{
		printf("Error in printf\n");
		return ;
	}
    	else
	{
		printf("%s %d",mybuf,arg);
		return;
	}
}

/**************************Printf2 System Call *************************/
void Printf2_Syscall(unsigned int arg1,unsigned int arg2)
{	
	char* mybuf1;
	char *mybuf2;
	mybuf1 = new char[50];
	mybuf2 = new char[50];

	  //read in the string at the virtual address, copy it to a buffer(mybuf),and returns the number of bytes read in a

	if((copyin(arg1, 50, mybuf1) == -1) || (copyin(arg2, 50, mybuf2) == -1)) //i.e a wrong address was passed
	{
		printf("In printf2():Error in Virtual Address\n");
		return ;
	}
	else
	{
		printf("%s %s",mybuf1,mybuf2);
		return;
	}
}

/**************************Scanf System Call *************************/
int Scanf_Syscall(void)
{
	int input;
	scanf("%d",&input);
	return input;
}

/******************** Handle Memory Full -> Step 4 ********************/

int handleMemoryFull(int vpn)
{
	int pageToBeEvicted = -1;
//	printf("\nInside handleMemoryFull");

	if(evictPolicy == 1)	// Select a Random Page to Evict
	{
		iptLock->Acquire();
		do
		{
			pageToBeEvicted = rand()%NumPhysPages;
		}while(ipt[pageToBeEvicted].use == TRUE);
			ipt[pageToBeEvicted].use = TRUE;
		iptLock->Release();
	}
//	printf("\npageToBeEvicted=>%d",pageToBeEvicted);
	
	//If pageToBeEvicted has been modified, it must be saved in swapFile
	IntStatus oldLevel = interrupt->SetLevel(IntOff);	// Disable interrupts
	for(int k=0;k<TLBSize;k++)
	{
		if(machine->tlb[k].physicalPage == pageToBeEvicted && machine->tlb[k].valid == TRUE)
		{
			ipt[machine->tlb[k].physicalPage].dirty = machine->tlb[k].dirty;
			machine->tlb[k].valid = FALSE;
		}
	}	
	(void) interrupt->SetLevel(oldLevel);	//Enable Interrupts 

		if( ipt[pageToBeEvicted].valid && ipt[pageToBeEvicted].dirty)
		{
			int freePageSwapFile = -1;
			swapFileLock->Acquire();
				freePageSwapFile = swapBitMapObject->Find();
			swapFileLock->Release();
			if(freePageSwapFile == -1)
			{
				printf("\nSwap File Full");
				interrupt -> Halt();
			}

			swapFile-> WriteAt (&(machine->mainMemory[pageToBeEvicted*PageSize]),PageSize, freePageSwapFile*PageSize);
				
			pageTableLock->Acquire();
			ipt[pageToBeEvicted].addrSpacePtr->pageTable[ipt[pageToBeEvicted].virtualPage].location = 2;	//Present in Swap File
			ipt[pageToBeEvicted].addrSpacePtr->pageTable[ipt[pageToBeEvicted].virtualPage].byteOffset = freePageSwapFile*PageSize;
			ipt[pageToBeEvicted].addrSpacePtr->pageTable[ipt[pageToBeEvicted].virtualPage].valid = FALSE;
			pageTableLock->Release();
		}
	else
	{	
		pageTableLock->Acquire();
		if(ipt[pageToBeEvicted].addrSpacePtr->pageTable[ipt[pageToBeEvicted].virtualPage].valid = TRUE)
			ipt[pageToBeEvicted].addrSpacePtr->pageTable[ipt[pageToBeEvicted].virtualPage].valid = FALSE;
		pageTableLock->Release();
	}
	
	return pageToBeEvicted;
}

/************************** Handle IPT Miss *************************/
int handleIPTMiss(int vpnLocal)
{
	
	//Allocate a page of memory
	//printf("\n\nVirtual Page that caused page Fault=>%d",vpn);
	int freePhysPage = -1;
	
	freePhysPageLock->Acquire();
		freePhysPage = bitMapObject->Find();	
	freePhysPageLock->Release();
	
	if(freePhysPage == -1)
	{
		freePhysPage = handleMemoryFull(vpnLocal);
	}
	
	if(currentThread->space->pageTable[vpnLocal].location == 0)	//Virtual page is in executable
	{
		currentThread->space->executablePtr->ReadAt(&(machine->mainMemory[(freePhysPage)*(PageSize)]),PageSize,currentThread->space->pageTable[vpnLocal].byteOffset) ;
	}
	if(currentThread->space->pageTable[vpnLocal].location == 2)	//Virtual page is in swapFile
	{
		swapFile->ReadAt(&(machine->mainMemory[(freePhysPage)*(PageSize)]),PageSize, currentThread->space->pageTable[vpnLocal].byteOffset) ;
	}	
	pageTableLock->Acquire();
	currentThread->space->pageTable[vpnLocal].physicalPage = freePhysPage;
	currentThread->space->pageTable[vpnLocal].valid = TRUE;
	pageTableLock->Release();
		
	ipt[freePhysPage].processId = currentThread->space->procid;
	ipt[freePhysPage].addrSpacePtr = currentThread->space;
	ipt[freePhysPage].physicalPage = freePhysPage;
	ipt[freePhysPage].virtualPage = vpnLocal;
	ipt[freePhysPage].valid = TRUE;
	ipt[freePhysPage].dirty = FALSE;
	ipt[freePhysPage].readOnly = FALSE;
	
	return freePhysPage;
}

void ExceptionHandler(ExceptionType which) 
{
	int type = machine->ReadRegister(2);
	int rv=0; 	

	if ( which == SyscallException ) 
	{
		switch (type) 
		{
			default:
				DEBUG('a', "Unknown syscall - shutting down.\n");
				
			case SC_Halt:
				DEBUG('a', "Shutdown, initiated by user program.\n");
				interrupt->Halt();
				break;

			case SC_Exec:
				DEBUG('a', "Exec syscall.\n");
				rv= Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
				break;
		
			case SC_Fork:
				DEBUG('a', "Fork system call. \n");
				Fork_Syscall(machine->ReadRegister(4));
				break;

			case SC_Create:
				DEBUG('a', "Create syscall.\n");
				Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
				break;
			
			case SC_Open:
				DEBUG('a', "Open syscall.\n");
				rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
				break;

			case SC_Write:
				DEBUG('a', "Write syscall.\n");
				Write_Syscall(machine->ReadRegister(4),machine->ReadRegister(5),machine->ReadRegister(6));
				break;

			case SC_Read:
				DEBUG('a', "Read syscall.\n");
				rv = Read_Syscall(machine->ReadRegister(4), machine->ReadRegister(5),machine->ReadRegister(6));
				break;

			case SC_Close:
				DEBUG('a', "Close syscall.\n");
				Close_Syscall(machine->ReadRegister(4));
				break;

			case SC_Yield:
				DEBUG('a', "Yield syscall.\n");
				Yield_Syscall();
				break;

			case SC_CreateLock:
				DEBUG('a', "CreateLock syscall.\n");
				rv=CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
				break;

			case SC_Acquire:
				DEBUG('a', "Acquire system call. \n");
				Acquire_Syscall(machine->ReadRegister(4));
				break;

			case SC_Release:
				DEBUG('a', "Release system call. \n");
				Release_Syscall(machine->ReadRegister(4));
				break;

			case SC_DestroyLock:
				DEBUG('a', "DestroyLock system call. \n");
				DestroyLock_Syscall(machine->ReadRegister(4));
				break;

			case SC_CreateCV:
				DEBUG('a', "CreateCondition system call.\n");
				rv = CreateCV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
				break;

			case SC_DestroyCV:
				DEBUG('a', "DestroyCondition system call. \n");
				DestroyCV_Syscall(machine->ReadRegister(4));
				break;

			case SC_Signal:
				DEBUG('a', "Signal system call. \n");
				Signal_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
				break;

			case SC_Wait:
				DEBUG('a', "Wait system call. \n");
				Wait_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
				break;

			case SC_Broadcast:
				DEBUG('a', "Wait system call. \n");
				Broadcast_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
				break;
				
			/*************** Project 3 RPC Calls for Monitor Varibles **************/
			case SC_CreateMV:
				DEBUG('a', "CreateMV system call. \n");
				rv = CreateMV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5),machine->ReadRegister(6));
				break;
				
			case SC_DestroyMV:
				DEBUG('a', "DestroyMV system call. \n");
				DestroyMV_Syscall(machine->ReadRegister(4));
				break;
				
			case SC_SetMV:
				DEBUG('a', "SetMV system call. \n");
				SetMV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5),machine->ReadRegister(6));
				break;

			case SC_GetMV:
				DEBUG('a', "GetMV system call. \n");
				rv = GetMV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
				break;	
		
			/****************************************************************/
			case SC_Exit:
		//		printf("\nExit called=>%d",currentThread->space->procid);
				DEBUG('a', "exit system call. \n");
				Exit_Syscall(machine->ReadRegister(4));
				break;
	    
			case SC_Printf:
				DEBUG('a', "printf system call for strings. \n");
				Printf_Syscall(machine->ReadRegister(4));
				break;
	    
			case SC_Scanf:
				DEBUG('a', "scanf() system call. \n");
				rv=Scanf_Syscall();
				break;
	    
			case SC_Printf1:
				DEBUG('a', "printf system call for integer args. \n");
				Printf1_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
				break;

			case SC_Printf2:
				DEBUG('a', "printf system call for char array type args. \n");
				Printf2_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
		}

		// Put in the return value and increment the PC
		machine->WriteRegister(2,rv);
		machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
		machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
		machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
		return;
	}
	else if(which == PageFaultException)
	{
		//cout << "Page fault generated"<<endl;
		int vpn = (machine->ReadRegister(39))/PageSize;
		if(vpn > PageSize*NumPhysPages)
		{
			printf("Register 39=>%d",machine->ReadRegister(39));
			printf("\nVPN=>%d is Out of Bound",vpn);
			interrupt->Halt();
		}

		int freePhysPage = -1;
		// Search the ipt		
		iptLock->Acquire();
		for(int i=0;i<NumPhysPages;i++)
		{
			if(ipt[i].virtualPage == vpn && ipt[i].valid == TRUE && currentThread->space->procid == ipt[i].processId)
			{
				if(ipt[i].use == FALSE)
				{
					freePhysPage = i;
					ipt[i].use = TRUE;
					break;
				}
			}
		}
		iptLock->Release();
		
		if(freePhysPage == -1)
		{
			freePhysPage = handleIPTMiss(vpn);
	//		printf("\nFreePhysPage to be loaded into TLB=>%d",freePhysPage);
		}
		//Populate TLB from ipt
		IntStatus oldLevel = interrupt->SetLevel(IntOff);	// Disable interrupts
		
		if(machine-> tlb[currentTLBEntry].valid)
		{
			if(machine-> tlb[currentTLBEntry].dirty)
				ipt[machine->tlb[currentTLBEntry].physicalPage].dirty = TRUE;
		}
		machine->tlb[currentTLBEntry].physicalPage = freePhysPage;		 
		machine->tlb[currentTLBEntry].virtualPage = ipt[freePhysPage].virtualPage;		 
		machine->tlb[currentTLBEntry].valid = ipt[freePhysPage].valid;
		machine->tlb[currentTLBEntry].use = ipt[freePhysPage].use;
		machine->tlb[currentTLBEntry].dirty = ipt[freePhysPage].dirty;
		machine->tlb[currentTLBEntry].readOnly = ipt[freePhysPage].readOnly;
		
		currentTLBEntry = (currentTLBEntry + 1)%TLBSize;					
/*		machine->tlb[currentTLBEntry].virtualPage = currentThread->space->pageTable[vpn].virtualPage;
		machine->tlb[currentTLBEntry].physicalPage = currentThread->space->pageTable[vpn].physicalPage;		 
		machine->tlb[currentTLBEntry].valid = currentThread->space->pageTable[vpn].valid;
		machine->tlb[currentTLBEntry].use = currentThread->space->pageTable[vpn].use;
		machine->tlb[currentTLBEntry].dirty = currentThread->space->pageTable[vpn].dirty;
		machine->tlb[currentTLBEntry].readOnly = currentThread->space->pageTable[vpn].readOnly;
		currentTLBEntry = (currentTLBEntry + 1)%TLBSize;		*/
		
		(void) interrupt->SetLevel(oldLevel);	//Enable Interrupts 

		ipt[freePhysPage].use = FALSE;
		
		return;
	}
	else
	{
		cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
		interrupt->Halt();
	}
}
