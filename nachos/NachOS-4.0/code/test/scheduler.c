#include "syscall.h"

void ping(){
    int i;
    for (i = 0; i < 1000; i++)
    {
        PrintChar('A');
    }
}

void pong(){
    int i;
    for (i = 0; i < 1000; i++)
    {
        PrintChar('B');
    }
}


void main()
{
    int pingPID, pongPID;
    PrintString("PING PONG:\n");
    // pingPID = Exec("./ping.c");
    // pongPID = Exec("./ping.c");

    pingPID = Exec(&ping);
    pongPID = Exec(&pong);

    Halt();
}