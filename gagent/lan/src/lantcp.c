#include "gagent.h"
#include "lan.h"

/****************************************************************
        FunctionName        :   Lan_setClientTimeOut.
        Description         :      setting new tcp client timeout.
        Add by Will.zhou     --2015-03-10
****************************************************************/
void Lan_setClientTimeOut(pgcontext pgc, int32 channel)
{
    uint16 GAgentStatus=0;
    GAgentStatus = pgc->rtinfo.GAgentStatus;
    if( (GAgentStatus&WIFI_MODE_AP)== WIFI_MODE_AP )
    {
        pgc->ls.tcpClient[channel].timeout = LAN_CLIENT_MAXLIVETIME*5;
    }
    if( (GAgentStatus&WIFI_MODE_STATION) == WIFI_MODE_STATION )
    {
        pgc->ls.tcpClient[channel].timeout = LAN_CLIENT_MAXLIVETIME;
    }
}

int32 Lan_AddTcpNewClient(pgcontext pgc, int fd, struct sockaddr_t *addr)
{
    int32 i;
    
    if(fd < 0)
    {
        return RET_FAILED;
    }

    for(i = 0; i < LAN_TCPCLIENT_MAX; i++)
    {
        if(pgc->ls.tcpClient[i].fd == INVALID_SOCKET)
        {
            pgc->ls.tcpClient[i].fd = fd;
            Lan_setClientTimeOut(pgc, i);
			
            return RET_SUCCESS;
        }
    }

    GAgent_Printf(GAGENT_DEBUG, "[LAN]tcp client over %d channel, denied!", LAN_TCPCLIENT_MAX);
    close(fd);
    
    return RET_FAILED;
}

/****************************************************************
        FunctionName        :   Lan_TcpServerHandler.
        Description         :   Lan handing new tcp client.
        Add by Will.zhou     --2015-03-10
****************************************************************/
int32 Lan_TcpServerHandler(pgcontext pgc)
{
    int newfd, ret = RET_FAILED;
    struct sockaddr_t addr;
    int addrLen = sizeof(struct sockaddr_t);
    
    if(pgc->ls.tcpServerFd < 0)
    {
        return RET_FAILED;
    }
    if(FD_ISSET(pgc->ls.tcpServerFd, &(pgc->rtinfo.readfd)))
    {
        /* if nonblock, can be done in accept progress */
        newfd = Socket_accept(pgc->ls.tcpServerFd, &addr, &addrLen);
        if(newfd > 0)
        {
            GAgent_Printf(GAGENT_DEBUG, "detected new client as %d", newfd);
            ret = Lan_AddTcpNewClient(pgc, newfd, &addr);
        }

    }
    return ret;
}

int32 LAN_readPacket(int32 fd, ppacket pbuf, int32 bufLen)
{
    int dataLenth = 0;
    
    resetPacket( pbuf );
    memset(pbuf->phead, 0, bufLen);
    
    dataLenth = recv(fd, pbuf->phead, bufLen, 0);

    return dataLenth;
    
}

int32 Lan_tcpClientDataHandle(pgcontext pgc, uint32 channel, 
                ppacket prxBuf, ppacket ptxBuf, int32 buflen)
{
    int32 fd = pgc->ls.tcpClient[channel].fd;
    int32 recDataLen =0;
    
    recDataLen = LAN_readPacket(fd, prxBuf, buflen);

    if(recDataLen <= 0)
    {
        if(pgc->ls.tcpClient[channel].fd > 0)
        {
            close(pgc->ls.tcpClient[channel].fd);
            pgc->ls.tcpClient[channel].fd = INVALID_SOCKET;
            pgc->ls.tcpClient[channel].isLogin = LAN_CLIENT_LOGIN_FAIL;
            pgc->ls.tcpClient[channel].timeout = 0;
           
            if(pgc->ls.tcpClientNums > 0)
            {
                pgc->ls.tcpClientNums--;
            }

            if(0 == (pgc->ls.tcpClientNums + pgc->rtinfo.waninfo.wanclient_num))
            {
                GAgent_SetWiFiStatus( pgc,WIFI_CLIENT_ON,0 );
            }
            
        }
        return RET_FAILED;
    }

    Lan_setClientTimeOut(pgc, channel);
    return Lan_dispatchTCPData(pgc, prxBuf, ptxBuf, channel);
}

/****************************************************************
        FunctionName        :   Lan_checkAuthorization.
        Description         :      check Authorization.
        Add by Nik.chen     --2015-04-18
****************************************************************/
static uint32 Lan_checkAuthorization( pgcontext pgc,  int clientIndex)
{
  
    if((LAN_CLIENT_LOGIN_FAIL == pgc->ls.tcpClient[clientIndex].isLogin)
        &&(INVALID_SOCKET != pgc->ls.tcpClient[clientIndex].fd))
    {
        close(pgc->ls.tcpClient[clientIndex].fd);
        pgc->ls.tcpClient[clientIndex].fd = INVALID_SOCKET;

        GAgent_Printf(GAGENT_INFO, "Illegal tcp client login!!! clientid[%d] ",clientIndex);
            return 0;
     }
     
    return 1;
}

