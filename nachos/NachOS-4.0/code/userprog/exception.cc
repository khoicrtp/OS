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
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "synchconsole.h"
#include "syscall.h"
#include "ksyscall.h"
#include "thread.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------
void increasePC()
{
	DEBUG(dbgSys, "----INCREASE PC");
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
	// Adjust Program Counter
}
char *User2System(int virtAddr, int limit)
{
	int i;
	int oneChar;
	char *kernelBuf = NULL;

	kernelBuf = new char[limit + 1];
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf, 0, limit + 1);

	for (i = 0; i < limit; i++)
	{
		kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

int System2User(int virtAddr, int len, char *buffer)
{
	if (len < 0)
		return -1;
	if (len == 0)
		return len;
	int i = 0;
	int oneChar = 0;
	do
	{
		oneChar = (int)buffer[i];
		kernel->machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

void StartProcess(char *filename)
{
	DEBUG(dbgSys,"STARTING PROCESS");
	OpenFile *executable = kernel->fileSystem->Open(filename);
	AddrSpace *space;

	if (executable == NULL)
	{
		DEBUG(dbgSys, "Unable to open file \n");
		printf("Unable to open file %s\n", filename);
		return;
	}
	space = new AddrSpace();
	kernel->currentThread->space = space;

	delete executable; // close file

	space->InitRegisters(); // set the initial register values
	space->RestoreState();	// load page table register

	kernel->machine->Run(); // jump to the user progam
	ASSERT(FALSE);			// machine->Run never returns;
							// the address space exits
							// by doing the syscall "exit"
}

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);
	char c;
	int vaddr, memval; //value address, memory value
	int buffer;
	int length;
	char *buf;
	int pid = 0;
	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case NoException:
		return;
	case PageFaultException:
	{
		DEBUG(dbgSys, "\n No valid trasnition found");
		SysHalt();
		ASSERTNOTREACHED();
		break;
	}
	case ReadOnlyException:
	{
		DEBUG(dbgSys, "\n Write attempted to page marked read-only");
		SysHalt();
		ASSERTNOTREACHED();
		break;
	}
	case BusErrorException:
	{
		DEBUG(dbgSys, "\n Translation resulted invalid physical address");
		SysHalt();
		ASSERTNOTREACHED();
		break;
	}
	case AddressErrorException:
	{
		DEBUG(dbgSys, "\n Unaligned reference or one that was beyond the end of the address space");
		SysHalt();
		ASSERTNOTREACHED();
		break;
	}
	case OverflowException:
	{
		DEBUG(dbgSys, "\n Integer overflow in add or sub.");
		SysHalt();
		ASSERTNOTREACHED();
		break;
	}
	case IllegalInstrException:
	{
		DEBUG(dbgSys, "\n Unimplemented or reserved instr.");
		SysHalt();
		ASSERTNOTREACHED();
		break;
	}
	case NumExceptionTypes:
	{
		DEBUG(dbgSys, "\n Number exception types");
		SysHalt();
		ASSERTNOTREACHED();
		break;
	}

	case SyscallException:
		switch (type)
		{
		case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

			SysHalt();

			ASSERTNOTREACHED();
			break;

		case SC_Add:
			DEBUG(dbgSys, "Adding... " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							/* int op2 */ (int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			/* Modify return point */
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}

			return;
			ASSERTNOTREACHED();
			break;

		case SC_ReadNum: //Read Interger
		{
			char num_string[11] = {0}; // max value and min value of C have 11 numbers
			long long l = 0;
			char c;
			int flag_overflow = 0;
			for (int i = 0; i < 12; i++)
			{
				c = kernel->synchConsoleIn->GetChar();
				if (i == 11 && c >= '0' && c <= '9') // Check if input are larger than 11 numbers
				{
					cerr << "Integer overflow";
					flag_overflow = 1;
					break;
				}
				if (c >= '0' && c <= '9') //Check if input are character or int
					num_string[i] = c;
				else if (i == 0 && c == '-') // Check to head of char
					num_string[i] = c;
				else
					break;
			}

			int i = (num_string[0] == '-') ? 1 : 0;
			while (i < 11 && num_string[i] >= '0' && num_string[i] <= '9')
				l = l * 10 + num_string[i++] - '0';
			l = (num_string[0] == '-') ? (-l) : l;
			if (flag_overflow == 1)
				l = 0;
			kernel->machine->WriteRegister(2, (int)l);
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}
			return;
			ASSERTNOTREACHED();
			break;
		}
		case SC_PrintNum: //Print Interger on screen
		{
			int n = kernel->machine->ReadRegister(4);

			char num_string[11] = {0}; // max value and min value of C have 11 numbers
			int tmp[11] = {0}, i = 0, j = 0;

			if (n < 0) //Check if N is negative
			{
				n = -n;
				num_string[i++] = '-';
			}

			do
			{
				tmp[j++] = n % 10; //Take rightmost number
				n /= 10;
			} while (n);
			while (j)
				num_string[i++] = '0' + (char)tmp[--j];
			for (int z = 0; z < i; z++)
				kernel->synchConsoleOut->PutChar(num_string[z]);

			kernel->machine->WriteRegister(2, 0);

			/* Modify return point */
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}

			return;

			ASSERTNOTREACHED();

			break;
		}
		case SC_RandomNum: //Randomize an interger number
		{
			RandomInit(time(0));	// from sysdep.cc nachos
			int n = RandomNumber(); //from sysdep.cc nachos
			DEBUG(dbgSys, n);
			kernel->machine->WriteRegister(2, int(n));

			/* Modify return point */
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}

			return;

			ASSERTNOTREACHED();

			break;
		}
		case SC_ReadChar: //Read a character

			c = kernel->synchConsoleIn->GetChar();	   //Read char (input)
			kernel->machine->WriteRegister(2, (int)c); //Write to register 2

			/* set previous programm counter (debugging only)*/
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			/* set next programm counter for brach execution */
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);

			return;
			ASSERTNOTREACHED();
			break;

		case SC_PrintChar: //Print a character

			c = (char)kernel->machine->ReadRegister(4); //Read from memory r4 to get value
			kernel->synchConsoleOut->PutChar(c);		//Write char

			/* set previous programm counter (debugging only)*/
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			/* set next programm counter for brach execution */
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);

			return;
			ASSERTNOTREACHED();
			break;

		case SC_ReadString: //Read char* with length
			// initialize variables
			int i;
			buffer = kernel->machine->ReadRegister(4); //Get the value from register r4
			length = kernel->machine->ReadRegister(5); //Get the value from register r5
			buf = NULL;

			if (length > 0) //Input string
			{
				buf = new char[length];
				for (i = 0; i < length - 1; i++)
				{
					c = kernel->synchConsoleIn->GetChar();
					if (c == '\n') //If endline means the end
						break;
					else
						buf[i] = c; //read each character to the array of char
				}
			}

			if (buf != NULL) //If string != NULL
			{
				int n = strlen(buf) + 1;
				for (int i = 0; i < n; i++)
				{
					kernel->machine->WriteMem(buffer + i, 1, (int)buf[i]); //Write to memory
				}
				delete[] buf; //release buf
			}
			//kernel->machine->WriteRegister(2, 0); //Write result to register 2

			/* set previous programm counter (debugging only)*/
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			/* set next programm counter for brach execution */
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);

			return;
			ASSERTNOTREACHED();
			break;

		case SC_PrintString: //Print char*
		{
			vaddr = kernel->machine->ReadRegister(4);
			kernel->machine->ReadMem(vaddr, 1, &memval); //read memory to get value address
			i = 0;
			while ((*(char *)&memval) != '\0') //While not end of string (\0)
			{
				DEBUG(dbgSys, "----------Printing string[i]:" << i);
				kernel->synchConsoleOut->PutChar(*(char *)&memval); //Write each char
				vaddr++;
				kernel->machine->ReadMem(vaddr, 1, &memval); //Read each char from memory to write
			}
			/* set previous programm counter (debugging only)*/
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			/* set next programm counter for brach execution */
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);

			return;
			ASSERTNOTREACHED();
			break;
			// DEBUG(dbgSys, "------Printing string\n");
			// int virtAddr;
			// char *buffer;
			// virtAddr = kernel->machine->ReadRegister(4); // Read string from register 4
			// buffer = User2System(virtAddr, 512);		 // Copy string from User space to System space

			// int i = 0;
			// while (buffer[i] != '\0')
			// {
			// 	DEBUG(dbgSys, "----------Printing string[i]:"<<i);
			// 	kernel->synchConsoleOut->PutChar(buffer[i]); // Print string to console
			// 	i++;
			// }
			// kernel->synchConsoleOut->PutChar('\n'); // Line break
			// DEBUG(dbgSys, "----------Finish Printing string");
			// delete buffer;
			// increasePC(); // Adjust program counter

			// return;
			// ASSERTNOTREACHED();
			// break;
		}

		case SC_Exec:
		// {
		// 	// Copy the executable name into kernel space
		// 	vaddr = kernel->machine->ReadRegister(4);
		// 	kernel->machine->ReadMem(vaddr, 1, &memval);
		// 	char *buffer;
		// 	int i = 0;
		// 	while ((*(char *)&memval) != '\0')
		// 	{
		// 		buffer[i] = (*(char *)&memval);
		// 		i++;
		// 		vaddr++;
		// 		kernel->machine->ReadMem(vaddr, 1, &memval);
		// 	}
		// 	buffer[i] = (*(char *)&memval);
		// 	DEBUG(dbgSys,"FILENAME: " << buffer);
		// 	StartProcess(buffer);
		// }
			{
				DEBUG(dbgSys, "EXEC:" << kernel->machine->ReadRegister(4) << "\n");
				//int virtAddr;
				//virtAddr = kernel->machine->ReadRegister(4);
				//VoidFunctionPtr func = (VoidFunctionPtr) kernel->machine->ReadRegister(4);
				int val = kernel->machine->ReadRegister(4);

				char *filename = &(kernel->machine->mainMemory[val]);

				DEBUG(dbgSys, "FILENAME:" << filename << "\n");
				char buf[255];
				bzero(buf, 255);
				sprintf(buf, "p%d", pid);
				Thread *newThread;
				newThread = new Thread(buf);
				newThread->pid = pid++;

				newThread->space = kernel->currentThread->space;
				newThread->SaveUserState();
				kernel->currentThread = newThread;

				//kernel->currentThread->SelfTest();
				DEBUG(dbgSys, "FORKING THREAD:" << "\n");
				kernel->currentThread->ForkThreadWithFilename(filename);
				kernel->currentThread->Yield();
				DEBUG(dbgSys, "THREAD FORKED:" << "\n");

				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
				break;
			}
		case SC_Exit:
		{
			{
				int exitcode;
				exitcode = kernel->machine->ReadRegister(4);
				printf("[pid %d]: Exit called. Code: %d\n", kernel->currentThread->GetPID(), exitcode);
				// We do not wait for the children to finish.
				// The children will continue to run.
				// We will worry about this when and if we implement signals.
				exitThreadArray[kernel->currentThread->GetPID()] = true;

				// Find out if all threads have called exit
				for (i = 0; i < thread_index; i++)
				{
					if (!exitThreadArray[i])
						break;
				}
				kernel->currentThread->Exit(i == thread_index, exitcode);
			}
		}
		default:
			cerr << "Unexpected system call " << type << "\n";
			break;
		}
		break;
	default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		break;
	}
	return;
	ASSERTNOTREACHED();
}
