#include "netevent.h"
#include "gagent.h"
#include "gagent_md5.h"

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
int8 GAgent_DRVWiFiPower( pgcontext pgc )
{
    
    return 100;
}
uint32 Http_ResGetFirmware( pgcontext pgc,int32 socketid )
{
    int ret;
    uint8 *httpReceiveBuf = NULL;
    int headlen = 0;
    char MD5[16] = {0};
    uint8 md5_calc[16] = {0};
    int offset = 0;
    uint8 *buf = NULL;
    int writelen = 0;
    uint8 filename[100] = {0};
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
    pgc->rtinfo.filelen = Http_BodyLen( httpReceiveBuf );
    //pgc->rtinfo.MD5 = (char *)malloc(32+1);
    // if( pgc->rtinfo.MD5 == NULL )
    // {
    //     return RET_FAILED;
    // }
    Http_GetMD5( httpReceiveBuf,MD5,pgc->mcu.MD5 );
    Http_GetSV( httpReceiveBuf,(char *)pgc->mcu.soft_ver);
    pgc->mcu.mcu_firmware_type = Http_GetFileType(httpReceiveBuf);
    offset = 0;
    buf = httpReceiveBuf + headlen;
    writelen = SOCKET_RECBUFFER_LEN - headlen;
    GAgent_MD5Init(&ctx);
    do
    {
        ret = GAgent_SaveUpgradFirmware( offset, buf, writelen );
        if(ret < 0)
        {
            GAgent_Printf(GAGENT_INFO, "[CLOUD]%s OTA upgrad fail at off:0x%x", __func__, offset);
            free(httpReceiveBuf);
            return RET_FAILED;
        }
        offset += writelen;
        GAgent_MD5Update(&ctx, buf, writelen);
        writelen = pgc->rtinfo.filelen - offset;
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
            return RET_FAILED;
        }
        buf = httpReceiveBuf;
    }while(offset < pgc->rtinfo.filelen);
    GAgent_MD5Final(&ctx, md5_calc);
    if(memcmp(MD5, md5_calc, 16) != 0)
    {
        GAgent_Printf(GAGENT_WARNING,"[CLOUD]md5 fail!");
        free(httpReceiveBuf);
        return RET_FAILED;
    }
    free(httpReceiveBuf);
    return RET_SUCCESS;
}