/****************************************************************
        FunctionName        :   Lan_handleLogin.
        Description         :      Lan Tcp logining.
        Add by Will.zhou     --2015-03-10
****************************************************************/
static void Lan_handleLogin( pgcontext pgc, ppacket src, ppacket dest, int clientIndex)
{
    int i;
    int ret;
    int32 fd;
    uint8 isLogin;
    uint8 *pbuf;

    resetPacket( dest );
    pbuf = dest->phead;
    fd = pgc->ls.tcpClient[clientIndex].fd;

    /* verify passcode */
    if( !memcmp((src->phead + 10), pgc->gc.wifipasscode, strlen( pgc->gc.wifipasscode)) )
    {
        /* login success */
        isLogin = LAN_CLIENT_LOGIN_SUCCESS;
         pgc->ls.tcpClientNums++;
         GAgent_SetWiFiStatus( pgc,WIFI_CLIENT_ON,1 );
    
        GAgent_Printf(GAGENT_INFO, "LAN login success! clientid[%d] ",clientIndex);
    }
    else
    {
        isLogin = LAN_CLIENT_LOGIN_FAIL;
        
        GAgent_Printf(GAGENT_WARNING,"LAN login fail. your passcode:%s",
                        src->phead + 10);
       
        GAgent_Printf(GAGENT_INFO, "expected passcode:%s", pgc->gc.wifipasscode);
    }
    /* protocol version */
     pbuf[0] = 0x00;
     pbuf[1] = 0x00;
     pbuf[2] = 0x00;
     pbuf[3] = 0x03;

     /* len */
     pbuf[4] = 0x04;

     /* flag */
     pbuf[5] = 0x00;

     /* cmd */
     pbuf[6] = 0x00;
     pbuf[7] = 0x09;

     /* login result */
     pbuf[8] = isLogin;

     pgc->ls.tcpClient[clientIndex].isLogin = isLogin;

     send(pgc->ls.tcpClient[clientIndex].fd, pbuf, 9, 0);

     if(LAN_CLIENT_LOGIN_FAIL == isLogin)
     {
            close(pgc->ls.tcpClient[clientIndex].fd);
            pgc->ls.tcpClient[clientIndex].fd = -1;
     }
    
}

/****************************************************************
        FunctionName        :   Lan_handlePasscode.
        Description         :   reponsing passcode to client for Binding.
        Add by Will.zhou     --2015-03-10
****************************************************************/
static void Lan_handlePasscode( pgcontext pgc, ppacket src, int clientIndex)
{
    int i;
    int ret;
    int32 fd;
    uint16 passcodeLen;
    uint8 *pbuf;

    resetPacket(src);
    pbuf = src->phead;
    fd = pgc->ls.tcpClient[clientIndex].fd;

    /* protocol version */
    pbuf[0] = 0x00;
    pbuf[1] = 0x00;
    pbuf[2] = 0x00;
    pbuf[3] = 0x03;

    /* len */
    pbuf[4] = 0x0f;

    /* flag */
    pbuf[5] = 0x00;

    /* cmd */
    pbuf[6] = 0x00;
    pbuf[7] = 0x07;

    /* passcode len */ 
    passcodeLen = htons(strlen( pgc->gc.wifipasscode ));
    *(uint16 *)(pbuf + 8) =  passcodeLen;

    /* passcode */
    for(i=0;i < passcodeLen;i++)
    {
    	pbuf[10+i] = pgc->gc.wifipasscode[i];
    }

    if((pgc->rtinfo.GAgentStatus & WIFI_MODE_BINDING) == WIFI_MODE_BINDING)//enable bind 
	{
		ret = send(fd, pbuf, 20, 0);			
        GAgent_Printf(GAGENT_INFO,"Send passcode(%s) to client[%d][send data len:%d] ", 
        pgc->gc.wifipasscode, fd, ret);		
	}
	else
	{
		/* passcode len */
    	pbuf[8] = 0x00;
    	pbuf[9] = 0x00;
    	
		ret = send(fd, pbuf, 10, 0);
        GAgent_Printf(GAGENT_ERROR,"timeout,client[%d]cannot bind,close socket", fd);
		close(fd);
		pgc->ls.tcpClient[clientIndex].fd = INVALID_SOCKET;
	}

    return;
}

