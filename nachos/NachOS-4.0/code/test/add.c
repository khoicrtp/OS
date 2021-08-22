/* add.c
 *	Simple program to test whether the systemcall interface works.
 *	
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int main()
{
  int result;
  char c;
  char str[100];
  result=ReadNum();

  PrintNum(150125012);
  Halt();
  /* not reached */
}
