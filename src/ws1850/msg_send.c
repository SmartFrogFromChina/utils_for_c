#include <stdio.h>
#include <string.h>

#include "msg_queue.h"

void print_msg_usage()
{
    printf( "usage:  \n"
            "msg_send recordOpen      打开录音文件\n"
            "msg_send recordStart     保存录音到文件\n"
            "msg_send recordStop      关闭录音文件\n"

    );
}


void parse_cmdline(int argc,char **argv,char *buf )
{
    if(argc == 2)
    {
        if(!strcmp(argv[1],"help"))
        {
            print_msg_usage();
            exit(0) ;
        }
    }
    else
    {
        print_msg_usage();
        exit(0) ;
    }
    int i = 0;
    char tmp[256];
    for(i=1; i<argc; i++ )
    {
        sprintf(tmp,"%s ",argv[i]);
        strcat(buf,tmp);
    }
    buf[strlen(buf)-1] = 0;

}


// myplay add http://ddddddddddddd.mp3

int main(int argc ,char **argv)
{
    char msg[512] = {0};

    parse_cmdline(argc,argv,msg);
    msg_queue_create_msg_queue(MAGIC_NUMBER);
    msg_queue_send(msg,MSG_TO_TURING);
    
    return 0;
}



