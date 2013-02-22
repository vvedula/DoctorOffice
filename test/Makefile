# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = /auto/home-scf-22/csci402/testgnu/decstation-ultrix/bin/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort test1 test2 test3 test4 test5 test6i test6ii test7 test8 test9 test10 test11 test12 test13 project4Tests project4Test1  project4Test2 project4Test3 doctoroffice maintest forkmatmult forksort wrn patient

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort
	
matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult
	
doctoroffice.o: doctoroffice.c
	$(CC) $(CFLAGS) -c doctoroffice.c
doctoroffice:doctoroffice.o start.o
	$(LD) $(LDFLAGS) start.o doctoroffice.o -o doctoroffice.coff
	../bin/coff2noff doctoroffice.coff doctoroffice	

test1.o: test1.c
	$(CC) $(CFLAGS) -c test1.c
test1: test1.o start.o
	$(LD) $(LDFLAGS) start.o test1.o -o test1.coff
	../bin/coff2noff test1.coff test1
	
	
test2.o: test2.c
	$(CC) $(CFLAGS) -c test2.c
test2: test2.o start.o
	$(LD) $(LDFLAGS) start.o test2.o -o test2.coff
	../bin/coff2noff test2.coff test2
	
test3.o: test3.c
	$(CC) $(CFLAGS) -c test3.c
test3: test3.o start.o
	$(LD) $(LDFLAGS) start.o test3.o -o test3.coff
	../bin/coff2noff test3.coff test3	
	
test4.o: test4.c
	$(CC) $(CFLAGS) -c test4.c
test4: test4.o start.o
	$(LD) $(LDFLAGS) start.o test4.o -o test4.coff
	../bin/coff2noff test4.coff test4	

test5.o: test5.c
	$(CC) $(CFLAGS) -c test5.c
test5: test5.o start.o
	$(LD) $(LDFLAGS) start.o test5.o -o test5.coff
	../bin/coff2noff test5.coff test5

test6i.o: test6i.c
	$(CC) $(CFLAGS) -c test6i.c
test6i: test6i.o start.o
	$(LD) $(LDFLAGS) start.o test6i.o -o test6i.coff
	../bin/coff2noff test6i.coff test6i		

	
test6ii.o: test6ii.c
	$(CC) $(CFLAGS) -c test6ii.c
test6ii: test6ii.o start.o
	$(LD) $(LDFLAGS) start.o test6ii.o -o test6ii.coff
	../bin/coff2noff test6ii.coff test6ii

test7.o: test7.c
	$(CC) $(CFLAGS) -c test7.c
test7: test7.o start.o
	$(LD) $(LDFLAGS) start.o test7.o -o test7.coff
	../bin/coff2noff test7.coff test7

test8.o: test8.c
	$(CC) $(CFLAGS) -c test8.c
test8: test8.o start.o
	$(LD) $(LDFLAGS) start.o test8.o -o test8.coff
	../bin/coff2noff test8.coff test8

test9.o: test9.c
	$(CC) $(CFLAGS) -c test9.c
test9: test9.o start.o
	$(LD) $(LDFLAGS) start.o test9.o -o test9.coff
	../bin/coff2noff test9.coff test9

test10.o: test10.c
	$(CC) $(CFLAGS) -c test10.c
test10: test10.o start.o
	$(LD) $(LDFLAGS) start.o test10.o -o test10.coff
	../bin/coff2noff test10.coff test10

test11.o: test11.c
	$(CC) $(CFLAGS) -c test11.c
test11: test11.o start.o
	$(LD) $(LDFLAGS) start.o test11.o -o test11.coff
	../bin/coff2noff test11.coff test11

test12.o: test12.c
	$(CC) $(CFLAGS) -c test12.c
test12: test12.o start.o
	$(LD) $(LDFLAGS) start.o test12.o -o test12.coff
	../bin/coff2noff test12.coff test12

test13.o: test13.c
	$(CC) $(CFLAGS) -c test13.c
test13: test13.o start.o
	$(LD) $(LDFLAGS) start.o test13.o -o test13.coff
	../bin/coff2noff test13.coff test13

project4Tests.o: project4Tests.c
	$(CC) $(CFLAGS) -c project4Tests.c
project4Tests: project4Tests.o start.o
	$(LD) $(LDFLAGS) start.o project4Tests.o -o project4Tests.coff
	../bin/coff2noff project4Tests.coff project4Tests

project4Test1.o: project4Test1.c
	$(CC) $(CFLAGS) -c project4Test1.c
project4Test1: project4Test1.o start.o
	$(LD) $(LDFLAGS) start.o project4Test1.o -o project4Test1.coff
	../bin/coff2noff project4Test1.coff project4Test1

project4Test2.o: project4Test2.c
	$(CC) $(CFLAGS) -c project4Test2.c
project4Test2: project4Test2.o start.o
	$(LD) $(LDFLAGS) start.o project4Test2.o -o project4Test2.coff
	../bin/coff2noff project4Test2.coff project4Test2
				
project4Test3.o: project4Test3.c
	$(CC) $(CFLAGS) -c project4Test3.c
project4Test3: project4Test3.o start.o
	$(LD) $(LDFLAGS) start.o project4Test3.o -o project4Test3.coff
	../bin/coff2noff project4Test3.coff project4Test3
						
maintest.o: maintest.c
	$(CC) $(CFLAGS) -c maintest.c
maintest: maintest.o start.o
	$(LD) $(LDFLAGS) start.o maintest.o -o maintest.coff
	../bin/coff2noff maintest.coff maintest

forkmatmult.o: forkmatmult.c
	$(CC) $(CFLAGS) -c forkmatmult.c
forkmatmult: forkmatmult.o start.o
	$(LD) $(LDFLAGS) start.o forkmatmult.o -o forkmatmult.coff
	../bin/coff2noff forkmatmult.coff forkmatmult

forksort.o: forksort.c
	$(CC) $(CFLAGS) -c forksort.c
forksort: forksort.o start.o
	$(LD) $(LDFLAGS) start.o forksort.o -o forksort.coff
	../bin/coff2noff forksort.coff forksort

wrn.o: wrn.c
	$(CC) $(CFLAGS) -c wrn.c
wrn: wrn.o start.o
	$(LD) $(LDFLAGS) start.o wrn.o -o wrn.coff
	../bin/coff2noff wrn.coff wrn

patient.o: patient.c
	$(CC) $(CFLAGS) -c patient.c
patient: patient.o start.o
	$(LD) $(LDFLAGS) start.o patient.o -o patient.coff
	../bin/coff2noff patient.coff patient

clean:
	rm -f *.o *.coff
