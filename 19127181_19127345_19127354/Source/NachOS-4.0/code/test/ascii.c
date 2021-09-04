/* ascii.c
 *	Simple program to print readable character in ascii table by systemcall
 */

#include "syscall.h"

int main()
{
  PrintChar('a');
  PrintChar('b');

  Halt();
  /* not reached */
}