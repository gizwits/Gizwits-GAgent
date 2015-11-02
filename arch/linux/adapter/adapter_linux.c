#include "gagent.h"
#include "platform.h"
#include "http.h"
#include "cloud.h"
#include "netevent.h"
#include "gprs.h"

#define GAGENT_CONFIG_FILE "./config/gagent_config.config"
void msleep(int m_seconds)
{ 
    usleep(m_seconds*1000);
}
uint32 GAgent_GetHostByName( int8 *domain, int8 *IPAddress)
{
    struct hostent *hptr;
    char   **pptr;
    char   str[32];
    memset(str, 0x0, sizeof(str));
    hptr = gethostbyname2(domain, AF_INET);
    if (hptr == NULL)
    {
        GAgent_Printf(GAGENT_DEBUG," resean : %s\n", hstrerror(h_errno));
        return 1;
    }
    pptr=hptr->h_addr_list;

    for(; *pptr!=NULL; pptr++)
    {
        inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str));
    }
    inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str));
    GAgent_Printf(GAGENT_INFO,"Server name %s  address: %s", domain, str);
    memcpy(IPAddress, str, 32);
    return 0;
}

int Gagent_setsocketnonblock(int socketfd)
{
#if 0
	int flags = fcntl(socketfd, F_GETFL, 0);
	flags |= O_NONBLOCK;
	return fcntl(socketfd, F_SETFL, flags | O_NONBLOCK);
#else
    return 0;
#endif
}

