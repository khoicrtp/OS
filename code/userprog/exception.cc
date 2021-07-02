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

#define MaxFileLength 32
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
char *User2System(int virtAddr, int limit)
{
	int i; // index
	int oneChar;
	char *kernelBuf = NULL;

	kernelBuf = new char[limit + 1]; //need for terminal string
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf, 0, limit + 1);

	//printf("\n Filename u2s:");
	for (i = 0; i < limit; i++)
	{
		kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		//printf("%c",kernelBuf[i]);
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}
/* 
Input: - User space address (int) 
 - Limit of buffer (int) 
 - Buffer (char[]) 
Output:- Number of bytes copied (int) 
Purpose: Copy buffer from System memory space to User memory space 
*/
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
	/* set previous programm counter (debugging only)*/
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

	/* set next programm counter for brach execution */
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
}

void ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case NoException:
		return;

	case PageFaultException:
		DEBUG(dbgFile, "\n No valid translation found");
		SysHalt();
		break;

	case ReadOnlyException:
		DEBUG(dbgFile, "\n Write attempted to page marked read-only");
		SysHalt();
		break;

	case BusErrorException:
		DEBUG(dbgFile, "\n Translation resulted invalid physical address");
		SysHalt();
		break;

	case AddressErrorException:
		DEBUG(dbgFile, "\n Unaligned reference or one that was beyond the end of the address space");
		SysHalt();
		break;

	case OverflowException:
		DEBUG(dbgFile, "\nInteger overflow in add or sub.");
		SysHalt();
		break;

	case IllegalInstrException:
		DEBUG(dbgFile, "\n Unimplemented or reserved instr.");
		SysHalt();
		break;

	case NumExceptionTypes:
		DEBUG(dbgFile, "\n Number exception types");
		SysHalt();
		break;

	case SyscallException:
		switch (type)
		{
		// case SC_Create:
		// {
		// 	int virtAddr;
		// 	char *filename;
		// 	DEBUG(dbgSys, "\n SC_Create call ...");
		// 	DEBUG('a', "\n Reading virtual address of filename");
			
		// 	virtAddr = kernel->machine->ReadRegister(4);
		// 	DEBUG('a', "\n Reading filename.");

		// 	filename = User2System(virtAddr, MaxFileLength + 1);
		// 	if (filename == NULL)
		// 	{
		// 		//printf("\n Not enough memory in system");
		// 		DEBUG('a', "\n Not enough memory in system");
		// 		kernel->machine->WriteRegister(2, -1); 
			
		// 		delete filename;
		// 		return;
		// 	}
		// 	DEBUG('a', "\n Finish reading filename.");
			
		// 	if (!kernel->fileSystem->Create(filename, 0))
		// 	{
		// 		//printf("\n Error create file '%s'", filename);
		// 		kernel->machine->WriteRegister(2, -1);
		// 		delete filename;
		// 		increasePC();
		// 		return;
		// 	}
		// 	kernel->machine->WriteRegister(2, 0); 
		// 	delete filename;
		// 	increasePC();
		// 	break;
		// }

		case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

			SysHalt();

			ASSERTNOTREACHED();
			break;

		case SC_Add:
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = (int)kernel->machine->ReadRegister(4) + (int)kernel->machine->ReadRegister(5);

			DEBUG(dbgSys, "Add returning with " << result << "\n");

			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			increasePC();
			/* Modify return point */

		case SC_ReadInt:
		{
			// Input: K co
			// Output: Tra ve so nguyen doc duoc tu man hinh console.
			// Chuc nang: Doc so nguyen tu man hinh console.
			//SynchConsoleInput synchIn;
			char *buffer;
			int MAX_BUFFER = 255;
			buffer = new char[MAX_BUFFER + 1];
			int numbytes = kernel->synchConsoleIn->GetChar(); // doc buffer toi da MAX_BUFFER ki tu, tra ve so ki tu doc dc
			int number = 0;									  // so luu ket qua tra ve cuoi cung

			/* Qua trinh chuyen doi tu buffer sang so nguyen int */

			// Xac dinh so am hay so duong
			bool isNegative = false; // Gia thiet la so duong.
			int firstNum = 0;
			int lastNum = 0;
			if (buffer[0] == '-')
			{
				isNegative = true;
				firstNum = 1;
				lastNum = 1;
			}

			// check legit of inputted number
			for (int i = firstNum; i < numbytes; i++)
			{
				if (buffer[i] == '.') /// 125.0000000 van la so
				{
					int j = i + 1;
					for (; j < numbytes; j++)
					{
						// So khong hop le
						if (buffer[j] != '0')
						{
							DEBUG('a', "\n The integer number is not valid");
							kernel->machine->WriteRegister(2, 0);
							
							increasePC();
							delete buffer;
							return;
						}
					}
					// la so thoa cap nhat lastNum
					lastNum = i - 1;
					break;
				}
				else if (buffer[i] < '0' && buffer[i] > '9')
				{
					DEBUG('a', "\n The integer number is not valid");
					kernel->machine->WriteRegister(2, 0);
					
					increasePC();
					delete buffer;
					return;
				}
				lastNum = i;
			}

			// La so nguyen hop le, tien hanh chuyen chuoi ve so nguyen
			for (int i = firstNum; i <= lastNum; i++)
			{
				number = number * 10 + (int)(buffer[i] - 48);
			}

			// neu la so am thi * -1;
			if (isNegative)
			{
				number = number * -1;
			}
			kernel->machine->WriteRegister(2, number);
			
			increasePC();
			delete buffer;
			return;
		}

		case SC_PrintInt:
		{
			// Input: mot so integer
			// Output: khong co
			// Chuc nang: In so nguyen len man hinh console
			int number = kernel->machine->ReadRegister(4);
			if (number == 0)
			{
				//gSynchConsole->Write("0", 1); // In ra man hinh so 0
				kernel->synchConsoleOut->PutChar('0');
				
				increasePC();
				return;
			}

			/*Qua trinh chuyen so thanh chuoi de in ra man hinh*/
			bool isNegative = false; // gia su la so duong
			int numberOfNum = 0;	 // Bien de luu so chu so cua number
			int firstNum = 0;

			if (number < 0)
			{
				isNegative = true;
				number = number * -1; // Nham chuyen so am thanh so duong de tinh so chu so
				firstNum = 1;
			}

			int t_number = number; // bien tam cho number
			while (t_number)
			{
				numberOfNum++;
				t_number /= 10;
			}

			// Tao buffer chuoi de in ra man hinh
			char *buffer;
			int MAX_BUFFER = 255;
			buffer = new char[MAX_BUFFER + 1];
			for (int i = firstNum + numberOfNum - 1; i >= firstNum; i--)
			{
				buffer[i] = (char)((number % 10) + 48);
				number /= 10;
			}
			if (isNegative)
			{
				buffer[0] = '-';
				buffer[numberOfNum + 1] = 0;

				kernel->synchConsoleOut->PutChar(*buffer);
				delete buffer;
				increasePC();
				return;
			}
			buffer[numberOfNum] = 0;
			kernel->synchConsoleOut->PutChar(*buffer);
			
			delete buffer;
			increasePC();
			return;
		}

		case SC_ReadChar:
		{
			//Input: Khong co
			//Output: Duy nhat 1 ky tu (char)
			//Cong dung: Doc mot ky tu tu nguoi dung nhap
			//int maxBytes = 255;
			char *buffer = new char[255];
			int numBytes = kernel->synchConsoleIn->GetChar();

			if (numBytes > 1) //Neu nhap nhieu hon 1 ky tu thi khong hop le
			{
				printf("Chi duoc nhap duy nhat 1 ky tu!");
				DEBUG('a', "\nERROR: Chi duoc nhap duy nhat 1 ky tu!");
				kernel->machine->WriteRegister(2, 0);
			}
			else if (numBytes == 0) //Ky tu rong
			{
				printf("Ky tu rong!");
				DEBUG('a', "\nERROR: Ky tu rong!");
				kernel->machine->WriteRegister(2, 0);
			}
			else
			{
				//Chuoi vua lay co dung 1 ky tu, lay ky tu o index = 0, return vao thanh ghi R2
				char c = buffer[0];
				kernel->machine->WriteRegister(2, c);
			}

			delete buffer;
			increasePC();
			break;
		}

		case SC_PrintChar:
		{
			// Input: Ki tu(char)
			// Output: Ki tu(char)
			// Cong dung: Xuat mot ki tu la tham so arg ra man hinh
			char c = (char)kernel->machine->ReadRegister(4); // Doc ki tu tu thanh ghi r4
			kernel->synchConsoleOut->PutChar(c);			 // In ky tu tu bien c, 1 byte
			increasePC();
			break;
		}

		case SC_ReadString:
		{
			// Input: Buffer(char*), do dai toi da cua chuoi nhap vao(int)
			// Output: Khong co
			// Cong dung: Doc vao mot chuoi voi tham so la buffer va do dai toi da
			int virtAddr, length;
			char *buffer;
			virtAddr = kernel->machine->ReadRegister(4); // Lay dia chi tham so buffer truyen vao tu thanh ghi so 4
			length = kernel->machine->ReadRegister(5);	 // Lay do dai toi da cua chuoi nhap vao tu thanh ghi so 5
			buffer = User2System(virtAddr, length);		 // Copy chuoi tu vung nho User Space sang System Space
			kernel->synchConsoleOut->PutChar(*buffer);	 // Goi ham Read cua SynchConsole de doc chuoi
			System2User(virtAddr, length, buffer);		 // Copy chuoi tu vung nho System Space sang vung nho User Space

			delete buffer;
			increasePC(); // Tang Program Counter
			return;
			//break;
		}

		case SC_PrintString:
		{
			// Input: Buffer(char*)
			// Output: Chuoi doc duoc tu buffer(char*)
			// Cong dung: Xuat mot chuoi la tham so buffer truyen vao ra man hinh
			int virtAddr;
			char *buffer;
			virtAddr = kernel->machine->ReadRegister(4); // Lay dia chi cua tham so buffer tu thanh ghi so 4
			buffer = User2System(virtAddr, 255);		 // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai 255 ki tu
			
			int i = 0;
			while (buffer[i] != '\0'){
				DEBUG(dbgSys, buffer[i]);
			}
			DEBUG(dbgSys, *buffer);

			kernel->synchConsoleOut->PutChar(*buffer); // Dem do dai that cua chuoi

			delete buffer;
			//increasePC(); // Tang Program Counter
			//return;
			break;
		}

			return;

			//ASSERTNOTREACHED();

			break;

		default:
			cerr << "Unexpected system call " << type << "\n";
			break;
		}
		break;
	}
	//ASSERTNOTREACHED();
}
