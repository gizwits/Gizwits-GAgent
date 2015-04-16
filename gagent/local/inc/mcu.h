
#ifndef  __HANDLE_UART_H_
#define  __HANDLE_UART_H_
#include "lan.h"


void WiFiReset();
int GAgent_CheckAck( int fd, pgcontext pgc,unsigned char *buf,int bufLen,ppacket pRxbuf,u32 time );
int GAgent_Ack2Mcu(int fd, char sn,char cmd);
#endif
