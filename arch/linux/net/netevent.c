#include "netevent.h"
#include "gagent.h"
/****************************************************************
Function    :   GAgent_DevCheckWifiStatus
Description :   check the wifi status and will set the wifi status
                and return it.
bit0        :   in soft ap mode     1 is on ,0 is off.
bit1        :   in station mode     1 is on ,0 is off.
bit2        :   in onboarding mode  1 is on ,0 is off.
...
bit4        :   if is connected to WiFi router,1 is on,0 is off.
bit6-bit8   :   only is meaningful if bit=1,this 3-bits integer 
                indicates the WiFi signal strength of the connected
                WiFi router. Range is from 0 to 7 ,0 is low and 
                7 is high.
wifistatus  :   wifi current status.
return      :   the new wifi status.
Add by Alex.lin     --2015-04-17.
****************************************************************/
uint16 GAgent_DevCheckWifiStatus( uint16 wifistatus  )
{

    return wifistatus;
}
/****************************************************************
Function    :   GAgent_CreateTcpServer
Description :   creat TCP server.
tcp_port    :   server port.
return      :   0> the socket id .
                other error.
Add by Alex.lin     --2015-04-24.
****************************************************************/
int32 GAgent_CreateTcpServer( uint16 tcp_port )
{
    struct sockaddr_t addr;
    int32 bufferSize=0;
    int32 serversocketid=0;

    serversocketid = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if( serversocketid < 0 )
    {
        serversocketid = INVALID_SOCKET;
        GAgent_Printf(GAGENT_ERROR, "TCPServer socket create error");
        return RET_FAILED;
    }
    bufferSize = SOCKET_TCPSOCKET_BUFFERSIZE;
    setsockopt( serversocketid, SOL_SOCKET, SO_RCVBUF, &bufferSize, 4 );
    setsockopt( serversocketid, SOL_SOCKET, SO_SNDBUF, &bufferSize, 4 );
    memset(&addr, 0x0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(tcp_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if( bind( serversocketid, (struct sockaddr *)&addr, sizeof(addr)) != 0 )
    {
        GAgent_Printf(GAGENT_ERROR, "TCPSrever socket bind error");
        close(serversocketid);
        serversocketid = INVALID_SOCKET;
        return RET_FAILED;
    }

    if(listen( serversocketid, LAN_TCPCLIENT_MAX ) != 0 )
    {
        GAgent_Printf( GAGENT_ERROR, "TCPServer socket listen error!");
        close( serversocketid );
        serversocketid = INVALID_SOCKET;
        return RET_FAILED;
    }
    return serversocketid;
}

int32 GAgent_CreateUDPServer( uint16 udp_port )
{
    int32 serversocketid = 0;
    struct sockaddr_t addr;
    
    serversocketid = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if(serversocketid < 0)
    {
        GAgent_Printf(GAGENT_ERROR, "UDPServer socket create error");
        serversocketid = INVALID_SOCKET;
        return RET_FAILED;
    }
    
    if(Gagent_setsocketnonblock(serversocketid) != 0)
    {
        GAgent_Printf(GAGENT_ERROR,"UDP Server Gagent_setsocketnonblock fail.");
    }
    memset(&addr, 0x0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(udp_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if( bind( serversocketid, (struct sockaddr *)&addr, sizeof(addr)) != 0 )
    {
        GAgent_Printf(GAGENT_ERROR, "UDPServer socket bind error");
        close(serversocketid);
        serversocketid = INVALID_SOCKET;
        return RET_FAILED;
    }

    GAgent_Printf(GAGENT_DEBUG,"UDP Server socketid:%d on port:%d", serversocketid, udp_port);
    return serversocketid;
}
int32 GAgent_CreateUDPBroadCastServer( uint16 udpbroadcast_port, struct sockaddr_t *sockaddr)
{
    int udpbufsize=2;
    int32 serversocketid = 0;
    struct sockaddr_t addr;
    memset( &addr, 0, sizeof(addr) );

    serversocketid = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if(serversocketid < 0) 
    {
        GAgent_Printf(GAGENT_DEBUG, "UDP BC socket create error");
        serversocketid = INVALID_SOCKET;
        return RET_FAILED;
    }

    if(Gagent_setsocketnonblock(serversocketid) != 0)
    {
        GAgent_Printf(GAGENT_DEBUG,"UDP BC Server Gagent_setsocketnonblock fail.");
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(udpbroadcast_port);
    addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    if( setsockopt(serversocketid, SOL_SOCKET, SO_BROADCAST, &udpbufsize,sizeof(int)) != 0 )
    {
        GAgent_Printf(GAGENT_DEBUG,"UDP BC Server setsockopt error!");
    }
    if(bind(serversocketid, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        GAgent_Printf(GAGENT_DEBUG,"UDP BC Server bind error!");
        close(serversocketid);
        serversocketid = INVALID_SOCKET;
    }
    GAgent_Printf(GAGENT_DEBUG,"UDP BC Server socketid:%d on port:%d", serversocketid, udpbroadcast_port);
    *sockaddr = addr;
    return serversocketid;
}

