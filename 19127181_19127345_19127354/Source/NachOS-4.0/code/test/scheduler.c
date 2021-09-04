#include "syscall.h"

void main(){
    PrintString("\n____PING PONG___\n");
    Exec("ping");
    Exec("pong");
    PrintString("\n___FINISH___\n");
}

