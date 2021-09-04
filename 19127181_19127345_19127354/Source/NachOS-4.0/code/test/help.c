/* help.c
 *	Simple program to introduce some basics about our team and briefly discription about the sort and acsii program.
 */

#include "syscall.h"

int main()
{
  PrintString("|-----------NACHOS-4.0-----------|\n");
  PrintString("|Bach Minh Khoi - MSSV: 19127181 |\n");
  PrintString("|Lam Quoc Cuong - MSSV: 19127345 |\n");
  PrintString("|Le Thanh Dat   - MSSV: 19127354 |\n");
  PrintString("|--------------------------------|\n");
  Halt();
  /* not reached */
}