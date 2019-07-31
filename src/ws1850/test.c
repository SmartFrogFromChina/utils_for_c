#include <stdio.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tty.h"
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "msg_queue.h"
#include "slog.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>


unsigned char  cmd ;

void msg_parse(char *buf)
{
    inf("msg_queue:[%s]",buf);
    
    if (!strcmp(buf,"recordOpen"))
    {
        cmd = 0x37;
    }
    else if (!strcmp(buf,"recordStart"))
    {
        cmd = 2;
    }
    else if (!strcmp(buf,"recordStop"))
    {
        cmd = 3;
    }    
    
    ttys0_write(&cmd,1);
}

void * parse_msg_queue_data()
{
    char buf[1024] = {0};
    char *tmp = NULL;
    
    //set_thread_name("msg_queue");
    
    msg_queue_create_msg_queue(MAGIC_NUMBER);
    
    while(1)
    {
        memset(buf,0,1024);
        tmp = msg_queue_recv(MSG_TO_TURING);
        if(tmp)
        {   
            strncpy(buf,tmp,1024);
            free(tmp);
            
            msg_parse(buf);
        }
    }
}


int fd = 0;
    
    
void * tty_read_thread(void *arg)
{
    int ret=0;

    int i = 0;
    fd_set rd;
    unsigned char buf[256]={0};
    
    ttys0_init(9600);
    fd = ttys0_get_fd();
    
    while(1)
    {
        memset(buf, 0, sizeof(buf));
        FD_ZERO(&rd);
		FD_SET(fd, &rd);
		    
        ret = select(fd+1, &rd, NULL, NULL, NULL);
        switch(ret)
        {
            case -1:perror("select");break;
            case  0:perror("time is over!");break;
            default:
                {
                    ret = ttys0_read(buf, 256);
                    if(ret < 0)
                        continue;
                    buf[ret] = 0;
                    //将接收到的数据发送给处理函数即可
                    //可以是消息队列、队列、管道、本地socket
                    
                    for(i=0; i<ret; i++)
                    {
                        printf("0x%02x ",buf[i]);
                    }
                    printf("\n");
                }
        } 
    }
    ttys0_deinit();
    return 0;
}

pthread_t thread[2];
void create_monitor_tty_thread()
{
    int iRet = 0;
    
	if((iRet = pthread_create(&thread[0], NULL, tty_read_thread, NULL)) != 0)
	{
		error("tty_read_thread create failed..");
	}
    
	if((iRet = pthread_create(&thread[1], NULL, parse_msg_queue_data, NULL)) != 0)
	{
		error("parse_msg_queue_data reate error:");
	}
}

static sem_t  main_run_sem;

int main()
{
    log_init("1111111",NULL);
    sem_init(&main_run_sem,0,0);
    create_monitor_tty_thread();
    sem_wait(&main_run_sem);
}