/****************************************************************
        FunctionName        :   GAgent_Lan_SendDevInfo.
        Description         :   	send Dev Info to App.
        Add by Eric.gong     --2015-04-22
****************************************************************/
void GAgent_Lan_SendDevInfo(pgcontext pgc,ppacket pTxBuf,int32 clientIndex)
{
    varc sendvarc;
    uint32 dataLen;
	uint16 wifi_firmware_ver_len;
    int i;
	int offset=0;
    
    /* protocol */
    *(uint32 *)pTxBuf->phead = htonl(GAGENT_PROTOCOL_VERSION);
	offset += LAN_PROTOCOL_HEAD_LEN;
    /* varLen */
	/* flag(1B) | cmd(2B) | p0(116B)  */
    dataLen = 1+2+116;
    sendvarc = Tran2varc(dataLen);  
    for(i=0;i<sendvarc.varcbty;i++)
    {
        pTxBuf->phead[offset] = sendvarc.var[i];
        offset++;
    }
    /* flag */
    pTxBuf->phead[offset] = 0x00;
	offset +=1;
    /* cmd */
    pTxBuf->phead[offset] = 0x00;
	pTxBuf->phead[offset+1] = 0x14;
	offset +=2;
	/* wifi_hard_ver */
	strcpy( pTxBuf->phead+offset,WIFI_HARDVER);
	offset +=8;
	/* wifi_soft_ver */
	strcpy( pTxBuf->phead+offset,WIFI_SOFTVAR);
	offset +=8;
	/* mcu_hard_ver */
	for(i=0;i<8;i++)
		pTxBuf->phead[offset+i]=pgc->mcu.hard_ver[i];
	offset +=8;
	/* mcu_soft_ver */
	for(i=0;i<8;i++)
		pTxBuf->phead[offset+i]=pgc->mcu.soft_ver[i];
	offset +=8;
	/* protocol version of payload */
	for(i=0;i<8;i++)
		pTxBuf->phead[offset+i]=pgc->mcu.p0_ver[i];
	offset +=8;
	/* wifi_firmware_id */
	for(i=0;i<8;i++)
		 pTxBuf->phead[offset+i]=0;
	offset +=8;
	/* wifi_firmware_ver_len */
	wifi_firmware_ver_len = pgc->gc.FirmwareVerLen[1] | (pgc->gc.FirmwareVerLen[0] << 8);
	if(wifi_firmware_ver_len > FIRMWARE_LEN_MAX)
	{
		wifi_firmware_ver_len = FIRMWARE_LEN_MAX;	
		pgc->gc.FirmwareVer[FIRMWARE_LEN_MAX - 1] = 0;
	}
	*(uint16 *)(pTxBuf->phead + offset) = htons(wifi_firmware_ver_len);
	offset +=2;
	/* wifi_firmware_ver */
	for(i = 0;i < wifi_firmware_ver_len;i++)
	 	pTxBuf->phead[offset+i]=pgc->gc.FirmwareVerLen[i];
	offset +=wifi_firmware_ver_len;	
	/* produckt_key_len */
	*(uint16 *)(pTxBuf->phead + offset) = htons(32);
	 offset +=2;
	/* produckt_key */
	for(i=0;i<32;i++)
		 pTxBuf->phead[offset+i]=pgc->mcu.product_key[i];
	offset +=32;

    if(pgc->ls.tcpClient[clientIndex].fd > 0 )
    {				
        send(pgc->ls.tcpClient[clientIndex].fd, pTxBuf->phead,offset, 0); 
    }
}      