uint32 GAgent_GetDevTime_MS()
{
/*
    int32           ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds

    return (s*1000)+ms;
    */
    
    struct timeval tv;
         
    gettimeofday(&tv, NULL);
    
    return (uint32)((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000) ;
    
}
uint32 GAgent_GetDevTime_S()
{
    struct timeval tv;
         
    gettimeofday(&tv, NULL);
    return (uint32)(tv.tv_sec) ;
}
/****************************************************************
FunctionName    :   GAgent_DevReset
Description     :   dev exit but not clean the config data                   
pgc             :   global staruc 
return          :   NULL
Add by Alex.lin     --2015-04-18
****************************************************************/
void GAgent_DevReset()
{
    GAgent_Printf( GAGENT_CRITICAL,"Please restart GAgent !!!\r\n");
    exit(0);
}
void GAgent_DevInit( pgcontext pgc )
{
    signal(SIGPIPE, SIG_IGN);
}
int8 GAgent_DevGetMacAddress( uint8* szmac )
{
    int sock_mac;
    struct ifreq ifr_mac;
    uint8 mac[6]={0};
    
    if(szmac == NULL)
    {
        return -1;
    }
    
    sock_mac = socket( AF_INET, SOCK_STREAM, 0 );
    if( sock_mac == -1)
    {
        perror("create socket falise...mac/n");
        return (-1); 
    }
    memset(&ifr_mac,0,sizeof(ifr_mac));
    
    /*get mac address*/

    strncpy(ifr_mac.ifr_name, NET_ADAPTHER , sizeof(ifr_mac.ifr_name)-1);
    if( (ioctl( sock_mac, SIOCGIFHWADDR, &ifr_mac)) < 0)
    {
        perror("mac ioctl error:");
        return (-1);
    }

    mac[0] = ifr_mac.ifr_hwaddr.sa_data[0];
    mac[1] = ifr_mac.ifr_hwaddr.sa_data[1];
    mac[2] = ifr_mac.ifr_hwaddr.sa_data[2];
    mac[3] = ifr_mac.ifr_hwaddr.sa_data[3];
    mac[4] = ifr_mac.ifr_hwaddr.sa_data[4];
    mac[5] = ifr_mac.ifr_hwaddr.sa_data[5];
    
    /*
    mac[0] = 0x00;
    mac[1] = 0x01;
    mac[2] = 0x02;
    mac[3] = 0x03;
    mac[4] = 0x04;
    mac[5] = 0x05;
    */
    sprintf((char *)szmac,"%02X%02X%02X%02X%02X%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    close( sock_mac );
    return 0;
}
uint32 GAgent_DevGetConfigData( gconfig *pConfig )
{
    int ret=0;
    int fd = open(GAGENT_CONFIG_FILE, O_RDONLY );
    if(-1 == fd)
    {
        perror("read open file fail");
        return -1;
    }
    ret = read(fd, pConfig, sizeof(gconfig));
    GAgent_Printf( GAGENT_INFO,"%s %d ",__FUNCTION__,ret );
    close(fd);
    return 0;
}
uint32 GAgent_DevSaveConfigData( gconfig *pConfig )
{
    int32 ret=0;
    int fd = open(GAGENT_CONFIG_FILE, O_CREAT | O_RDWR, S_IRWXU);
    if(-1 == fd)
    {
        printf(" write open file fail\r\n");
        return -1;
    }
    
    ret = write(fd, pConfig, sizeof(gconfig));
    GAgent_Printf( GAGENT_INFO,"%s ret=%d",__FUNCTION__,ret  );
    close(fd);
    return 0;
}

/********************************************************
 Function : 兼容旧版本考虑，获取旧版本的配置文件
            2015-04以后不需要实现该函数，调用GAgent_DevGetConfigData即可
 Return :  > 0 读取到的字节数
           = 0 无可读取数据或读到文件尾
           < 0 异常
 ********************************************************/
int8 Dev_GAgentGetOldConfigData( GAgent_OldCONFIG_S *p_oldgc )
{
    int8 ret = 0;
    int fd = open(GAGENT_CONFIG_FILE, O_RDONLY );
    if(-1 == fd)
    {
        perror("read open file fail");
        return -1;
    }
    ret = read(fd, p_oldgc, sizeof(GAgent_OldCONFIG_S));
    close(fd);

    return ret;
}

uint32 GAgent_SaveFile( int offset,uint8 *buf,int len )
{
    int fd;  
    fd = open("ota.bin", O_CREAT | O_RDWR, S_IRWXU);
    if(-1 == fd)
    {
        printf("open file fail\r\n");
        return RET_FAILED;
    }
    lseek(fd , offset , SEEK_SET);
    write(fd, buf, len);
    close(fd);
    return RET_SUCCESS;
}
uint32 GAgent_ReadFile( uint32 offset,int8* buf,uint32 len )
{
    int fd;
    int ret;
    if((fd = open("ota.bin",O_RDONLY))==-1)
    {
    	GAgent_Printf(GAGENT_WARNING,"Can't open the ota.bin file\n");
    	return RET_FAILED;
    }
    lseek( fd, offset, SEEK_SET );
    ret = read( fd, buf, len );
    close(fd);
    return ret;
}

uint32 GAgent_SaveUpgradFirmware( int offset,uint8 *buf,int len )
{
    return GAgent_SaveFile( offset, buf, len );
}

void WifiStatusHandler(int event)
{

}
/****************************************************************
*   function    :   socket connect 
*   flag        :   1 block 
                    0 no block 
    return      :   0> connect ok socketid 
                :   other fail.
    add by Alex.lin  --2015-05-13
****************************************************************/
int32 GAgent_connect( int32 iSocketId, uint16 port,
                        int8 *ServerIpAddr,int8 flag)
{
    int8 ret=0;
    
    struct sockaddr_in Msocket_address;
    GAgent_Printf(GAGENT_INFO,"do connect ip:%s port=%d",ServerIpAddr,port );

    Msocket_address.sin_family = AF_INET;
    Msocket_address.sin_port= htons(port);
    Msocket_address.sin_addr.s_addr = inet_addr(ServerIpAddr);
/*
    unsigned long ul = 1;
    ioctl(iSocketId, FIONBIO, &ul); //设置为非阻塞模式
*/
    ret = connect(iSocketId, (struct sockaddr *)&Msocket_address, sizeof( struct sockaddr_in));  
/*  
    if( 0==ret )
    {
        GAgent_Printf( GAGENT_INFO,"immediately connect ok !");
    }
    else
    {
        if( errno == EINPROGRESS )
        {
            int times = 0;
            fd_set rfds, wfds;
            struct timeval tv;
            int flags;
            tv.tv_sec = 10;   
            tv.tv_usec = 0;                 
            FD_ZERO(&rfds);  
            FD_ZERO(&wfds);  
            FD_SET(iSocketId, &rfds);  
            FD_SET(iSocketId, &wfds);    
            int selres = select(iSocketId + 1, &rfds, &wfds, NULL, &tv);
                switch( selres )
                {
                    case -1:
                        ret=-1;
                        break;
                    case 0:
                        ret = -1;
                        break;
                    default:
                        if (FD_ISSET(iSocketId, &rfds) || FD_ISSET(iSocketId, &wfds))
                        {
                            connect(iSocketId, (struct sockaddr *)&Msocket_address, sizeof( struct sockaddr_in));   
                            int err = errno;  
                            if  (err == EISCONN)  
                            {  
                                GAgent_Printf( GAGENT_INFO,"1 connect finished .\n");  
                                ret = 0;  
                            }
                            else
                            {
                                ret=-1;
                            } 
                            char buff[2];  
                            if (read(iSocketId, buff, 0) < 0)  
                            {  
                                GAgent_Printf( GAGENT_INFO,"connect failed. errno = %d\n", errno);  
                                ret = errno;  
                            }  
                            else  
                            {  
                                GAgent_Printf( GAGENT_INFO,"2 connect finished.\n");  
                                ret = 0;  
                            }  
                        }
                }
        }
    }
    //
    ioctl(iSocketId, FIONBIO, &ul); //设置为阻塞模式
    //
    */
    if( ret==0)
        return iSocketId;
    else 
    return  -1;
}
/****************************************************************
FunctionName    :   GAgent_DRVBootConfigWiFiMode.
Description     :   获取上电前设置的WiFi运行模式,此函数对需要上电需要
                    确定运行模式的模块有用。对于可以热切换模式的平台，
                    此处返回0.
return          :   1-boot预先设置为STA运行模式
                    2-boot预先设置为AP运行模式
                    3-boot预先设置为STA+AP模式运行
                    >3 保留值。
Add by Alex.lin     --2015-06-01                    
****************************************************************/
int8 GAgent_DRVBootConfigWiFiMode( void )
{
    return 0;
}
/****************************************************************
FunctionName    :   GAgent_DRVGetWiFiMode.
Description     :   通过判断pgc->gc.flag  |= XPG_CFG_FLAG_CONNECTED，
                    是否置位来判断GAgent是否要启用STA 或 AP模式.
pgc             :   全局变量.
return          :   1-boot预先设置为STA运行模式
                    2-boot预先设置为AP运行模式
                    3-boot预先设置为STA+AP模式运行
                    >3 保留值。
Add by Alex.lin     --2015-06-01                    
****************************************************************/
int8 GAgent_DRVGetWiFiMode( pgcontext pgc )
{ 
    //linux x86默认运行STA模式.
    pgc->gc.flag |=XPG_CFG_FLAG_CONNECTED;
    return 1;
}
//return the new wifimode 
int8 GAgent_DRVSetWiFiStartMode( pgcontext pgc,uint32 mode )
{
    return ( pgc->gc.flag +=mode );
}
void DRV_ConAuxPrint( char *buffer, int len, int level )
{ 
    buffer[len]='\0';
    printf("%s", buffer);
    fflush(stdout);
}
int32 GAgent_OpenUart( int32 BaudRate,int8 number,int8 parity,int8 stopBits,int8 flowControl )
{
    int32 uart_fd=0;
    uart_fd = serial_open( UART_NAME,BaudRate,number,'N',stopBits );
    if( uart_fd < 0 )
        return (-1);
    return uart_fd;
}
void GAgent_LocalDataIOInit( pgcontext pgc )
{
    pgc->rtinfo.local.uart_fd = GAgent_OpenUart( 9600,8,0,0,0 );
    while( pgc->rtinfo.local.uart_fd < 0 )
    {
        pgc->rtinfo.local.uart_fd = GAgent_OpenUart( 9600,8,0,0,0 );
        sleep(1);
    }
    //serial_write( pgc->rtinfo.local.uart_fd,"GAgent Start !!!",strlen("GAgent Start !!!") );
    return ;
}

int16 GAgent_DRV_WiFi_SoftAPModeStart( const int8* ap_name,const int8 *ap_password,int16 wifiStatus )
{
    GAgent_DevCheckWifiStatus( WIFI_STATION_CONNECTED,0 );
    GAgent_DevCheckWifiStatus( WIFI_MODE_AP,1 );
    return WIFI_MODE_AP;
}
int16 GAgent_DRVWiFi_StationCustomModeStart(int8 *StaSsid,int8 *StaPass,uint16 wifiStatus )
{
    GAgent_DevCheckWifiStatus( WIFI_STATION_CONNECTED,1 );
    GAgent_DevCheckWifiStatus( WIFI_MODE_STATION,1 );
    GAgent_Printf( GAGENT_INFO," Station ssid:%s StaPass:%s",StaSsid,StaPass );
    return WIFI_STATION_CONNECTED;
    //return 0;
}
int16 GAgent_DRVWiFi_StationDisconnect()
{
    return 0;
}
void GAgent_DRVWiFi_APModeStop( pgcontext pgc )
{
/*  uint16 tempStatus=0;
    tempStatus = pgc->rtinfo.GAgentStatus;
    tempStatus = GAgent_DevCheckWifiStatus( tempStatus );*/
    GAgent_DevCheckWifiStatus( WIFI_MODE_AP,0 );
    return ;
}
void GAgent_DRVWiFiPowerScan( pgcontext pgc )
{

}
int8 GAgent_DRVWiFiPowerScanResult( pgcontext pgc )
{
    return 100;
}
void GAgent_DevTick()
{
    fflush(stdout);
}
void GAgent_DevLED_Red( uint8 onoff )
{

}
void GAgent_DevLED_Green( uint8 onoff )
{

}
int Socket_sendto(int sockfd, u8 *data, int len, void *addr, int addr_size)
{
    return sendto(sockfd, data, len, 0, (const struct sockaddr*)addr, addr_size);
}
int Socket_accept(int sockfd, void *addr, socklen_t *addr_size)
{
    return accept(sockfd, (struct sockaddr*)addr, addr_size);
}
int Socket_recvfrom(int sockfd, u8 *buffer, int len, void *addr, socklen_t *addr_size)
{
    return recvfrom(sockfd, buffer, len, 0, (struct sockaddr *)addr, addr_size);
}
int connect_mqtt_socket(int iSocketId, struct sockaddr_t *Msocket_address, unsigned short port, char *MqttServerIpAddr)
{
    return 0;
}
int32 GAgent_select(int32 nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds,int32 sec,int32 usec )
{
    struct timeval t;
    
    t.tv_sec = sec;// 秒
    t.tv_usec = usec;// 微秒
    return select( nfds,readfds,writefds,exceptfds,&t );
}
void GAgent_OpenAirlink( int32 timeout_s )
{
    //TODO
    return ;
}
void GAgent_AirlinkResult( pgcontext pgc )
{
    return ;
}
void GAgent_DRVWiFiStartScan( )
{

}
void GAgent_DRVWiFiStopScan( )
{

}
NetHostList_str *GAgentDRVWiFiScanResult( NetHostList_str *aplist )
{
    //需要再平台相关的扫描结果调用该函数。
    //把平台相关扫描函数的结果拷贝到NetHostList_str这个结构体上。

    /* 申请内存，用于保存热点列表 */
    return NULL;
}

int32 GAgent_StartUpgrade()
{
 //TODO    
 
 remove("./ota.bin");
 return 0;
}
/****************************************************************
FunctionName    :   GAgent_ReadOTAFile
Description     :   read file from flash
return          :   =0 : read file complete  
                      >0 : read success
                      <0 : read failed
****************************************************************/
int32 GAgent_ReadOTAFile( uint32 offset,int8* buf,uint32 len )
{
    return  GAgent_ReadFile( offset, buf, len );
}
int32 GAgent_WIFIOTAByUrl( pgcontext pgc,int8 *szdownloadUrl )
{
     int32 ret = 0;
    int32 http_socketid = -1;
    int8 OTA_IP[32]={0};
    int8 *url = NULL;
    int8 *host = NULL;   
    if(RET_FAILED == Http_GetHost( szdownloadUrl, &host, &url ) )
    {
        return RET_FAILED;
    }
    ret = GAgent_GetHostByName( host,OTA_IP );
    if( ret!=0)
    {
        GAgent_Printf( GAGENT_INFO,"file:%s function:%s line:%d ",__FILE__,__FUNCTION__,__LINE__ );
        return RET_FAILED;
    }
    
    http_socketid = Cloud_InitSocket( http_socketid, OTA_IP, 80, 0 );

    
    ret = Http_ReqGetFirmware( url, host, http_socketid );
    if( RET_SUCCESS == ret )
    {
        ret = Http_ResGetFirmware( pgc, http_socketid );
        close(http_socketid);
        return ret;
    }
    else
    {
        GAgent_Printf(GAGENT_WARNING,"req get Firmware failed!\n");
        close(http_socketid);
        return RET_FAILED;
    }
}
uint32 GAgent_getIpaddr(uint8 *ipaddr)
{
    return RET_SUCCESS;
}
uint32 GAgent_sendWifiInfo( pgcontext pgc )
{
    int32 pos = 0;
    uint8 ip[16] = {0};
    if(0 != GAgent_getIpaddr(ip))
    {
        GAgent_Printf(GAGENT_WARNING,"GAgent get ip failed!");
    }  
    
    /* ModuleType */
    pgc->rtinfo.Txbuf->ppayload[0] = 0x01;
    pos += 1;

    /* MCU_PROTOCOLVER */
    memcpy( pgc->rtinfo.Txbuf->ppayload+pos, pgc->mcu.protocol_ver, MCU_PROTOCOLVER_LEN );
    pos += MCU_PROTOCOLVER_LEN;

    /* HARDVER */
    memcpy( pgc->rtinfo.Txbuf->ppayload+pos, WIFI_HARDVER, 8 );
    pos += 8;

    /* SOFTVAR */
    memcpy( pgc->rtinfo.Txbuf->ppayload+pos, WIFI_SOFTVAR, 8 );
    pos += 8;

    /* MAC */
    memset( pgc->rtinfo.Txbuf->ppayload+pos, 0 , 16 );
    memcpy( pgc->rtinfo.Txbuf->ppayload+pos, pgc->minfo.szmac, 16 );
    pos += 16;
  
    /* IP */ 
    memset( pgc->rtinfo.Txbuf->ppayload+pos, 0 , 16 );
    memcpy( pgc->rtinfo.Txbuf->ppayload+pos, ip, strlen(ip) );
    pos += 16;

    /* MCU_MCUATTR */
    memcpy( pgc->rtinfo.Txbuf->ppayload+pos, pgc->mcu.mcu_attr, MCU_MCUATTR_LEN);
    pos += MCU_MCUATTR_LEN;

    pgc->rtinfo.Txbuf->pend = pgc->rtinfo.Txbuf->ppayload + pos;
    return RET_SUCCESS;   
}

/****************************************************************
        FunctionName  :  GAgent_sendmoduleinfo.
        Description      :  send module info,according to the actual situation to choose one .
        return             :  0 successful other fail.
****************************************************************/
uint32 GAgent_sendmoduleinfo( pgcontext pgc )
{
//    return GAgent_sendWifiInfo(pgc);
    
    return GAgent_sendGprsInfo(pgc);
}

/*
void Socket_CreateTCPServer(int tcp_port)
{
    return;
}

void Socket_CreateUDPServer(int udp_port)
{
    return;
}

void Socket_CreateUDPBroadCastServer( int udp_port )
{
    return;
}
*/
