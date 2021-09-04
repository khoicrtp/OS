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
#include "ptable.h"

#define MAX_BUFFER 255
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

void StartProcess(int id)
{
	kernel->currentThread->space->Execute(id);
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

void increasePC()
{
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
}

int ctoi(char c)
{
	if (c == '0')
	{
		return 0;
	}
	else if (c == '1')
	{
		return 1;
	}
	else if (c == '2')
	{
		return 2;
	}
	else if (c == '3')
	{
		return 3;
	}
	else if (c == '4')
	{
		return 4;
	}
	else if (c == '5')
	{
		return 5;
	}
	else if (c == '6')
	{
		return 6;
	}
	else if (c == '7')
	{
		return 7;
	}
	else if (c == '8')
	{
		return 8;
	}
	else if (c == '9')
	{
		return 9;
	}
	else
		return -1;
}

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);
	char c;
	int vaddr, memval; //value address, memory value
	int buffer;
	int length;
	char *buf;
	int number;
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
		case SC_Exec:		
			char *filename;
			int ch;
			int i;
			i = 0;
			int buffadd;
			int id;
			filename = new char[256];
			
			buffadd = kernel->machine->ReadRegister(4); /* only one argument, so that’s in R4 */

			if (!kernel->machine->ReadMem(buffadd, 1, &ch))
				return;

			while (ch != 0)
			{
				filename[i] = (char)ch;
				buffadd += 1;
				i++;
				if (!kernel->machine->ReadMem(buffadd, 1, &ch))
					return;
			}

			filename[i] = (char)0;

			while ((*(char *)&memval) != '\0') //While not end of string (\0)
			{
				filename[i] = *(char *)&memval;
				vaddr++;
				kernel->machine->ReadMem(vaddr, 1, &memval); //Read each char from memory to write
			}

			extern PTable *pTab;

			id = pTab->ExecUpdate(filename);

			increasePC();

			return;
			ASSERTNOTREACHED();
			break;
		
		case SC_Join:
		{
			int id; // dia chi cua thread
			id = kernel->machine->ReadRegister(4);

			int res = pTab->JoinUpdate(id);
			increasePC();

			return;
			ASSERTNOTREACHED();
			break;
		}
		case SC_Exit:
		{
			int exitStatus = kernel->machine->ReadRegister(4);

			if (exitStatus != 0)
				increasePC();

			int res = pTab->ExitUpdate(exitStatus);

			kernel->currentThread->FreeSpace();
			kernel->currentThread->Finish();
			kernel->machine->WriteRegister(2, res);
			increasePC();

			return;
			ASSERTNOTREACHED();
			break;
		}
		case SC_Add:
			DEBUG(dbgSys, "Adding... " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							/* int op2 */ (int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			increasePC();

			return;
			ASSERTNOTREACHED();
			break;

		case SC_ReadNum: //Read Interger
		{
			int virtAddr, length = 10;
			char *buffer;
			virtAddr = kernel->machine->ReadRegister(4);
			buffer = User2System(virtAddr, length); // Copy string from User Space to System Space
			int i = 0;
			int numLenght = 0;
			while (true)
			{
				buffer[i] = kernel->synchConsoleIn->GetChar(); // Read buffer from console
				numLenght++;
				if (buffer[i] == '\n')
				{
					buffer[i] = '\0';
					break;
				}
				i++;
			}

			System2User(virtAddr, length, buffer); // Copy string from System Space to User Space

			bool isNegative = false;
			int firstNum = 0;
			int lastNum = 0;
			if (buffer[0] == '-') // Check negative number
			{
				isNegative = true;
				firstNum = 1;
				lastNum = 1;
			}

			for (int i = firstNum; i < numLenght; i++)
			{
				if (buffer[i] == '.') /// x.00 still an integer
				{
					for (int j = i + 1; j < numLenght; j++)
					{
						if (buffer[j] != '0')
						{
							char *error = (char *)"The inputted number is not integer or is a real number";

							int i = 0;
							while (error[i] != '\0')
							{
								kernel->synchConsoleOut->PutChar(error[i]); // Print error to console
								i++;
							}
							kernel->synchConsoleOut->PutChar('\n');

							kernel->machine->WriteRegister(2, 0);

							increasePC();
							delete buffer;
							return;
						}
					}
					lastNum = i - 1;
					break;
				}
				else if (ctoi(buffer[i]) == -1) //meet unknown character
				{
					if (buffer[i] == '\0')
					{
						break;
					}
					else
					{
						char *error = (char *)"The inputted number is not integer";

						int i = 0;
						while (error[i] != '\0')
						{
							kernel->synchConsoleOut->PutChar(error[i]); // Print error to console
							i++;
						}
						kernel->synchConsoleOut->PutChar('\n');
						kernel->machine->WriteRegister(2, 0);

						delete buffer;
						increasePC();
						return;
					}
				}
				lastNum = i;
			}

			int number = 0;

			for (int i = firstNum; i <= lastNum; i++)
			{
				number = number * 10 + ctoi(buffer[i]);
			}

			if (isNegative)
			{
				number = number * -1;
			}

			kernel->machine->WriteRegister(2, number);

			delete buffer;
			increasePC();
			return;
		}
		case SC_PrintNum: //Print Interger on screen
		{
			number = (int)kernel->machine->ReadRegister(4); //Read form register
			if (number == 0)
			{
				kernel->synchConsoleOut->PutChar('0'); // Print 0
				increasePC();
				return;
			}

			bool isNegative = false;
			int numberOfNum = 0;
			int firstNum = 0;

			if (number < 0)
			{
				isNegative = true;
				number = number * -1; // transist to positive number
				firstNum = 1;
			}

			int t_number = number;
			while (t_number) // count digit
			{
				numberOfNum++;
				t_number /= 10;
			}

			char *buffer;
			buffer = new char[MAX_BUFFER + 1];
			for (int i = firstNum + numberOfNum - 1; i >= firstNum; i--)
			{
				buffer[i] = (char)((number % 10) + 48); // cast int to char into buffer
				number /= 10;
			}

			if (isNegative) //print negative number
			{
				buffer[0] = '-'; // Insert '-'
				buffer[numberOfNum + 1] = 0;

				int i = 0;
				while (buffer[i] != '\0') // Print buffer
				{
					kernel->synchConsoleOut->PutChar(buffer[i]);
					i++;
				}
				kernel->synchConsoleOut->PutChar('\n');
				delete buffer;
				increasePC();
				return;
			}
			else // Print positive number
			{
				buffer[numberOfNum] = 0;

				int i = 0;
				while (buffer[i] != '\0')
				{
					kernel->synchConsoleOut->PutChar(buffer[i]);
					i++;
				}
				kernel->synchConsoleOut->PutChar('\n');
			}

			delete buffer;
			increasePC();
			return;
			ASSERTNOTREACHED();
			break;
		}

		case SC_RandomNum:
		{
			RandomInit(time(0));
			int x = RandomNumber();
			DEBUG(dbgSys, x);
			kernel->machine->WriteRegister(2, int(x));

			increasePC();

			return;

			ASSERTNOTREACHED();

			break;
		}
		case SC_ReadChar:

			c = kernel->synchConsoleIn->GetChar();
			kernel->machine->WriteRegister(2, (int)c);

			increasePC();

			return;
			ASSERTNOTREACHED();
			break;

		case SC_PrintChar:

			c = (char)kernel->machine->ReadRegister(4);
			kernel->synchConsoleOut->PutChar(c);

			increasePC();

			return;
			ASSERTNOTREACHED();
			break;

		case SC_ReadString:
			buffer = kernel->machine->ReadRegister(4); //Get the value from register r4
			length = kernel->machine->ReadRegister(5); //Get the value from register r5
			buf = NULL;

			if (length > 0)
			{
				buf = new char[length];
				for (int i = 0; i < length - 1; i++)
				{
					c = kernel->synchConsoleIn->GetChar();
					if (c == '\n')
						break;
					else
						buf[i] = c;
				}
			}

			if (buf != NULL)
			{
				int n = strlen(buf) + 1;
				for (int i = 0; i < n; i++)
				{
					kernel->machine->WriteMem(buffer + i, 1, (int)buf[i]); //Write to memory
				}
				delete[] buf;
			}

			increasePC();

			return;
			ASSERTNOTREACHED();
			break;

		case SC_PrintString: //Print char*
		{
			vaddr = kernel->machine->ReadRegister(4);
			kernel->machine->ReadMem(vaddr, 1, &memval); //read memory to get value address
			while ((*(char *)&memval) != '\0')			 //While not end of string (\0)
			{
				kernel->synchConsoleOut->PutChar(*(char *)&memval); //Write each char
				vaddr++;
				kernel->machine->ReadMem(vaddr, 1, &memval); //Read each char from memory to write
			}

			increasePC();

			return;
			ASSERTNOTREACHED();
			break;
		}
		case SC_Sleep:
		{
			// int val = kernel->machine->ReadRegister(4);
			// cout << "Sleep Time:" << val << "(ms)" << endl;
			// kernel->alarm->WaitUntil(val);
			// break;
		}

		default:
		{
			cerr << "Unexpected user mode exception" << (int)which << "\n";
			break;
		}
			ASSERTNOTREACHED();
		}
	}
}
