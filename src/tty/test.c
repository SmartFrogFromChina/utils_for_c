#include <stdio.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tty.h"

int main()
{
    int ret=0;
    int fd = 0;
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
		            }
		    } 
    }
    ttys0_deinit();
    return 0;
}
