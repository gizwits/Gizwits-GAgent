#ifndef __PLATFORM_H_
#define __PLATFORM_H_
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <sys/un.h> 
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <netdb.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define sockaddr_t sockaddr_in
#define _POSIX_C_SOURCE 200809L

#include "hal_uart.h"
#include "gagent_typedef.h"

#define WIFI_SOFTVAR    "04010004"
#define WIFI_HARDVER    "00-LINUX"

#define UART_NAME       "/dev/ttyUSB0"
#define NET_ADAPTHER    "eth0"

extern void msleep(int m_seconds);
int Gagent_setsocketnonblock(int socketfd);

#endif

