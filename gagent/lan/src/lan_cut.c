#include "gagent.h"
#include "lan.h"
#include "lanudp.h"
#include "3rdlan.h"
#include "platform.h"


/****************************************************************
*       FunctionName    :   combination_broadcast_packet
*       Description     :   combination broadcast packet data.
*       return          :   0 success other error.
*       Add by Frank Liu   --2015-04-14
****************************************************************/
int32 combination_broadcast_packet(pgcontext pgc,u8* Udp_Broadcast,uint16 cmdWord)
{
    int32 i;    
    int16 tmpDidLen = 0;
    int16 tmpMacLen = 0;
    int16 tmpPkLen = 0;
    int16 tmpFirmwareverLen = 0;
    int16 tmpMcuattrLen = 0; 
    int8 strMacByte[3] = {0};
    int pos;
    if(NULL == pgc || NULL == Udp_Broadcast) 
    {
        return RET_FAILED;
    }

    tmpDidLen = strlen(pgc->gc.DID);
    if(tmpDidLen > DID_LEN)
    {
       tmpDidLen = DID_LEN - 1;
    }
    
    tmpPkLen = strlen((char *)pgc->mcu.product_key);
    if(tmpPkLen > PK_LEN)
    {
        tmpPkLen = PK_LEN;
    }
    
    tmpFirmwareverLen=strlen(pgc->gc.FirmwareVer);
    if(tmpFirmwareverLen > FIRMWARELEN)
    {
        tmpFirmwareverLen = FIRMWARELEN;
    }
    
    tmpMcuattrLen = strlen((char *)pgc->mcu.mcu_attr);
    tmpMacLen = 6;

    if((tmpDidLen > (DID_LEN - 1)) || (tmpFirmwareverLen > FIRMWARELEN) \
        || (tmpMcuattrLen > LAN_PROTOCOL_MCU_ATTR_LEN) || (tmpPkLen > PK_LEN))
    {
        return RET_FAILED;
    }
    
    //protocolver
    *(uint32 *)Udp_Broadcast = htonl(GAGENT_PROTOCOL_VERSION);

    //cmdword 
    if((GAGENT_LAN_CMD_STARTUP_BROADCAST == cmdWord) || (GAGENT_LAN_CMD_REPLY_BROADCAST == cmdWord)) 
    {
        //varlen =flag(1b)+cmd(2b)+didlen(2b)+did(didLen)+maclen(2b)+mac+firwareverLen(2b)+firwarever+2+productkeyLen+mcu_attr
        Udp_Broadcast[4] =LAN_PROTOCOL_FLAG_LEN+LAN_PROTOCOL_CMD_LEN+sizeof(tmpDidLen)+strlen(pgc->gc.DID) \
        +sizeof(tmpMacLen)+tmpMacLen+sizeof(tmpFirmwareverLen)+strlen(pgc->gc.FirmwareVer)+sizeof(tmpPkLen)+strlen((char *)pgc->mcu.product_key)+LAN_PROTOCOL_MCU_ATTR_LEN;

        //flag
        Udp_Broadcast[5] = 0x00;
        
        //cmdword
        *(uint16 *)(Udp_Broadcast + 6) = htons(cmdWord);

        pos = 8;
        //didlen
        Udp_Broadcast[pos]=0x00;
        pos++;
        Udp_Broadcast[pos]=tmpDidLen;
        pos++;
        //did
        for(i=0;i<tmpDidLen;i++)
        {
            Udp_Broadcast[pos+i]=pgc->gc.DID[i];
        } 
        pos += tmpDidLen;
        
        //maclen
        Udp_Broadcast[pos]=0x00;
        pos++;
        Udp_Broadcast[pos]=tmpMacLen;//macLEN
        pos++;
        //mac    
        strMacByte[2] = 0;
        for(i=0;i<tmpMacLen;i++)
        {
            strMacByte[0] = pgc->minfo.szmac[i*2];
            strMacByte[1] = pgc->minfo.szmac[i*2 + 1];
            Udp_Broadcast[pos+i] = strtoul(strMacByte, NULL, 16);
        }

        pos += tmpMacLen;
        //firmwarelen
        Udp_Broadcast[pos]=0x00;
        pos++;
        Udp_Broadcast[pos]=strlen(pgc->gc.FirmwareVer);//firmwareVerLen  
        pos++;
        
        //firmware
        memcpy( &Udp_Broadcast[pos],pgc->gc.FirmwareVer,tmpFirmwareverLen);
        pos += tmpFirmwareverLen;
        
        //productkeylen
        Udp_Broadcast[pos]=0x00;
        pos++;
        Udp_Broadcast[pos]=strlen((char *)pgc->mcu.product_key);
        pos++;
        //productkey
        memcpy(&Udp_Broadcast[pos],pgc->mcu.product_key,tmpPkLen);
        pos += tmpPkLen;

        //mcu attr
        memcpy(&Udp_Broadcast[pos],pgc->mcu.mcu_attr,LAN_PROTOCOL_MCU_ATTR_LEN);
        pos += LAN_PROTOCOL_MCU_ATTR_LEN;
    }
    else if(GAGENT_LAN_CMD_AIR_BROADCAST == cmdWord) 
    {
        //varlen =flag(1b)+cmd(2b)+didlen(2b)+did(didLen)+maclen(2b)+mac+firwareverLen(2b)+firwarever+2+productkeyLen
        Udp_Broadcast[4] =LAN_PROTOCOL_FLAG_LEN+LAN_PROTOCOL_CMD_LEN+sizeof(tmpDidLen)+strlen(pgc->gc.DID) \
        +sizeof(tmpMacLen)+tmpMacLen+sizeof(tmpPkLen)+strlen((char *)pgc->mcu.product_key);
        
        //flag
        Udp_Broadcast[5] = 0x00;
        
        //cmdword
        *(uint16 *)(Udp_Broadcast + 6) = htons(cmdWord);

        pos = 8;
        //maclen
        Udp_Broadcast[pos]=0x00;
        pos++;
        Udp_Broadcast[pos]=tmpMacLen;//macLEN
        pos++;
        //mac    
        strMacByte[2] = 0;
        for(i=0;i<tmpMacLen;i++)
        {
            strMacByte[0] = pgc->minfo.szmac[i*2];
            strMacByte[1] = pgc->minfo.szmac[i*2 + 1];
            Udp_Broadcast[pos+i] = strtoul(strMacByte, NULL, 16);
        }

        pos += tmpMacLen;
        
        //productkeylen
        Udp_Broadcast[pos]=0x00;
        pos++;
        Udp_Broadcast[pos]=strlen((char *)pgc->mcu.product_key);
        pos++;
        //productkey
        memcpy(&Udp_Broadcast[pos],pgc->mcu.product_key,tmpPkLen);
        pos += tmpPkLen;

        //didlen
        Udp_Broadcast[pos]=0x00; 
        pos++;
        Udp_Broadcast[pos]=tmpDidLen;
        pos++;
        
        //did
        for(i=0;i<tmpDidLen;i++)
        {
            Udp_Broadcast[pos + i]=pgc->gc.DID[i];
        } 
        pos += tmpDidLen;
    }
    else 
    {
        return RET_FAILED;
    }

    return RET_SUCCESS;
}


