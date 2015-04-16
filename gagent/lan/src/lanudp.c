#include "gagent.h"
#include "lan.h"
#include "lanudp.h"

/***********************************************
*
*   UdpFlag:    0 respond udp discover 
*               1 wifi send udp broadcast
*
***********************************************/
static void Build_BroadCastPacket(  pgcontext pgc, int UdpFlag,u8* Udp_Broadcast )
{
    int i;    
    int TempFirmwareVerLen=8;
    uint8 strMacByte[3] = {0};
    int len;

    //protocolver
    *(uint32 *)Udp_Broadcast = htonl(GAGENT_PROTOCOL_VERSION);
        
    //varlen =flag(1b)+cmd(2b)+didlen(2b)+did(didLen)+maclen(2b)+mac+firwareverLen(2b)+firwarever+2+productkeyLen
    Udp_Broadcast[4] =1+2+2+22+2+6+2+TempFirmwareVerLen+2+32; 
    //flag
    Udp_Broadcast[5] = 0x00;
    //cmd 
    if( UdpFlag==1 ) 
    {
        Udp_Broadcast[6] = 0x00; 
        Udp_Broadcast[7] = 0x05;
    }
    if( UdpFlag==0 ) 
    {
        Udp_Broadcast[6] = 0x00; 
        Udp_Broadcast[7] = 0x04;
    }
    // didlen
    Udp_Broadcast[8]=0x00; 
    Udp_Broadcast[9]=22; 
    //did
    for(i=0;i<22;i++)
    {
        Udp_Broadcast[10+i]=pgc->gc.DID[i];
    }                
    //maclen
    Udp_Broadcast[9+Udp_Broadcast[9]+1]=0x00;
    Udp_Broadcast[9+Udp_Broadcast[9]+2]=0x06;//macLEN
    //mac    
    strMacByte[2] = 0;
    for(i=0;i<6;i++)
    {
        strMacByte[0] = pgc->minfo.szmac[i*2];
        strMacByte[1] = pgc->minfo.szmac[i*2 + 1];
        Udp_Broadcast[9+Udp_Broadcast[9]+3+i] = strtoul(strMacByte, NULL, 16);
    }

    //firmwarelen
    Udp_Broadcast[9+Udp_Broadcast[9]+2+7]=0x00;
    Udp_Broadcast[9+Udp_Broadcast[9]+2+8]=TempFirmwareVerLen;//firmwareVerLen    
    //firmware
    memcpy( &Udp_Broadcast[9+Udp_Broadcast[9]+2+9],pgc->gc.FirmwareVer,TempFirmwareVerLen );
    len = 9+Udp_Broadcast[9]+2+8+TempFirmwareVerLen+1;
    //productkeylen
    Udp_Broadcast[len]=0x00;
    Udp_Broadcast[len+1]=32;
    len=len+2;
    memcpy( &Udp_Broadcast[len],pgc->mcu.product_key,32 );
    len=len+22;
    //MCU_SendData(Udp_Broadcast,len);
}

