#include <stdio.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>
#include <stdlib.h>
#include <fcntl.h>
#include "tty.h"

static int ttyS1Fd = 0;
static pthread_mutex_t ttyS1Mut = PTHREAD_MUTEX_INITIALIZER;

static int ttyS0Fd = 0;
static pthread_mutex_t ttyS0Mut = PTHREAD_MUTEX_INITIALIZER;

static int open_port(char *dev)
{
    int fd;

    fd = open(dev, O_RDWR|O_NOCTTY|O_NDELAY);
    if (-1 == fd)
    {
        perror("Can't Open Serial Port");
        return(-1);
    }
    return fd;
}


static int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio, oldtio;

    if ( tcgetattr( fd,&oldtio) != 0)
    {
        printf("SetupSerial 1");
        return -1;
    }
    bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch( nBits )
    {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }

    switch( nEvent )
    {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E':
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':
        newtio.c_cflag &= ~PARENB;
        break;
    }
    switch( nSpeed )
    {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    default:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    }
    if ( nStop == 1 )
    {
        newtio.c_cflag &= ~CSTOPB;
    }
    else if ( nStop == 2 )
    {
        newtio.c_cflag |= CSTOPB;
    }
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN]  = 1;
    tcflush(fd,TCIFLUSH);
    if ((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        perror("com set error:");
        return -1;
    }
    return 0;
}

static int tty_init(char *name, int paud)
{
    int fd = -1;
    fd = open_port(name);
    if(fd <= 0) 
    {
        perror("open port failed:");
        exit(-1);
    }
    set_opt(fd,paud,8,'N',1);

	return fd;
}

static int tty_write(int fd , unsigned char *buf, int len)
{
    return write(fd, buf, len);
}

static int tty_read(int fd , unsigned char *buf, int len)
{
    return read(fd, buf, len);
}

int ttys0_init(int paud)
{
    ttyS0Fd = tty_init("/dev/ttySGK2", paud);
    return ttyS0Fd;
}
int ttys0_deinit()
{
    if(ttyS0Fd > 0)
        close(ttyS0Fd);
}

int ttys0_write(unsigned char *buf, int len)
{
    int ret;
    pthread_mutex_lock(&ttyS0Mut);
    ret = tty_write(ttyS0Fd, buf, len);
    pthread_mutex_unlock(&ttyS0Mut);
    return ret;
}

int ttys0_read(unsigned char *buf, int len)
{
    int ret;
    pthread_mutex_lock(&ttyS0Mut);
    ret = tty_read(ttyS0Fd, buf, len);
    pthread_mutex_unlock(&ttyS0Mut);
    return ret;
}

int ttys0_get_fd()
{
    return ttyS0Fd;
}


int ttys1_init(int paud)
{
    ttyS1Fd = tty_init("/dev/ttyS1", paud);
    return ttyS1Fd;
}
int ttys1_deinit()
{
    if(ttyS1Fd > 0)
        close(ttyS1Fd);
}

int ttys1_write(unsigned char *buf, int len)
{
    int ret ;
    pthread_mutex_lock(&ttyS1Mut);
    ret = tty_write(ttyS1Fd, buf, len);
    pthread_mutex_unlock(&ttyS1Mut);
    return ret;
}

int ttys1_read(unsigned char *buf, int len)
{
    int ret;
    pthread_mutex_lock(&ttyS1Mut);
    ret = tty_read(ttyS1Fd, buf, len);
    pthread_mutex_unlock(&ttyS1Mut);
    return ret;
}

int ttys1_get_fd()
{
    return ttyS1Fd;
}