/****************************************************************
*       FunctionName    :   send_broadCastPacket
*       Description     :   Send BroadCast data.
*       Add by Frank Liu   --2015-04-14
****************************************************************/
void send_broadCastPacket(pgcontext pgc,uint8* ptxBuf,uint16 cmdWord)
{
    int32 ret = 0;
    int32 len = 0;
    struct sockaddr_t addr;
    memset(&addr,0,sizeof(addr));
    
    resetPacket(pgc->rtinfo.Rxbuf);
    addr = pgc->ls.addr;

    if(NULL == ptxBuf)
    {
        return;
    }

    ret = combination_broadcast_packet(pgc,ptxBuf,cmdWord);
    if((RET_SUCCESS == ret) && (INVALID_SOCKET != pgc->ls.udpBroadCastServerFd))
    {
        len = ptxBuf[4] + LAN_PROTOCOL_HEAD_LEN + 1;
        ret = Socket_sendto(pgc->ls.udpBroadCastServerFd,ptxBuf,len,&addr,sizeof(addr));
        GAgent_Printf( GAGENT_INFO,"Send BroadCast datalen=%d cmd=%04X ret :%d",len,cmdWord,ret );
    }

    return ;
}

static void Lan_Upload_AllApp(pgcontext pgc, ppacket pTxBuf)
{
    uint8 i = 0;
    
    for(i = 0; i < LAN_TCPCLIENT_MAX; i++)
    {
        if(pgc->ls.tcpClient[i].fd >= 0 && 
            LAN_CLIENT_LOGIN_SUCCESS == pgc->ls.tcpClient[i].isLogin)
        {
            send(pgc->ls.tcpClient[i].fd, pTxBuf->phead, pTxBuf->pend - pTxBuf->phead, 0);
        }
    }
}

