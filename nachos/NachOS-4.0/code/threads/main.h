// main.h 
//	This file defines the Nachos global variables
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef MAIN_H
#define MAIN_H

#define MAX_CHILD 8
#define MAX_THREAD 8

#include "copyright.h"
#include "debug.h"
#include "kernel.h"

extern Kernel *kernel;
extern Debug *debug;
extern bool exitThreadArray[MAX_CHILD];
extern unsigned thread_index;
extern Thread *threadToBeDestroyed; 
extern Thread *threadArray[MAX_CHILD];


#endif // MAIN_H

