// system.h reference
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "synch.h"

#define MAX_PROCESSES 100
#define MAX_THREADS 100

/* Initialization and cleanup routines */
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock


#ifdef USER_PROGRAM
#include "machine.h"
#include "bitmap.h"
extern Machine* machine;	// user program memory and registers
extern BitMap *bitMapObject;			// one bit map object for the entire OS
extern BitMap *swapBitMapObject;

extern Lock *processTableLock;
extern Lock *memoryLock;
extern Lock *swapFileLock;
extern Lock *pageTableLock;

/* Kernel Lock Structure*/
struct kernelLocks
{
	bool exists;
	Lock* lock;
	AddrSpace *addrSpace;
	bool toBeDestroyed;
	int count;
};
extern kernelLocks kernelLock[100];

/* CV Structure*/
struct CVsList
{
	bool exists;
	bool toBeDestroyed;
	Condition* condition;
	AddrSpace *addrSpace;
	int count;
};
extern CVsList CVList[100];

/* Locks to manage locks & CVs among threads*/
extern Lock *osLock;
extern Lock *cvLock;

/* Total Locks & CVs */
extern int totalLocks;
extern int totalCVs;
extern Lock *vpnLock;
extern Lock *iptLock;
extern Lock *freePhysPageLock;

extern int totalProcesses;

/* Structure for Process Table */
struct ProcessTable	
{ 
	bool processPresent;
	int totalThreads;
	AddrSpace *processAddrSpace;
	bool threadTable[MAX_THREADS];
};

extern ProcessTable processTable[MAX_PROCESSES];
		
extern int currentTLBEntry;
extern int globalProcessid;
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
extern int numServers;
extern int serverMachineId;
#endif

#include "translate.h"
class newTranslationEntry : public TranslationEntry
{
	public:
		int processId;
		AddrSpace *addrSpacePtr;
};
extern newTranslationEntry *ipt;

extern int evictPolicy;
extern int swapLocation;
extern OpenFile *swapFile;


#endif // SYSTEM_H