void Lan_sendTcpData(pgcontext pgc, int32 fd, uint16 cmd, int32 sn, ppacket pTxBuf)
{
    varc sendvarc;
    uint32 dataLen;
    int i;
    int offset = 0;

        /* protocol(4B) | varlen(xB) | flag(1B) | cmd(2B) | p0(xB)  */
    dataLen = pTxBuf->pend - pTxBuf->ppayload + LAN_PROTOCOL_CMD_LEN + LAN_PROTOCOL_FLAG_LEN;
    /* while cmd == 0x93/94,sn follow with cmd */
    if(GAGENT_LAN_CMD_CTL_93 == cmd || GAGENT_LAN_CMD_CTLACK_94 == cmd)
    {
        dataLen += LAN_PROTOCOL_SN_LEN;
    }
    sendvarc = Tran2varc(dataLen);
    pTxBuf->phead = pTxBuf->ppayload - sendvarc.varcbty - 
            LAN_PROTOCOL_HEAD_LEN - LAN_PROTOCOL_CMD_LEN - LAN_PROTOCOL_FLAG_LEN;
    
    /* while cmd == 0x93/94,sn follow with cmd */
    if(GAGENT_LAN_CMD_CTL_93 == cmd || GAGENT_LAN_CMD_CTLACK_94 == cmd)
    {
        pTxBuf->phead -= LAN_PROTOCOL_SN_LEN;
    }
    offset = 0;
    /* protocol */
    *(uint32 *)pTxBuf->phead = htonl(GAGENT_PROTOCOL_VERSION);
    offset += LAN_PROTOCOL_HEAD_LEN;
    /* varLen */
    for(i=0;i<sendvarc.varcbty;i++)
    {
        pTxBuf->phead[offset] = sendvarc.var[i];
        offset++;
    }
    /* flag */
    pTxBuf->phead[offset] = 0x00;
    offset += 1;

    /* cmd */
    *(uint16 *)(pTxBuf->phead + offset) = htons(cmd);
    offset += LAN_PROTOCOL_CMD_LEN;
    switch(cmd)
    {
        case GAGENT_LAN_CMD_CTL_93:
            sn = 0;
            *(int32 *)(pTxBuf->phead + offset) = htonl(sn);
            offset += LAN_PROTOCOL_SN_LEN;
            fd = -1;
            break;
        case GAGENT_LAN_CMD_CTLACK_94:
            *(int32 *)(pTxBuf->phead + offset) = htonl(sn);
            offset += LAN_PROTOCOL_SN_LEN;
            break;
        case GAGENT_LAN_CMD_TRANSMIT_91:
            break;
        default:
            /* not support ,return directly */
            return ;
            break;
    }

    if(fd >= 0)
    {
        send(fd, pTxBuf->phead, pTxBuf->pend - pTxBuf->phead, 0);
    }
    else
    {
        Lan_Upload_AllApp(pgc, pTxBuf);
    }
    
    return ;
}

void Lan_SetClientAttrs(pgcontext pgc, int32 fd, uint16 cmd, int32 sn)
{
    pgc->ls.srcAttrs.sn = sn;
    pgc->ls.srcAttrs.fd = fd;
    pgc->ls.srcAttrs.cmd = cmd;
}

