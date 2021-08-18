#include "syscall.h"

void main()
{
    int pingPID, pongPID, i;
    PrintString("----PING PONG----\n");
    // for (i = 0; i < 10; i++)
    // {
    //     PrintChar('A');
    // }
    //DEBUG(dbgSys, "---PING PONG DEBUG---\n");
    Exec("ping");
    Exec("pong");
    //pingPID = Exec("./ping.c");
    //pongPID = Exec("./ping.c");

    //Halt();
}