#include "gagent.h"
#include "platform.h"
#include "md5.h"
#include "http.h"

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

uint32 GAgent_SaveUpgradFirmware( int offset,uint8 *buf,int len )
{
    int fd;  
    fd = open("ota.bin", O_CREAT | O_RDWR, S_IRWXU);
    if(-1 == fd)
    {
        printf("open file fail\r\n");
        return -1;
    }
    
    lseek(fd , offset , SEEK_SET);
    write(fd, buf, len);
    close(fd);
    return 0;
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
    if( uart_fd<=0 )
        return (-1);
    return uart_fd;
}
void GAgent_LocalDataIOInit( pgcontext pgc )
{
    pgc->rtinfo.local.uart_fd = GAgent_OpenUart( 9600,8,0,0,0 );
    while( pgc->rtinfo.local.uart_fd <=0 )
    {
        pgc->rtinfo.local.uart_fd = GAgent_OpenUart( 9600,8,0,0,0 );
        sleep(1);
    }
    //serial_write( pgc->rtinfo.local.uart_fd,"GAgent Start !!!",strlen("GAgent Start !!!") );
    return ;
}

int16 GAgent_DRV_WiFi_SoftAPModeStart( const int8* ap_name,const int8 *ap_password,int16 wifiStatus )
{
    wifiStatus &=~ WIFI_STATION_CONNECTED;
    wifiStatus |= WIFI_MODE_AP;
    wifiStatus = GAgent_DevCheckWifiStatus( wifiStatus  );    
    return WIFI_MODE_AP;
}
int16 GAgent_DRVWiFi_StationCustomModeStart(int8 *StaSsid,int8 *StaPass,uint16 wifiStatus )
{
    wifiStatus |= WIFI_STATION_CONNECTED;
    wifiStatus = GAgent_DevCheckWifiStatus( wifiStatus  );
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
    uint16 tempStatus=0;
    tempStatus = pgc->rtinfo.GAgentStatus;
    tempStatus = GAgent_DevCheckWifiStatus( tempStatus );
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
uint32 GAgent_OTAByUrl( pgcontext pgc,int32 socketid,int8 *sMD5,int32 *filelen )
{
    int ret;
    uint8 *httpReceiveBuf = NULL;
    int headlen = 0;
    char MD5[16] = {0};
    uint8 md5_calc[16] = {0};
    int offset = 0;
    uint8 *buf = NULL;
    int writelen = 0;
    MD5_CTX ctx;

    httpReceiveBuf = malloc(SOCKET_RECBUFFER_LEN);
    if(httpReceiveBuf == NULL)
    {
        GAgent_Printf(GAGENT_INFO, "[CLOUD]%s malloc fail!len:%d", __func__, SOCKET_RECBUFFER_LEN);
        return RET_FAILED;
    }

    ret = Http_ReadSocket( socketid, httpReceiveBuf, SOCKET_RECBUFFER_LEN );  
    if(ret <=0 ) 
    { 
        free(httpReceiveBuf);
        return RET_FAILED;
    }
    
    ret = Http_Response_Code( httpReceiveBuf );
    if(200 != ret)
    {
        free(httpReceiveBuf);
        return RET_FAILED;
    }
    headlen = Http_HeadLen( httpReceiveBuf );
    *filelen = Http_BodyLen( httpReceiveBuf );
    Http_GetMD5( httpReceiveBuf,MD5,sMD5);
    Http_GetSV( httpReceiveBuf,(char *)pgc->mcu.soft_ver);
  
    offset = 0;
    buf = httpReceiveBuf + headlen;
    writelen = SOCKET_RECBUFFER_LEN - headlen;
    MD5Init(&ctx);
    do
    {
        ret = GAgent_SaveUpgradFirmware( offset, buf, writelen );
        if(ret < 0)
        {
            GAgent_Printf(GAGENT_INFO, "[CLOUD]%s OTA upgrad fail at off:0x%x", __func__, offset);
            free(httpReceiveBuf);
            return -1;
        }
        offset += writelen;
        MD5Update(&ctx, buf, writelen);
        writelen = *filelen - offset;
        if(0 == writelen)
            break;
        if(writelen > SOCKET_RECBUFFER_LEN)
        {
            writelen = SOCKET_RECBUFFER_LEN;
        }
        writelen = Http_ReadSocket( socketid, httpReceiveBuf, writelen );    
        if(writelen <= 0 )
        {
            GAgent_Printf(GAGENT_INFO,"[CLOUD]%s, socket recv ota file fail!recived:0x%x", __func__, offset);
            free(httpReceiveBuf);
            return -1;
        }
        buf = httpReceiveBuf;
    }while(offset < *filelen);
    MD5Final(&ctx, md5_calc);
    if(memcmp(MD5, md5_calc, 16) != 0)
    {
        GAgent_Printf(GAGENT_WARNING,"[CLOUD]md5 fail!");
        free(httpReceiveBuf);
        return RET_FAILED;
    }
    free(httpReceiveBuf);
    return RET_SUCCESS;
}
int32 Http_ReqGetFirmware( int8 *downloadurl,int32 socketid )
{
    static int8 *getBuf = NULL;
    int32 totalLen=0;
    int32 ret=0;
    int8 url[30] = {0};
    int8 host[30] = {0};
    Http_GetHost( downloadurl, host, url );
    getBuf = (int8*)malloc( 200 );
    if(getBuf == NULL)
    {
        return RET_FAILED;
    }
    memset( getBuf,0,200 );
    snprintf( getBuf,200,"%s %s %s%s%s %s%s%s%s",
              "GET",url,"HTTP/1.1",kCRLFNewLine,
              "Host:",host,kCRLFNewLine,
              "Content-Type: application/text",kCRLFLineEnding);
    totalLen =strlen( getBuf );
    ret = send( socketid, getBuf,totalLen,0 );
    free(getBuf); 
//    free(host);
//    free(url);
    getBuf = NULL;
    if(ret<=0 ) 
    {
        return RET_FAILED;
    }
    else
    {
        return RET_SUCCESS;
    }    
}


int32 GAgent_StartUpgrade()
{
 //TODO    
 
 remove("./ota.bin");
 return 0;
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
