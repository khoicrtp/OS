#include "syscall.h"

void main()
{
    int pingPID, pongPID, i;
    PrintString("----PING PONG----\n");
    // for (i = 0; i < 10; i++)
    // {
    //     PrintChar('%');
    // }
    //Exec("ping");
    Exec("ping");
    PrintString("--------TEST-------\n");
    Exec("pong");
    ExecLoaded();

    //ReadChar();
    //pingPID = Exec("./ping.c");
    //pongPID = Exec("./ping.c");

    //Halt();
}