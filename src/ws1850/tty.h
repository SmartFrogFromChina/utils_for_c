#ifndef __TTY_H__
#define __TTY_H__

int ttys0_init();
int ttys0_deinit();
int ttys0_write(unsigned char *buf, int len);
int ttys0_read(unsigned char *buf, int len);
int ttys0_get_fd();



int ttys1_init();
int ttys1_deinit();
int ttys1_write(unsigned char *buf, int len);
int ttys1_read(unsigned char *buf, int len);
int ttys1_get_fd();






#endif
