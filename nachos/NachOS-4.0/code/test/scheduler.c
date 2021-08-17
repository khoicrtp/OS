#include "syscall.h"

void main()
{
    int pingPID, pongPID;
    PrintString("PING PONG:\n");
    Exec("test.c");
    //pingPID = Exec("./ping.c");
    //pongPID = Exec("./ping.c");

    Halt();
}