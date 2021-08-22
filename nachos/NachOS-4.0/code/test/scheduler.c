#include "syscall.h"

void main(){
    PrintString("____PING PONG___");
    //PrintChar('*');
    //PrintChar('*');
    //PrintChar('^');
    Exec("ping");
    PrintString("STARTING PING___");
    //PrintChar('&');
    //PrintChar('&');
    //PrintChar('&');
    Exec("pong");
    //PrintChar('~');
    // PrintChar('~');
    // PrintChar('~');
    //PrintChar('~');
    //PrintString("AB");
    //ReadChar();
    PrintString("___FINISH___");
}