/****************************************************************
        FunctionName        :   GAgent_LANInit.
        Description         :   init clients socket and create tcp/udp server.
        Add by Will.zhou     --2015-03-10
****************************************************************/
void Lan_CreateUDPServer(int32 *pFd, int udp_port)
{
    struct sockaddr_t addr;

    if(NULL == pFd)
    {
        GAgent_Printf(GAGENT_ERROR, "UDPServer pFd NULL!");
        return ;
    }
    
    if (*pFd == -1)
    {
        *pFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(*pFd < 0)
        {
            GAgent_Printf(GAGENT_ERROR, "UDPServer socket create error,errno:%d", errno);
            *pFd = -1;
            return ;
        }
        
        if(Gagent_setsocketnonblock(*pFd) != 0)
        {
            GAgent_Printf(GAGENT_ERROR,"UDP Server Gagent_setsocketnonblock fail.errno:%d", errno);
        }
        memset(&addr, 0x0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(udp_port);
        addr.sin_addr.s_addr = INADDR_ANY;
        if(bind(*pFd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
        {
            GAgent_Printf(GAGENT_ERROR, "UDPServer socket bind error,errno:%d", errno);
            close(*pFd);
            *pFd = -1;
            return ;
        }

    }
    
    GAgent_Printf(GAGENT_DEBUG,"UDP Server socketid:%d on port:%d", *pFd, udp_port);
    return;
}

/****************************************************************
        FunctionName        :   Lan_CreateUDPBroadCastServer.
        Description         :   create udp BroadCastServer.
        Add by Will.zhou     --2015-03-10
****************************************************************/
void Lan_CreateUDPBroadCastServer(int32 *pFd, int udp_port )
{
    int udpbufsize=2;
    struct sockaddr_t addr;

    if( *pFd == -1 )
    {

        *pFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if(*pFd < 0)
        {
            GAgent_Printf(GAGENT_DEBUG, "UDP BC socket create error,errno:%d", errno);
            *pFd = -1;
            return ;
        }

        if(Gagent_setsocketnonblock(*pFd) != 0)
        {
            GAgent_Printf(GAGENT_DEBUG,"UDP BC Server Gagent_setsocketnonblock fail.errno:%d", errno);
        }

        addr.sin_family = AF_INET;
        addr.sin_port=htons(udp_port);
        addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);

        if(setsockopt(*pFd, SOL_SOCKET, SO_BROADCAST, &udpbufsize,sizeof(int)) != 0)
        {
            GAgent_Printf(GAGENT_DEBUG,"UDP BC Server setsockopt error,errno:%d", errno);
            //return;
        }
        if(bind(*pFd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
        {
            GAgent_Printf(GAGENT_DEBUG,"UDP BC Server bind error,errno:%d", errno);
            close(*pFd);
            *pFd = -1;
            return;
        }

    }
    GAgent_Printf(GAGENT_DEBUG,"UDP BC Server socketid:%d on port:%d", *pFd, udp_port);
    return;
}

void Lan_udpDataHandle(pgcontext pgc, ppacket prxBuf, ppacket ptxBuf, int len)
{
    struct sockaddr_t addr;
    int addrLen = sizeof(struct sockaddr_t);
    int32 recLen;
    
    if(FD_ISSET(pgc->ls.udpServerFd, &(pgc->rtinfo.readfd)))
    {
        resetPacket(prxBuf);
        recLen = Socket_recvfrom(pgc->ls.udpServerFd, prxBuf->phead, len,
            &addr, &addrLen);

        Lan_dispatchUdpData(pgc, &addr, prxBuf, ptxBuf, recLen);
    }
}

/****************************************************************
        FunctionName        :   Lan_dispatchUdpData.
        Description         :   dispatchUdpData.
        Add by Will.zhou     --2015-03-10
****************************************************************/
static void Lan_dispatchUdpData(pgcontext pgc, struct sockaddr_t *paddr,
            ppacket prxBuf, ppacket ptxBuf, int32 recLen)
{
    int32 packetLen;
    int32 bytesOfLen;
    int32 dataLen;
    uint16 cmd;
    int32 ret;
    uint8 *buf;

    buf = prxBuf->phead;
    bytesOfLen = mqtt_num_rem_len_bytes(buf + 4);
    dataLen = mqtt_parse_rem_len(buf + 3);

    packetLen = LAN_PROTOCOL_HEAD_LEN + bytesOfLen + dataLen;
    
    if(packetLen != recLen)
    {
        GAgent_Printf(GAGENT_WARNING, "Invalid UDP packet length");
        return;
    }

    resetPacket(ptxBuf);
    cmd = *(uint16 *)(buf + bytesOfLen + LAN_PROTOCOL_HEAD_LEN + LAN_PROTOCOL_FLAG_LEN);
    cmd = ntohs(cmd);
    switch(cmd)
    {
        case GAGENT_LAN_CMD_ONDISCOVER:
            LAN_onDiscoverAck(pgc, ptxBuf->phead, paddr);
            break;
        case GAGENT_LAN_CMD_ONBOARDING:
            Lan_udpOnBoarding(pgc, ptxBuf->phead);
            Socket_sendto(pgc->ls.udpServerFd, ptxBuf->phead, 8, paddr, sizeof(struct sockaddr_t));
            break;

        default:
            break;
    }
}

static void Lan_udpOnBoarding(pgcontext pgc, u8 *buf)
{
    unsigned char* passwdlength = NULL;
    
    passwdlength = buf+(10+buf[9]+1);
    
    memset(pgc->gc.wifi_ssid,0, SSID_LEN);
    memset(pgc->gc.wifi_key,0,WIFIKEY_LEN);

    strncpy(pgc->gc.wifi_ssid, buf+10,buf[9]);
    strncpy(pgc->gc.wifi_key,buf+(10+buf[9]+2),*passwdlength);

    GAgent_DevSaveConfigData(&(pgc->gc));

    return ;
}

/****************************************************************
        FunctionName        :   LAN_onDiscoverAck.
        Description         :   reponsing client discover ack.
        Add by Will.zhou     --2015-03-10
****************************************************************/
static void LAN_onDiscoverAck(pgcontext pgc, uint8 *ptxBuf, struct sockaddr_t *paddr)
{
    int32 len;
    int32 ret;

    Build_BroadCastPacket( pgc, 0, ptxBuf );
    len = ptxBuf[4] + 5;
    ret = Socket_sendto(pgc->ls.udpServerFd, ptxBuf, len, paddr, sizeof(struct sockaddr_t));
    if(ret != len)
    {
        GAgent_Printf(GAGENT_ERROR,"send discover response fail,len:%d.ret:0x%x",
                        len, ret);
    }
}

