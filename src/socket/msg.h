#ifndef __MSG_H__
#define __MSG_H__


#define SOCKET_FILE "/tmp/msg_socket"

#define FIFO_BUFF_LENGTH 256

typedef struct  _msg {
    unsigned char type;
    unsigned char operate;
    unsigned int len;
    char data[128];
}msg_t;
#endif