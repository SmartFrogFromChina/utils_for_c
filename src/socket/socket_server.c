#include <stdio.h>
#include <sys/socket.h> 
#include <sys/un.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "msg.h"

int creat_socket_server(char *ppath)
{
    int listen_fd;
    int ret = -1;
    struct sockaddr_un srv_addr;
    unlink(ppath);
    listen_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(listen_fd < 0)
    {
        printf("cannot create communication socket");
        return -1;
    } 
    //set server addr_param
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path, ppath);
    ret = bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if(ret < 0)
    {
        printf("cannot bind server socket");
        close(listen_fd);
        unlink(ppath);
        return -1;
    }
    ret = listen(listen_fd, 4);
    if(ret < 0)
    {
        printf("cannot listen the client connect request");
        close(listen_fd);
        unlink(ppath);
        return 1;
    }
    return listen_fd;
}
int socket_accept(int iSocket)
{
    int iConnectSocket = -1;
    struct sockaddr_in ClientAddr;
    struct timeval sTimeout;
    fd_set socketSet;
    unsigned long dwLen = 0;
    // socket
    if (iSocket <= 0)
    {
        printf("error: socket invalid error!");
        return -1;
    }
    memset(&ClientAddr, 0, sizeof(ClientAddr));
    // select
    sTimeout.tv_sec  = 0;
    sTimeout.tv_usec = 1000 * 500;
    FD_ZERO(&socketSet);
    FD_SET(iSocket, &socketSet);
    // select
    if (select(iSocket + 1, (fd_set*)&socketSet, (fd_set*)NULL, (fd_set*)NULL, &sTimeout ) <= 0)
    {
        return -2;
    }
    else
    {
        // accept
        dwLen = sizeof(struct sockaddr_in);
        iConnectSocket = accept(iSocket, (struct sockaddr *)&ClientAddr, (socklen_t *)&dwLen);
    }
    return iConnectSocket;
}
int socket_recv_nowait(int iSocket, unsigned char * pByBuf, int iLen, int * piRecvLen)
{
    struct timeval sTimeout;
    fd_set socketSet;
    int ret = -1;
    int iRecvCount = 0;
    // socket
    if (iSocket <= 0)
    {
        printf("socket invalid error!!!\r\n");
        return -1;
    }
    sTimeout.tv_sec  = 0;
    sTimeout.tv_usec = 1000 * 100;
    FD_ZERO(&socketSet);
    FD_SET(iSocket, &socketSet);
    // select
    ret = select(iSocket + 1, (fd_set*)&socketSet, (fd_set*)NULL, (fd_set*)NULL, &sTimeout );
    if (ret <  0)
    {
        // Error or Timeout
        printf("Error occur............");
        return -2;
    }
    else if(ret == 0)
    {
        printf("Time out............");
        return -1;
    }
    else
    {
        printf("Received data ");
        if (FD_ISSET(iSocket, &socketSet) > 0)
        {
            // Receive data from the agent.
            iRecvCount = read(iSocket, (char *)pByBuf, iLen);
            if (0 < iRecvCount)
            {
                *piRecvLen = iRecvCount;
            }
            close(iSocket);
        }
    }
    return 0;
}
//解析/tmp/msg_socket中的数据，该数据来自send_to_server()
int main(int argc,char **argv)
{
    int iCount = 0;
    int iRet = -1;
    int iSocket = -1;
    int iConnect;
    int iRecvLen = 0;
    char byReadBuf[FIFO_BUFF_LENGTH] = {0};
    msg_t *msg = NULL;
    iSocket = creat_socket_server(SOCKET_FILE);
    while (1)
    {
        iConnect = socket_accept(iSocket);
        if (iConnect >= 0)
        {
            printf("iConnect = %d\n",iConnect);
            memset(byReadBuf, 0, FIFO_BUFF_LENGTH);
            iRet = socket_recv_nowait(iConnect, byReadBuf, FIFO_BUFF_LENGTH, &iRecvLen);
            if (0 == iRet)
            {
                msg = (msg_t *)byReadBuf;
                printf("turingIot--->uartd:0x%02x 0x%02x len=%d\n" ,msg->type,msg->operate,iRecvLen);
                switch(msg->type)
                {
                    case 0x44:break;
                    case 0x4C:break;
                    case 0x53:break;
                    default:printf("Testing  \n");break;
                }
            }
            else
            {
                printf("iRet = %d\n",iRet);
            }
        }
    }
    usleep(200*1000);
    return 0;
}