/* add.c
 *	Simple program to test whether the systemcall interface works.
 *	
 *	Just do a add syscall that adds two values and returns the result.
 *
 */

#include "syscall.h"

int main()
{
  int len, result, a, b;
  char filename[24];
  char str[] = "NachOS \n";

  PrintString("NachOS");

  //result = Add(100, 200);
  
  //PrintInt(result);
  
  Halt();
  /* not reached */
}