/****************************************************************
        FunctionName        :   Lan_AckHeartbeak.
        Description         :      Gagent response client heartbeat
        Add by Will.zhou     --2015-03-10
****************************************************************/
static void Lan_AckHeartbeak( pgcontext pgc, ppacket src, int clientIndex )
{
    int32 fd;
    uint8 *pbuf;

    resetPacket(src);
    pbuf = src->phead;
    fd = pgc->ls.tcpClient[clientIndex].fd;

    /* protocol version */
    pbuf[0] = 0x00;
    pbuf[1] = 0x00;
    pbuf[2] = 0x00;
    pbuf[3] = 0x03;

    /* len */
    pbuf[4] = 0x03;

    /* flag */
    pbuf[5] = 0x00;

    /* cmd */
    pbuf[6] = 0x00;
    pbuf[7] = 0x16;

    send( fd, pbuf, 8, 0);
}
/****************************************************************
        FunctionName        :   Local_Ack2TcpClient.
        Description         :   Gagent response test ack to tcp client
        Add by Frank Liu        --2015-05-05
****************************************************************/
void Local_Ack2TcpClient(pgcontext pgc, uint32 channel)
{
    int8 tmpBuf[8] = {0,};
    int8 flag = 0;
    int16 cmdWord = GAGENT_LAN_REPLY_TEST;
    memset(tmpBuf,0,sizeof(tmpBuf));
    
    if(pgc->ls.tcpClient[channel].fd > 0)
    {
        //protocolver
        *(uint32 *)tmpBuf = htonl(GAGENT_PROTOCOL_VERSION);
        tmpBuf[4] = sizeof(flag) + sizeof(cmdWord);
        //flag
        tmpBuf[5] = 0x00;
        //cmdword
        *(uint16 *)(tmpBuf + 6) = htons(cmdWord);
        
        send( pgc->ls.tcpClient[channel].fd,tmpBuf,sizeof(tmpBuf),0 );
    }
}
/****************************************************************
        FunctionName        :   Lan_dispatchTCPData.
        Description         :   parse and dispatch tcp cmd message.
        Add by Will.zhou     --2015-03-10
****************************************************************/
int32 Lan_dispatchTCPData(pgcontext pgc, ppacket prxBuf, ppacket ptxBuf, int32 clientIndex)
{
    int datalen;
    int ret = 0;
    uint16 cmd;
    int32 bytesOfLen;
    int len;

    bytesOfLen = mqtt_num_rem_len_bytes(prxBuf->phead + 3);
    len = mqtt_parse_rem_len(prxBuf->phead + 3);

    cmd = *(uint16 *)(prxBuf->phead + LAN_PROTOCOL_HEAD_LEN + LAN_PROTOCOL_FLAG_LEN
                        + bytesOfLen);
    cmd = ntohs(cmd);
    prxBuf->type = SetPacketType( prxBuf->type,LAN_TCP_DATA_IN,1 );
    ret = ParsePacket(prxBuf);
    
    if((cmd != GAGENT_LAN_CMD_BINDING) && (cmd != GAGENT_LAN_CMD_LOGIN)
		&& (cmd != GAGENT_LAN_CMD_INFO))
    {
        ret = Lan_checkAuthorization(pgc, clientIndex);
        if(0 == ret)
        {
            return ret;
        }
    }
    ret = 0;
    switch (cmd)
    {
        case GAGENT_LAN_CMD_BINDING:
            Lan_handlePasscode(pgc, ptxBuf, clientIndex);
            break;
        case GAGENT_LAN_CMD_LOGIN:
            Lan_handleLogin(pgc, prxBuf, ptxBuf, clientIndex);
            break;
        case GAGENT_LAN_CMD_TRANSMIT:
            prxBuf->type = SetPacketType( prxBuf->type, LAN_TCP_DATA_IN, 0 );
            prxBuf->type = SetPacketType( ptxBuf->type, LOCAL_DATA_OUT, 1 );
            if((prxBuf->pend - prxBuf->ppayload) > 0)
               ret = prxBuf->pend - prxBuf->ppayload;
            else
               ret = 0;
            break;
        case GAGENT_LAN_CMD_HOSTPOTS:
            /* 
            Lan_GetWifiHotspots();
            */
            break;
        case GAGENT_LAN_CMD_LOG:
            break;
        case GAGENT_LAN_CMD_INFO:
    		resetPacket(ptxBuf);
    		GAgent_Lan_SendDevInfo(pgc, ptxBuf, clientIndex);
            break;
        case GAGENT_LAN_CMD_TICK:
            GAgent_Printf(GAGENT_WARNING,"LAN TCP heartbeat...");
            Lan_AckHeartbeak(pgc, ptxBuf, clientIndex);
            break;
        case GAGENT_LAN_CMD_TEST:
            Local_Ack2TcpClient( pgc,clientIndex);
            GAgent_ExitTest( pgc );          
            break;
        default:
            break;
    }
    return ret;
}

/****************************************************************
        FunctionName        :   LAN_tcpClientInit.
        Description         :      init tcp clients.
        Add by Will.zhou     --2015-03-10
****************************************************************/
int32 LAN_tcpClientInit(pgcontext pgc)
{
    int32 i;

    for (i = 0; i < LAN_TCPCLIENT_MAX; i++)
    {
        memset(&(pgc->ls.tcpClient[i]), 0x0, sizeof(pgc->ls.tcpClient[i]));
        pgc->ls.tcpClient[i].fd = -1;
        pgc->ls.tcpClient[i].isLogin = LAN_CLIENT_LOGIN_FAIL;
        pgc->ls.tcpClient[i].timeout = 0;
    }

    return  RET_SUCCESS;
}

/****************************************************************
        FunctionName        :   Lan_CreateTCPServer.
        Description         :      create tcp server.
        Add by Will.zhou     --2015-03-10
****************************************************************/
void Lan_CreateTCPServer(int32 *pFd, uint16 tcp_port)
{
    *pFd = GAgent_CreateTcpServer( tcp_port );
}

