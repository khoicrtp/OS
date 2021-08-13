/* help.c
 *	Simple program to introduce some basics about our team and briefly discription about the sort and acsii program.
 */

#include "syscall.h"
#include "../userprog/syscall.h"

int main()
{
  PrintString("!!!!!!!This is our NachOS 4.0 project!!!!!!!\n");
  PrintString("Our team members: Nguyen Cao Thien Long - ID: 19127203\n");
  PrintString("                  Doan Nhat Minh - ID: 19127469\n");
  PrintString("And now , we will introduce some system call in Nachos 4.0 throught two example: \n");
  PrintString("1.Ascii example : This example is about to use some system call as PrintNum,PrintChar... \n");
  PrintString("                  to print all readable character in Ascii table\n");
  PrintString("2.Sort example :  Like example 1, this example also use system call to Sort array of number \n");
  PrintString("                  to ascending order or descending order by Bubble Sort Algorithm\n");
  PrintString("That is all about our project.We hope that it will please you\n");
  PrintString("Thank you for reading :3\n");
  Halt();
  /* not reached */
}