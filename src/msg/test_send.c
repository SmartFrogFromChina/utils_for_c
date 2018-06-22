#include <stdio.h>
#include <string.h>
#include "msg_queue.h"


int main(int argc , char **argv)
{
    char *buf = NULL;
    
    if(argc >= 2)
    {
        buf = argv[1];
    }
    else
    {
        buf = "StartSpeech";
    }
    msg_queue_create_msg_queue(MAGIC_NUMBER);
    msg_queue_send(buf, MSG_UARTD_TO_IOT);
    
    return 0;
}