#include "pcb.h"
#include "utility.h"
#include "thread.h"
#include "addrspace.h"

extern void StartProcess(int arg);
PCB::PCB(int id)
{
	if (id == 0)
		this->parentID = -1;
	else
		this->parentID = kernel->currentThread->threadId;

	this->numwait = this->exitcode = this->boolBG = 0;
	this->thread = NULL;

	this->joinsem = new Semaphore("joinsem", 0);
	this->exitsem = new Semaphore("exitsem", 0);
	this->multex = new Semaphore("multex", 1);
}
PCB::~PCB()
{
	if (joinsem != NULL)
		delete this->joinsem;
	if (exitsem != NULL)
		delete this->exitsem;
	if (multex != NULL)
		delete this->multex;
	if (thread != NULL)
	{
		thread->FreeSpace();
		thread->Finish();
	}
}
int PCB::GetID() { return this->thread->threadId; }
int PCB::GetNumWait() { return this->numwait; }
int PCB::GetExitCode() { return this->exitcode; }

void PCB::SetExitCode(int ec) { this->exitcode = ec; }

// Process tranlation to block
// Wait for JoinRelease to continue exec
void PCB::JoinWait()
{
	joinsem->P();
}

void PCB::JoinRelease()
{
	joinsem->V();
}

void PCB::ExitWait()
{
	exitsem->P();
}

// Release wating process
void PCB::ExitRelease()
{
	exitsem->V();
}

void PCB::IncNumWait()
{
	multex->P();
	++numwait;
	multex->V();
}

void PCB::DecNumWait()
{
	multex->P();
	if (numwait > 0)
		--numwait;
	multex->V();
}

void PCB::SetFileName(char *fn) { strcpy(FileName, fn); }
char *PCB::GetFileName() { return this->FileName; }

int PCB::Exec(char *filename, int id)
{
	multex->P();

	this->thread = new Thread(filename);

	if (this->thread == NULL)
	{
		printf("\nPCB::Exec:: Not enough memory..!\n");
		multex->V();
		return -1;
	}

	this->thread->threadId = id;

	this->parentID = kernel->currentThread->threadId;

	this->thread->Fork((VoidFunctionPtr)&StartProcess, (void *)id);

	multex->V();
	return id;
}