#ifndef _IOF_ARCH_H_
#define _IOF_ARCH_H_
#include "gagent_typedef.h"
#include "platform.h"
//return wifistatus. 
void CoreInit( void );
void GAgent_DevReset( void );
void GAgent_DevInit( pgcontext pgc );
void GAgent_DevTick( void );
uint32 GAgent_GetDevTime_MS( void );
uint32 GAgent_GetDevTime_S( void );
int8 GAgent_DevGetMacAddress( uint8* szmac );
void GAgent_DevLED_Red( uint8 onoff );
void GAgent_DevLED_Green( uint8 onoff );
uint32 GAgent_sendmoduleinfo( pgcontext pgc );

int8 Dev_GAgentGetOldConfigData( GAgent_OldCONFIG_S *p_oldgc );
uint32 GAgent_DevGetConfigData( gconfig *pConfig );
uint32 GAgent_DevSaveConfigData( gconfig *pConfig);
void GAgent_LocalDataIOInit( pgcontext pgc );
int32 GAgent_ReadOTAFile( uint32 offset,int8* buf,uint32 len );

/*********Net event function************/
void GAgent_OpenAirlink( int32 timeout_s );
void GAgent_AirlinkResult( pgcontext pgc );
int8 GAgent_DRVBootConfigWiFiMode( void );
int8 GAgent_DRVGetWiFiStartMode( pgcontext pgc );
int8 GAgent_DRVSetWiFiStartMode( pgcontext pgc,uint32 mode );
int16 GAgent_DRV_WiFi_SoftAPModeStart( const int8* ap_name,const int8 *ap_password,int16 wifiStatus );
int16 GAgent_DRVWiFi_StationCustomModeStart(int8 *StaSsid,int8 *StaPass,uint16 wifiStatus );
int16 GAgent_DRVWiFi_StationDisconnect( void );
void GAgent_DRVWiFi_APModeStop( pgcontext pgc );
void GAgent_DRVWiFiStartScan( void );
void GAgent_DRVWiFiStopScan( void );
NetHostList_str *GAgentDRVWiFiScanResult( NetHostList_str *aplist );
void GAgent_DRVWiFiPowerScan( pgcontext pgc );
int8 GAgent_DRVWiFiPowerScanResult( pgcontext pgc );
int32 GAgent_StartUpgrade( void );
uint32 Http_ResGetFirmware( pgcontext pgc,int32 socketid );
int32 GAgent_WIFIOTAByUrl( pgcontext pgc,int8 *szdownloadUrl );

/*********Net socket function************/
uint32 GAgent_GetHostByName( int8 *domain, int8 *IPAddress );
int32  GAgent_accept( int32 sockfd );
int32  GAgent_listen( int32 sockfd, int32 backlog );
uint32 GAgent_sendto( int32  sockfd,  const  void  *buf, int32 len,  int32 flags );
int32 GAgent_select(int32 nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds,int32 sec,int32 usec );
int32 GAgent_CreateTcpServer( uint16 tcp_port );
int32 GAgent_CreateUDPServer( uint16 udp_port );
int32 GAgent_CreateUDPBroadCastServer( uint16 udpbroadcast_port, struct sockaddr_t *sockaddr);

                
/****************************************************************
*   functionName    :   GAgent_connect
*   flag            :   1 block 
*                       0 no block 
*   return          :   0> successful socketid
                         other fail.
****************************************************************/
int32 GAgent_connect( int32 iSocketId, uint16 port,
                        int8 *ServerIpAddr,int8 flag);



void DRV_ConAuxPrint( char *buffer, int len, int level );


/****************************************************************
        FunctionName        :   GAgent_OpenUart.
        Description         :   Open uart.
        BaudRate            :   The baud rate.
        number              :   The number of data bits.
        parity              :   The parity(0: none, 1:odd, 2:evn).
        StopBits            :   The number of stop bits .
        FlowControl         :   support flow control is 1
        return              :   uart fd.
****************************************************************/
int32 GAgent_OpenUart( int32 BaudRate,int8 number,int8 parity,int8 stopBits,int8 flowControl );
#endif
