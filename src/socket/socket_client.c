//用于进程间通信，作为客户端，发送数据
#include <stdio.h>
#include <sys/socket.h> 
#include <sys/un.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "msg.h"

int send_to_server(msg_t *msg)
{
    int iRet = -1;
    int iConnect_fd = -1;
    int flags = -1;
    int len ;
    struct sockaddr_un srv_addr;
    char *sendData = NULL;
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path, SOCKET_FILE);
    iConnect_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(iConnect_fd < 0)
    {
        printf("\033[1;32;40mcannot create communication socket. \033[0m\n");
        return 1;
    }
    flags = fcntl(iConnect_fd, F_GETFL, 0);
    fcntl(iConnect_fd, F_SETFL, flags|O_NONBLOCK);
    len = sizeof(msg_t);
    printf("len:%d  %ld\n", len,sizeof(msg_t));
    if(0 != connect(iConnect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)))
    {
        printf("\033[1;32;40mconnect server failed..\033[0m\n");
    }
    iRet = write(iConnect_fd, msg , len);
    printf("iRet:%d\n", iRet);
    if (iRet != len)
    {
        printf("\033[1;32;40mwrite failed..\033[0m\n");
        iRet = -1;
    }
    close(iConnect_fd);
    return iRet;
}
int send_cmd_play(int operate)
{
    char *filename="https://www.linuxidc.com/Linux/2017-02/140571.htm";
    msg_t msg;
    memset(&msg, 0, sizeof(msg_t));
    msg.type = 'P';
    msg.len = strlen(filename);
    memcpy(&msg.data, filename, strlen(filename));
    msg.operate = operate;
    return send_to_server(&msg);
}
int main(int argc,char **argv)
{
    int i = 0;
    int times = 0;
   
    if(argc >=2)
    {
        times = atoi(argv[1]);
    }
    else
    {
        times = 5;
    }
       
    for(i=0; i< times; i++)
    {
        send_cmd_play(0x02);
        usleep(1000*1000);
    }
       
    return 0;
}