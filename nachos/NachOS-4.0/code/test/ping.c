//Print A
#include "syscall.h"

void main(){
    int j;
    for (j = 0; j < 10; j++)
    {
        PrintChar('A');
    }
    Exec("pong");
    //Exit(0);
}