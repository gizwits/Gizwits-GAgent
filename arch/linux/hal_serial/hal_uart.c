#include "hal_uart.h"

/****************************************************************
Function        :   open_serial
Description     :   open serial 
comport         :   serial comport number
bandrate        :   serial bandrate 
nBits           :   serial data Bit
return          :   >0 the serial fd 
                    other fail.
Add by Alex.lin     --2015-03-31
****************************************************************/
int serial_open(char *comport, int bandrate,int nBits,char nEvent,int nStop )
{
  //char uart[20]={0};
  int Tmpfd;
  struct termios option;
  //sprintf(uart,"/dev/%s%d",UART_NAME,comport);
  printf("uart name :%s\r\n",comport );
  Tmpfd = open(comport,O_RDWR|O_NOCTTY|O_NDELAY); //读写方式打开串口

  if(Tmpfd ==-1) //打开失败
    return (-1);    
  if(tcgetattr(Tmpfd,&option)!=0)
    return (-1);
  cfmakeraw(&option); //原始模式
  bzero( &option, sizeof( option ) );
  /********数据位8位****************/
  option.c_cflag |= CLOCAL | CREAD;
  option.c_cflag &= ~CSIZE; 
  switch( nBits )
  {
    case 7:
    option.c_cflag |= CS7;
    break;
    case 8:
    option.c_cflag |= CS8;
    break;
  }
/*************奇偶校验*************/

  switch( nEvent )
  {
    case 'O':
      option.c_cflag |= PARENB;
      option.c_cflag |= PARODD;
      option.c_iflag |= (INPCK | ISTRIP);
    break;
    case 'E':
      option.c_iflag |= (INPCK | ISTRIP);
      option.c_cflag |= PARENB;
      option.c_cflag &= ~PARODD;
    break;
    case 'N':
      option.c_cflag &= ~PARENB;
    break;
  }

/********波特率设置****************/

  switch(bandrate)
  {
    case 1200:
      cfsetispeed(&option,B1200); //波特率设置为9600bps
      cfsetospeed(&option,B1200);
    break;
    case 2400:
      cfsetispeed(&option,B2400); //波特率设置为9600bps
      cfsetospeed(&option,B2400);
    break;
    case 4800:
      cfsetispeed(&option,B4800); //波特率设置为9600bps
      cfsetospeed(&option,B4800);
    break;
    case 9600:
      cfsetispeed(&option,B9600); //波特率设置为9600bps
      cfsetospeed(&option,B9600);
    break;    
    case 19200:
      cfsetispeed(&option,B19200); //波特率设置为9600bps
      cfsetospeed(&option,B19200);
    break;
    case 38400:
      cfsetispeed(&option,B38400); //波特率设置为9600bps
      cfsetospeed(&option,B38400);
    break;
    case 57600:
      cfsetispeed(&option,B57600); //波特率设置为9600bps
      cfsetospeed(&option,B57600);
    break;
    case 115200:
      cfsetispeed(&option,B115200); //波特率设置为9600bps
      cfsetospeed(&option,B115200);
      printf("yes\r\n");
    break;
    default:
      cfsetispeed(&option,B9600); //波特率设置为9600bps
      cfsetospeed(&option,B9600);
    break;
  }

  if( nStop == 1 )
  option.c_cflag &= ~CSTOPB;
  else if ( nStop == 2 )
  option.c_cflag |= CSTOPB;

  option.c_cc[VTIME] = 0;
  option.c_cc[VMIN] = 0;

  tcflush(Tmpfd,TCIFLUSH);

  if(tcsetattr(Tmpfd,TCSANOW,&option)!=0)//设置立即生效
    return (-1);

  return Tmpfd;
}
/****************************************************************
Function        :   serial_read
Description     :   read data form serial fd 
buf             :   data form serial pointer.
buflen          :   data want to read.
return          :   >0 the realy data length form serial 
Add by Alex.lin     --2015-03-31
****************************************************************/
int serial_read( int serial_fd, unsigned char *buf,int buflen )
{
  int datalen=0;

    if( serial_fd<0 )
      return 0;
    datalen = read( serial_fd,buf,buflen );
  return datalen;   
}

/****************************************************************
Function        :   serial_write
Description     :   write data to serial fd 
buf             :   data need to serial pointer.
buflen          :   data want to write.
return          :   >0 the number of bytes written is returned
                    other error.
Add by Alex.lin     --2015-03-31
****************************************************************/
int serial_write( int serial_fd,unsigned char *buf,int buflen )
{
  int datalen=0;

    if( serial_fd<0 )
      return 0;
    datalen = write( serial_fd,buf,buflen);
  return datalen;
}
