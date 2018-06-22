#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "msg_queue.h"


int main(int argc , char **argv)
{
    char buf[32] = {0};
    char *tmp = NULL;
    
    msg_queue_create_msg_queue(MAGIC_NUMBER);
    while(1)
    {
        memset(buf,0,32);
        tmp = msg_queue_recv(MSG_UARTD_TO_IOT);
        if(tmp)
        {   
            strncpy(buf,tmp,32);
            free(tmp);
            if(!strcmp(buf,"quit"))
                break;
            
            printf("print start:buf = %s\n",buf);
        }
    }
    msg_queue_del();
    return 0;
    }