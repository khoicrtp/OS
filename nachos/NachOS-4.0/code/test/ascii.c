/* ascii.c
 *	Simple program to print readable character in ascii table by systemcall
 */

#include "syscall.h"

int main()
{
  int i=33;
  while(i<127)
  {
    PrintNum(i);
    PrintString(" ");
    PrintChar(i);
    PrintString("\n");
    i=Add(i,1);
  }

  Halt();
  /* not reached */
}