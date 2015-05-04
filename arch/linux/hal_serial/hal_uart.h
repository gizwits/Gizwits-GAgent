#ifndef _UART_H_
#define _UART_H_
#include "platform.h"
int serial_open(char  *comport, int bandrate,int nBits,char nEvent,int nStop );
int serial_write( int serial_fd,unsigned char *buf,int buflen );
int serial_read( int serial_fd, unsigned char *buf,int buflen );
#endif