void Lan_ClearClientAttrs(pgcontext pgc, stLanAttrs_t *client)
{
    if(NULL != client)
    {
        client->sn = 0;
        client->fd = INVALID_SOCKET;
        client->cmd = 0;
    }
}

/****************************************************************
        FunctionName    :   GAgent_Lan_SendTcpData
        Description     :   send buf data to TCP client.
        return          :   void
        Add by Will.zhou     --2015-03-17
****************************************************************/
void GAgent_Lan_SendTcpData(pgcontext pgc,ppacket pTxBuf)
{
    int32 fd;
    uint16 cmd;
    int32 sn;

    fd = pgc->rtinfo.stChannelAttrs.lanClient.fd;
    cmd = pgc->rtinfo.stChannelAttrs.lanClient.cmd;
    sn = pgc->rtinfo.stChannelAttrs.lanClient.sn;

    Lan_sendTcpData(pgc, fd, cmd, sn, pTxBuf);
    
    return ;
}
/****************************************************************
        FunctionName        :   CreateUDPBroadCastServer.
        Description         :   create udp broadcast server.
        Add by Frank Liu     --2015-04-22
****************************************************************/
void CreateUDPBroadCastServer(pgcontext pgc)
{   

    if(NULL == pgc)
    {
        return;
    }
    pgc->ls.onboardingBroadCastTime = SEND_UDP_DATA_TIMES;
/*
    if((pgc->ls.onboardingBroadCastTime > 0) && (pgc->ls.onboardingBroadCastTime != SEND_UDP_DATA_TIMES))
    {
        pgc->ls.broResourceNum--;
    }
    pgc->ls.broResourceNum++;
*/

    if(INVALID_SOCKET == pgc->ls.udpBroadCastServerFd)
    {
       pgc->ls.addr = Lan_CreateUDPBroadCastServer(&(pgc->ls.udpBroadCastServerFd),LAN_UDP_BROADCAST_SERVER_PORT);
    }
}
/****************************************************************
        FunctionName        :   DestroyUDPBroadCastServer.
        Description         :   destroy udp broadcast server.
        Add by Frank Liu     --2015-04-22
****************************************************************/
void DestroyUDPBroadCastServer(pgcontext pgc)
{
    if(NULL == pgc)
    {
        return;
    }
    /*
    pgc->ls.broResourceNum--;
    if(pgc->ls.broResourceNum <= 0)
    {*/
        if(INVALID_SOCKET != pgc->ls.udpBroadCastServerFd)
        {
            close(pgc->ls.udpBroadCastServerFd);
            pgc->ls.udpBroadCastServerFd = INVALID_SOCKET;
        }
//      pgc->ls.broResourceNum = 0;
//   }
}
 
void GAgent_LanTick( pgcontext pgc,uint32 dTime_s )
{
  
}

uint32 GAgent_Lan_Handle(pgcontext pgc, ppacket prxBuf,int32 len)
{
    return 0;
}

void GAgent_LANInit(pgcontext pgc)
{
    GAgent_Printf( GAGENT_WARNING,"LAN module has been cut!\n");
}
/****************************************************************
        FunctionName        :   LAN_InitSocket.
        Description         :      create tcp/udp server.
        Add by Will.zhou     --2015-03-10
****************************************************************/
int32 LAN_InitSocket(pgcontext pgc)
{
    pgc->ls.tcpServerFd = -1;
    pgc->ls.udpServerFd = -1;
    pgc->ls.udpBroadCastServerFd = INVALID_SOCKET;
    pgc->ls.tcpClientNums = 0;
    pgc->ls.onboardingBroadCastTime = SEND_UDP_DATA_TIMES;
    pgc->ls.startupBroadCastTime = SEND_UDP_DATA_TIMES;
    
    Lan_CreateTCPServer(&(pgc->ls.tcpServerFd), GAGENT_TCP_SERVER_PORT);
    Lan_CreateUDPServer(&(pgc->ls.udpServerFd), LAN_UDP_SERVER_PORT );
    CreateUDPBroadCastServer(pgc);//startup broadcast
    return 0;
}   
  



