// addrspace.h
//	Data structures to keep track of executing user programs
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "debug.h"

#define UserStackSize 1024 // increase this as necessary!
//#define NumPhysPages 128
class AddrSpace
{
public:
  AddrSpace(){
    DEBUG(dbgSys, "----CREATE ADDRSPACE");
    PhyPageStatus = new bool[128];
    NumFreePages=128;
  };            // Create an address space.
  ~AddrSpace(); // De-allocate an address space

  bool Load(char *fileName); // Load a program into addr space from
                             // a file
                             // return false if not found

  //void Execute();             	// Run a program
  // assumes the program has already
  // been loaded
  void Execute();

  void SaveState();    // Save/restore address space-specific
  void RestoreState(); // info on a context switch

  bool *getPhyPageStatus()
  {
    return PhyPageStatus;
  }

  int getNumFreePages()
  {
    return NumFreePages;
  }

  void setNumFreePages(int n){
    NumFreePages = n;
  }

  void decreaseNumFreePages()
  {
    DEBUG(dbgSys, "dec:"<< NumFreePages - 1);
    setNumFreePages(NumFreePages - 1);
    DEBUG(dbgSys, "aft:"<< NumFreePages);
    DEBUG(dbgSys,"AFT:"<<getNumFreePages());
  }
  void InitRegisters(); // Initialize user-level CPU registers,
  // Translate virtual address _vaddr_
  // to physical address _paddr_. _mode_
  // is 0 for Read, 1 for Write.
  ExceptionType Translate(unsigned int vaddr, unsigned int *paddr, int mode);

private:
  TranslationEntry *pageTable; // Assume linear page table translation
                               // for now!
  unsigned int numPages;       // Number of pages in the virtual
                               // address space


  bool* PhyPageStatus;
  int NumFreePages;                 // before jumping to user code
};

#endif // ADDRSPACE_H
