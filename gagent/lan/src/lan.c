#include "gagent.h"
#include "lan.h"
#include "platform.h"

/****************************************************************
        FunctionName    :   GAgent_Lan_SendTcpData
        Description     :   send buf data to TCP client.
        return          :   void
        Add by Will.zhou     --2015-03-17
****************************************************************/
void GAgent_Lan_SendTcpData(pgcontext pgc,ppacket pTxBuf)
{
    varc sendvarc;
    uint32 dataLen;
    int i;
    int offset = 0;

        /* protocol(4B) | varlen(xB) | flag(1B) | cmd(2B) | p0(xB)  */
    dataLen = pTxBuf->pend - pTxBuf->ppayload + LAN_PROTOCOL_CMD_LEN + LAN_PROTOCOL_FLAG_LEN;
    sendvarc = Tran2varc(dataLen);
    pTxBuf->phead = pTxBuf->ppayload - sendvarc.varcbty - 
            LAN_PROTOCOL_HEAD_LEN - LAN_PROTOCOL_CMD_LEN - LAN_PROTOCOL_FLAG_LEN;

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
    *(uint16 *)(pTxBuf->phead + offset) = htons(0x0091);
    offset += LAN_PROTOCOL_CMD_LEN;

    for(i = 0; i < LAN_TCPCLIENT_MAX; i++)
    {
        if(pgc->ls.tcpClient[i].fd > 0 && 
            LAN_CLIENT_LOGIN_SUCCESS == pgc->ls.tcpClient[i].isLogin)
        {
            send(pgc->ls.tcpClient[i].fd, pTxBuf->phead, pTxBuf->pend - pTxBuf->phead, 0);
        }
    }
        
}

/****************************************************************
        FunctionName        :   GAgent_LanTick.
        Description         :   check clients whether it is timeout.
        Add by Will.zhou     --2015-03-10
****************************************************************/
void GAgent_LanTick(pgcontext pgc)
{
    uint32 cTime=0,dTime=0;
    int32 i;
    static uint32 preTime = 0;

    if(0 == preTime)
        preTime = GAgent_GetDevTime_S();
    cTime = GAgent_GetDevTime_S();
    if(cTime >= preTime)
    {
        dTime = cTime - preTime;
    }
    else
    {
        dTime = cTime + (~preTime);
    }
    
    if(dTime < 1)
    {
        return ;
    }
    preTime = GAgent_GetDevTime_S();

    for(i = 0; i < LAN_TCPCLIENT_MAX; i++)
    {
        if(pgc->ls.tcpClient[i].fd > 0)
        {
            if(pgc->ls.tcpClient[i].timeout <= dTime)
            {
                close(pgc->ls.tcpClient[i].fd);
                pgc->ls.tcpClient[i].fd = -1;
                if( LAN_CLIENT_LOGIN_SUCCESS == pgc->ls.tcpClient[i].isLogin)
                {
                    pgc->ls.tcpClientNums--;
                }
            }
            else
            {
                pgc->ls.tcpClient[i].timeout -= dTime;
            }
        }
    }

    if(pgc->ls.udpSendBroadCastTime > 0)
    {
        pgc->ls.udpSendBroadCastTime--;
    }
}

uint32 GAgent_Lan_Handle(pgcontext pgc, ppacket prxBuf, ppacket ptxBuf, int32 len)
{
    int i =0;
    int fd =0;
    int ret =0;
    
    Lan_udpDataHandle(pgc, prxBuf, ptxBuf, len);
    Lan_TcpServerHandler(pgc);
    
    for(i = 0;i < LAN_TCPCLIENT_MAX; i++)
    {
        fd = pgc->ls.tcpClient[i].fd;
        if(fd <= 0)
            continue;
        if(FD_ISSET(fd, &(pgc->rtinfo.readfd)))
        {
            ret = Lan_tcpClientDataHandle(pgc, i, prxBuf, ptxBuf, len);
            if(ret > 0)
            {
                dealPacket(pgc, prxBuf);
            }
        }
    }
}

/****************************************************************
        FunctionName        :   GAgent_LANInit.
        Description         :   init clients socket and create tcp/udp server.
        Add by Will.zhou     --2015-03-10
****************************************************************/
void GAgent_LANInit(pgcontext pgc)
{
    LAN_tcpClientInit(pgc);      
    LAN_InitSocket(pgc);           
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
    pgc->ls.udpBroadCastServerFd = -1;
    
    Lan_CreateTCPServer(&(pgc->ls.tcpServerFd), GAGENT_TCP_SERVER_PORT);
    Lan_CreateUDPServer(&(pgc->ls.udpServerFd), LAN_UDP_SERVER_PORT );
    Lan_CreateUDPBroadCastServer(&(pgc->ls.udpBroadCastServerFd), LAN_UDP_BROADCAST_SERVER_PORT );
    signal(SIGPIPE, SIG_IGN);
    return 0;
}

