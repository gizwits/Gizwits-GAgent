#include "gagent.h"
#include "utils.h"
#include "lan.h"
#include "local.h"
#include "hal_receive.h"
#include "mcu.h"
#include "cloud.h"

#define ACKBUF_LEN 1024
ppacket pLocalAckbuf=NULL;

pfMasterMCU_ReciveData PF_ReceiveDataformMCU = NULL;
pfMasertMCU_SendData   PF_SendData2MCU = NULL;

int isleap(int year)
{   
	return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}
int get_yeardays(int year) 
{
    if (isleap(year))
        return 366;
    return 365;
}
_tm GAgent_GetLocalTimeForm(uint32 time)
{
	time += Eastern8th;
	_tm tm;
	int x;
	int i=1970, mons[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	for(i=1970; time>0;)
	{
	    x=get_yeardays(i);
	    if(time >= x*DAY_SEC)
	    {
	        time -= x*DAY_SEC;
	        i++;
	    }
	    else
	    {
	        break;
	    }
	}
    tm.year = i;
    
	for(i=0; time>0;)
	{
        if (isleap(tm.year))
            mons[1]=29;       
        if(time >= mons[i]*DAY_SEC)
	    {
	        time -= mons[i]*DAY_SEC;
	        i++;
	    }
	    else
	    {
	        break;
	    }
	}
	mons[1]=28;
	tm.month = i+1;
   
    for(i=1; time>0;)
	{
        if(time >= DAY_SEC)
	    {
	        time -= DAY_SEC;
	        i++;
	    }
	    else
	    {
	        break;
	    }
	}	
	tm.day=i;
    
    tm.hour = time/(60*60);
    tm.minute = time%(60*60)/60;
    tm.second = time%60;
    	
	return tm;
}

/* 注册GAgent接收local  数据函数 */
void GAgent_RegisterReceiveDataHook(pfMasterMCU_ReciveData fun)
{
    PF_ReceiveDataformMCU = fun;
    return;
}
/* 注册GAgent发送local  数据函数 */
void GAgent_RegisterSendDataHook(pfMasertMCU_SendData fun)
{
    PF_SendData2MCU = fun;
    return;
}
/****************************************************************
FunctionName    :   Local_DataValidityCheck
Description     :   check local receive data validity.
Rxbuf           :   local data.
return          :   0 useful data
                    1 error data
Add by Alex.lin     --2015-05-04
****************************************************************/
uint8 Local_DataValidityCheck( int32 fd,ppacket buf,int32 RxLen )
{        
    int8 cmd=0;
    uint8 sn=0,checksum=0;
    uint8 *localRxbuf = NULL;
    localRxbuf = buf->phead;

    cmd = localRxbuf[4];
    sn  = localRxbuf[5];

    if(RxLen > 0)
    {
        if((0xFF != localRxbuf[0]) || (0xFF != localRxbuf[1]))
        {
            buf->ppayload[0] = GAGENT_MCU_OTHER_ERROR;
            buf->pend = (buf->ppayload)+1;
            Local_Ack2MCUwithP0( buf, fd, sn, MCU_DATA_ILLEGAL );
            return 1;
        }

        switch(cmd)
        {
            case MCU_INFO_CMD_ACK:
            case WIFI_PING2MCU_ACK:
            case MCU_CTRL_CMD_ACK:
            case WIFI_STATUS2MCU_ACK:
                return 0;
            case MCU_REPORT:
            case MCU_CONFIG_WIFI:
            case MCU_RESET_WIFI:
            case WIFI_TEST:
            case MCU_ENABLE_BIND:
            case MCU_REQ_GSERVER_TIME:
                checksum = GAgent_SetCheckSum( localRxbuf,RxLen-1 );
                if( checksum != localRxbuf[RxLen-1] )
                {
                    buf->ppayload[0] = GAGENT_MCU_CHECKSUM_ERROR;
                    buf->pend = (buf->ppayload)+1;
                    Local_Ack2MCUwithP0( buf, fd, sn, MCU_DATA_ILLEGAL );
                    return 1;
                }
                return 0;
            case MCU_REPLY_GAGENT_DATA_ILLEGAL:
                GAgent_Printf( GAGENT_WARNING,"mcu reply the data from gagent is illegal!\r\n");
                return 0;
            default:
                buf->ppayload[0] = GAGENT_MCU_CMD_ERROR;
                buf->pend = (buf->ppayload)+1;
                Local_Ack2MCUwithP0( buf, fd, sn, MCU_DATA_ILLEGAL );  
                return 1;
        }
    }
    
    return 0;
}
/****************************************************************
FunctionName    :   GAgent_MoveOneByte
Description     :   move the array one byte to left or right
pData           :   need to move data pointer.
dataLen         :   data length 
flag            :   0 move right
                    1 move left.
return          :   NULL
Add by Alex.lin     --2015-04-01
****************************************************************/
void GAgent_MoveOneByte( uint8 *pData,int32 dataLen,uint8 flag )
{
    int32 i=0;
    if( 0==flag)
    {
        for( i=dataLen;i>0;i-- )
        {
            pData[i] = pData[i-1];
        }
    }
    else if( 1==flag )
    {
        for( i=0;i<dataLen;i++ )
        {
            pData[i] = pData[i+1];
        }
    }
    return;
}

/****************************************************************
FunctionName    :   GAgent_LocalDataAdapter
Dercription     :   the function will add 0x55 after local send data,
pData           :   the source of data need to change.
dataLen         :   the length of source data.
destBuf         :   the data after change.
return          :   the length of destBuf.                                  
Add by Alex.lin     --2015-03-31
****************************************************************/
int32 Local_DataAdapter( uint8 *pData,int32 dataLen )
{

    int32 i=0,j=0,len = 0;

    len = 2;//MCU_LEN_NO_PAYLOAD;
    len += dataLen;

    for( i=0;i<dataLen;i++ )
    {
        if( 0xFF==pData[i] )
        {
            GAgent_MoveOneByte( &pData[i+1],(dataLen-i),0 );
            pData[i+1] =0x55;
            j++;
            dataLen++;
        }
    }
    return len+j;

}
/****************************************************************
FunctionName    :   GAgent_LocalHalInit
Description     :   init hal buf.
return          :   NULL
Add by Alex.lin     --2015-04-07
****************************************************************/
void Local_HalInit()
{
    int totalCap = ACKBUF_LEN+ BUF_HEADLEN;
    int bufCap = ACKBUF_LEN;
    hal_ReceiveInit( );

    pLocalAckbuf = (ppacket)malloc(sizeof(packet));
    pLocalAckbuf->allbuf = (uint8 *)malloc( totalCap );
    while( pLocalAckbuf->allbuf==NULL )
    {
        pLocalAckbuf->allbuf = (uint8 *)malloc( totalCap );
        sleep(1);
    }
    memset( pLocalAckbuf->allbuf,0,totalCap );
    pLocalAckbuf->totalcap = totalCap;
    pLocalAckbuf->bufcap = bufCap;
    resetPacket( pLocalAckbuf );

}

/****************************************************************
FunctionName    :   GAgent_LocalSendData
Description     :   send data to local io.
return          :   NULL
Add by Alex.lin     --2015-04-07
****************************************************************/
uint32 Local_SendData( int32 fd,uint8 *pData, int32 bufferMaxLen )
{
    int32 i=0;
    if( PF_SendData2MCU!=NULL )
    {
        GAgent_Printf( GAGENT_DUMP,"local send len = %d:\r\n",bufferMaxLen );
        for( i=0;i<bufferMaxLen;i++ )
            GAgent_Printf( GAGENT_DUMP," %02x",pData[i]);
        GAgent_Printf( GAGENT_DUMP,"\r\n");
        PF_SendData2MCU( fd,pData,bufferMaxLen );
    }
    return 0;
}
/****************************************************************
FunctionName    :   Local_Ack2MCU.
Description     :   ack to mcu after receive mcu data.
fd              :   local data fd.
sn              :   receive local data sn .
cmd             :   ack to mcu cmd.
****************************************************************/
void Local_Ack2MCU( int32 fd,uint8 sn,uint8 cmd )
{
    int32 len = MCU_LEN_NO_PAYLOAD; 
    uint16 p0_len = htons(5);    
    uint8 buf[MCU_LEN_NO_PAYLOAD];
    
    memset(buf, 0, len);
    buf[0] = MCU_HDR_FF;
    buf[1] = MCU_HDR_FF;
    memcpy(&buf[MCU_LEN_POS], &p0_len, 2);
    buf[MCU_CMD_POS] = cmd;
    buf[MCU_SN_POS] = sn;
    buf[MCU_LEN_NO_PAYLOAD-1]=GAgent_SetCheckSum( buf, (MCU_LEN_NO_PAYLOAD-1));
    Local_SendData( fd,buf,len );

    return ;
}
/****************************************************************
FunctionName    :   Local_Ack2MCUwithP0.
Description     :   ack to mcu with P0 after receive mcu data.
pbuf            :   the data send to MCU
fd              :   local data fd.
sn              :   receive local data sn .
cmd             :   ack to mcu cmd.
****************************************************************/
void Local_Ack2MCUwithP0( ppacket pbuf, int32 fd,uint8 sn,uint8 cmd )
{
    uint16 datalen = 0;
    uint16 flag = 0;
    uint16 sendLen = 0; 
    pbuf->phead = (pbuf->ppayload)-8;
    
    /* head(0xffff)| len(2B) | cmd(1B) | sn(1B) | flag(2B) |  payload(xB) | checksum(1B) */
    pbuf->phead[0] = MCU_HDR_FF;
    pbuf->phead[1] = MCU_HDR_FF;
    datalen = pbuf->pend - pbuf->ppayload + 5;    //p0 + cmd + sn + flag + checksum
    *(uint16 *)(pbuf->phead + 2) = htons(datalen);
    pbuf->phead[4] = cmd;
    pbuf->phead[5] = sn;
    *(uint16 *)(pbuf->phead + 6) = htons(flag);
    *( pbuf->pend )  = GAgent_SetCheckSum(pbuf->phead, (pbuf->pend)-(pbuf->phead) );
    pbuf->pend += 1;  /* add 1 Byte of checksum */

    sendLen = (pbuf->pend) - (pbuf->phead);
    sendLen = Local_DataAdapter( (pbuf->phead)+2,( (pbuf->pend) ) - ( (pbuf->phead)+2 ) );
    Local_SendData( fd, pbuf->phead,sendLen );
    
    return;
}

/****************************************************************
FunctionName    :   GAgent_LocalReceData
Description     :   receive data form local io.
pgc             :   gagent global struct. 
return          :   one packe local data length.
Add by Alex.lin     --2015-04-07
****************************************************************/
int32 GAgent_Local_GetPacket( pgcontext pgc, ppacket Rxbuf )
{
    int32 dataLen=0;
    if( pgc->rtinfo.local.uart_fd<0 )
        return 0;
    if(FD_ISSET( pgc->rtinfo.local.uart_fd,&(pgc->rtinfo.readfd)) )
    {
         int8 ret=0;
         resetPacket( Rxbuf ); 
         dataLen = hal_ReceivepOnePack( pgc->rtinfo.local.uart_fd,Rxbuf->phead );
         ret = Local_DataValidityCheck( pgc->rtinfo.local.uart_fd,Rxbuf,dataLen );
         if( ret!=0 )
         {
            dataLen=0;
         }
         pgc->rtinfo.local.timeoutCnt=0;
    }
    /*
    if( dataLen>0 )
        GAgent_DebugPacket(Rxbuf->phead, dataLen);
        */
    return dataLen;  
}

/****************************************************************
FunctionName    :   GAgent_LocalDataWriteP0
Description     :   send p0 to local io and add 0x55 after 0xff
                    auto.
cmd             :   MCU_CTRL_CMD or WIFI_STATUS2MCU
return          :   0-ok other -error
Add by Alex.lin     --2015-04-07
****************************************************************/
int32 GAgent_LocalDataWriteP0( pgcontext pgc,int32 fd,ppacket pTxBuf,uint8 cmd )
{
    int8 ret =0;
    uint16 datalen = 0;
    uint16 flag = 0;
    uint16 sendLen = 0;
    /* head(0xffff)| len(2B) | cmd(1B) | sn(1B) | flag(2B) |  payload(xB) | checksum(1B) */
    pTxBuf->phead = pTxBuf->ppayload - 8;
    pTxBuf->phead[0] = MCU_HDR_FF;
    pTxBuf->phead[1] = MCU_HDR_FF;
    datalen = pTxBuf->pend - pTxBuf->ppayload + 5;    //p0 + cmd + sn + flag + checksum
    *(uint16 *)(pTxBuf->phead + 2) = htons(datalen);
    pTxBuf->phead[4] = cmd;
    pTxBuf->phead[5] = GAgent_NewSN();
    *(uint16 *)(pTxBuf->phead + 6) = htons(flag);
    *( pTxBuf->pend )  = GAgent_SetCheckSum(pTxBuf->phead, (pTxBuf->pend)-(pTxBuf->phead) );
    pTxBuf->pend += 1;  /* add 1 Byte of checksum */

    sendLen = (pTxBuf->pend) - (pTxBuf->phead);
    sendLen = Local_DataAdapter( (pTxBuf->phead)+2,( (pTxBuf->pend) ) - ( (pTxBuf->phead)+2 ) );
    Local_SendData( fd, pTxBuf->phead,sendLen );

    if(GAgent_CheckAck( fd,pgc,pTxBuf->phead,sendLen,pLocalAckbuf,GAgent_GetDevTime_MS())==0)
    {
        GAgent_Printf( GAGENT_INFO,"%s %d GAgent_CheckAck OK",__FUNCTION__,__LINE__ );
        ret =0;  
    }
    else
    {
        GAgent_Printf( GAGENT_INFO,"%s %d GAgent_CheckAck Fail",__FUNCTION__,__LINE__ );
        ret=1;
    }
    resetPacket( pLocalAckbuf );
    
    return ret;
}
/****************************************************************
FunctionName  :     GAgent_LocalGetInfo
Description   :     get localinfo like pk.
return        :     return 
Add by Alex.lin         --2015-04-18
****************************************************************/
void Local_GetInfo( pgcontext pgc )
{
    uint8 i=0;
    int32 pos=0;
    int8 length =0;
    uint16 *pTime=NULL;
    uint16 *pplength=NULL;
    uint8 get_Mcu_InfoBuf[9]=
    {
        0xff,0xff,0x00,0x05,0x01,0x01,0x00,0x00,0x07
    }; 
    //memset(&mcuAttrData[0],0,sizeof(mcuAttrData) );
    GAgent_DevLED_Green(0);
    get_Mcu_InfoBuf[8]  = GAgent_SetCheckSum( get_Mcu_InfoBuf, 8);
    Local_SendData( pgc->rtinfo.local.uart_fd,get_Mcu_InfoBuf, 9 );

    for( i=0;i<20;i++ )
    {
        if(GAgent_CheckAck( pgc->rtinfo.local.uart_fd,pgc,get_Mcu_InfoBuf,9,pgc->rtinfo.Rxbuf,GAgent_GetDevTime_MS())==0)
        {
            uint8 * Rxbuf=NULL;
            Rxbuf = pgc->rtinfo.Rxbuf->phead;

            pplength = (u16*)&((pgc->rtinfo.Rxbuf->phead +2)[0]);
            length = ntohs(*pplength);
            
            pos+=8;
            memcpy( pgc->mcu.protocol_ver, Rxbuf+pos, MCU_PROTOCOLVER_LEN );
            pgc->mcu.protocol_ver[MCU_PROTOCOLVER_LEN] = '\0';
            pos += MCU_PROTOCOLVER_LEN;

            memcpy( pgc->mcu.p0_ver,Rxbuf+pos, MCU_P0VER_LEN);
            pgc->mcu.p0_ver[MCU_P0VER_LEN] = '\0';
            pos+=MCU_P0VER_LEN;

            memcpy( pgc->mcu.hard_ver,Rxbuf+pos,MCU_HARDVER_LEN);
            pgc->mcu.hard_ver[MCU_HARDVER_LEN] = '\0';
            pos+=MCU_HARDVER_LEN;

            memcpy( pgc->mcu.soft_ver,Rxbuf+pos,MCU_SOFTVER_LEN);
            pgc->mcu.soft_ver[MCU_SOFTVER_LEN] = '\0';
            pos+=MCU_SOFTVER_LEN;

            memcpy( pgc->mcu.product_key,Rxbuf+pos,PK_LEN);
            pgc->mcu.product_key[PK_LEN] = '\0';
            pos+=PK_LEN;

            pTime = (u16*)&Rxbuf[pos];
            pgc->mcu.passcodeEnableTime = ntohs(*pTime);
            pos+=2;


            if( length >= (pos+MCU_MCUATTR_LEN+1 - MCU_P0_LEN - MCU_CMD_LEN) ) //pos+8+1:pos + mcu_attr(8B)+checksum(1B)
            {
                memcpy( pgc->mcu.mcu_attr,Rxbuf+pos, MCU_MCUATTR_LEN);
            }
            else
            {
                memset( pgc->mcu.mcu_attr, 0, MCU_MCUATTR_LEN);
            }

            GAgent_Printf( GAGENT_INFO,"GAgent get local info ok.");
            GAgent_Printf( GAGENT_INFO,"MCU Protocol Vertion:%s.",pgc->mcu.protocol_ver);
            GAgent_Printf( GAGENT_INFO,"MCU P0 Vertion:%s.",pgc->mcu.p0_ver);
            GAgent_Printf( GAGENT_INFO,"MCU Hard Vertion:%s.",pgc->mcu.hard_ver);
            GAgent_Printf( GAGENT_INFO,"MCU Soft Vertion:%s.",pgc->mcu.soft_ver);
            GAgent_Printf( GAGENT_INFO,"MCU old product_key:%s.",pgc->gc.old_productkey);
            GAgent_Printf( GAGENT_INFO,"MCU product_key:%s.",pgc->mcu.product_key);
            GAgent_Printf( GAGENT_INFO,"MCU passcodeEnableTime:%d s.\r\n",pgc->mcu.passcodeEnableTime);
            for( i=0;i<MCU_MCUATTR_LEN;i++ )
            {
                GAgent_Printf( GAGENT_INFO,"MCU mcu_attr[%d]= 0x%x.",i, (uint32)pgc->mcu.mcu_attr[i]);
            }
            
            if( strcmp( (int8 *)pgc->mcu.product_key,pgc->gc.old_productkey )!=0 )
            {
                GAgent_UpdateInfo( pgc,pgc->mcu.product_key );
                GAgent_Printf( GAGENT_INFO,"2 MCU old product_key:%s.",pgc->gc.old_productkey);
            }
            break;
        }
    }
    if( 20==i )
    {
    
        GAgent_Printf( GAGENT_INFO," GAgent get local info fail ... ");
        GAgent_Printf( GAGENT_INFO," Please check your local data,and restart GAgent again !!");
        GAgent_DevReset();
    }
}
/****************************************************************
FunctionName    :   GAgent_Reset
Description     :   update old info and send disable device to 
                    cloud,then reboot(clean the config data,unsafe).
pgc             :   global staruc 
return          :   NULL
Add by Alex.lin     --2015-04-18
****************************************************************/
/* Use this function carefully!!!!!!!!!!!!!!!!!!!! */
void GAgent_Reset( pgcontext pgc )
{
    GAgent_Clean_Config(pgc);
    
    sleep(2);
    
    GAgent_DevReset();
}
/****************************************************************
FunctionName    :   GAgent_Clean_Config
Description     :   GAgent clean the device config                  
pgc             :   global staruc 
return          :   NULL
Add by Frank Liu     --2015-05-08
****************************************************************/
void GAgent_Clean_Config( pgcontext pgc )
{
    memset( pgc->gc.old_did,0,DID_LEN);
    memset( pgc->gc.old_wifipasscode,0,PASSCODE_MAXLEN + 1);
  
    memcpy( pgc->gc.old_did,pgc->gc.DID,DID_LEN );
    memcpy( pgc->gc.old_wifipasscode,pgc->gc.wifipasscode,PASSCODE_MAXLEN + 1 );
    GAgent_Printf(GAGENT_INFO,"Reset GAgent and goto Disable Device !");  
    Cloud_ReqDisable( pgc );
    GAgent_SetCloudConfigStatus( pgc,CLOUD_RES_DISABLE_DID );

    memset( pgc->gc.wifipasscode,0,PASSCODE_MAXLEN + 1);
    memset( pgc->gc.wifi_ssid,0,SSID_LEN_MAX + 1 );
    memset( pgc->gc.wifi_key,0, WIFIKEY_LEN_MAX + 1 );
    memset( pgc->gc.DID,0,DID_LEN);
    
    memset( (uint8*)&(pgc->gc.cloud3info),0,sizeof( GAgent3Cloud ) );
    
    memset( pgc->gc.GServer_ip,0,IP_LEN_MAX + 1);
    memset( pgc->gc.m2m_ip,0,IP_LEN_MAX + 1);
    make_rand(pgc->gc.wifipasscode);

    pgc->gc.flag &=~XPG_CFG_FLAG_CONNECTED;
    GAgent_DevSaveConfigData( &(pgc->gc) );
}
/****************************************************************
        FunctionName        :   GAgent_LocalSendGAgentstatus.
        Description         :   check Gagent's status whether it is update.
        Add by Nik.chen     --2015-04-18
****************************************************************/
void GAgent_LocalSendGAgentstatus(pgcontext pgc,uint32 dTime_s )
{
    uint16 GAgentStatus = 0; 
    uint16 LastGAgentStatus = 0; 
    resetPacket( pgc->rtinfo.Rxbuf );
    if( (pgc->rtinfo.GAgentStatus) != (pgc->rtinfo.lastGAgentStatus) )
    {
          GAgentStatus = pgc->rtinfo.GAgentStatus&LOCAL_GAGENTSTATUS_MASK;
          LastGAgentStatus = pgc->rtinfo.lastGAgentStatus&LOCAL_GAGENTSTATUS_MASK;
          GAgent_Printf( GAGENT_INFO,"GAgentStatus change, lastGAgentStatus=0x%04x, newGAgentStatus=0x%04x", LastGAgentStatus, GAgentStatus);
          pgc->rtinfo.lastGAgentStatus = pgc->rtinfo.GAgentStatus&LOCAL_GAGENTSTATUS_MASK;
          GAgentStatus = htons(GAgentStatus);
          memcpy((pgc->rtinfo.Rxbuf->ppayload), (uint8 *)&GAgentStatus, 2);
          pgc->rtinfo.Rxbuf->pend =  (pgc->rtinfo.Rxbuf->ppayload)+2;
          pgc->rtinfo.updatestatusinterval =  0; 
          //GAgent_Printf(GAGENT_CRITICAL,"updateGagentstatusLast time=%d", (pgc->rtinfo.send2LocalLastTime));
         GAgent_LocalDataWriteP0( pgc,pgc->rtinfo.local.uart_fd, (pgc->rtinfo.Rxbuf), WIFI_STATUS2MCU );
    }

     pgc->rtinfo.updatestatusinterval+= dTime_s;

    if( (pgc->rtinfo.updatestatusinterval)  > LOCAL_GAGENTSTATUS_INTERVAL)
    {
        pgc->rtinfo.updatestatusinterval = 0;
        GAgentStatus = pgc->rtinfo.GAgentStatus&LOCAL_GAGENTSTATUS_MASK;
        GAgentStatus = htons(GAgentStatus);
        memcpy((pgc->rtinfo.Rxbuf->ppayload), (uint8 *)&GAgentStatus, 2);
        pgc->rtinfo.Rxbuf->pend =  (pgc->rtinfo.Rxbuf->ppayload)+2;
        GAgent_LocalDataWriteP0( pgc,pgc->rtinfo.local.uart_fd, (pgc->rtinfo.Rxbuf), WIFI_STATUS2MCU );
    }
}
void GAgent_LocalInit( pgcontext pgc )
{
    GAgent_LocalDataIOInit( pgc );
    Local_HalInit();
    GAgent_RegisterReceiveDataHook( GAgent_Local_GetPacket );
    GAgent_RegisterSendDataHook( serial_write );
    Local_GetInfo( pgc );
}
void GAgent_LocalTick( pgcontext pgc,uint32 dTime_s )
{
    pgc->rtinfo.local.oneShotTimeout+=dTime_s;
    if( pgc->rtinfo.local.oneShotTimeout >= MCU_HEARTBEAT )
    {
        if( pgc->rtinfo.local.timeoutCnt> 3 )
        {
            GAgent_Printf(GAGENT_CRITICAL,"Local heartbeat time out ...");
            GAgent_DevReset();
        }
        else
        {
            pgc->rtinfo.local.oneShotTimeout = 0;
            pgc->rtinfo.local.timeoutCnt++;
            resetPacket( pgc->rtinfo.Rxbuf );
            GAgent_Printf(GAGENT_CRITICAL,"Local ping...");
            GAgent_LocalDataWriteP0( pgc,pgc->rtinfo.local.uart_fd, pgc->rtinfo.Rxbuf,WIFI_PING2MCU );
        }
    }
}
uint32 GAgent_LocalDataHandle( pgcontext pgc,ppacket Rxbuf,int32 RxLen /*,ppacket Txbuf*/ )
{
    int8 cmd=0;
    uint8 sn=0,checksum=0;
    uint8 *localRxbuf=NULL;
    uint32 ret = 0;
    uint8 configType=0;
    _tm tm;
    if( RxLen>0 )
    {
        localRxbuf = Rxbuf->phead;
        
        cmd = localRxbuf[4];
        sn  = localRxbuf[5];
        checksum = GAgent_SetCheckSum( localRxbuf,RxLen-1 );
        if( checksum!=localRxbuf[RxLen-1] )
        {
            GAgent_Printf( GAGENT_ERROR,"local data cmd=%02x checksum error !",cmd );
            GAgent_DebugPacket(Rxbuf->phead, RxLen);
            return 0;
        }

        switch( cmd )
        {
            case MCU_REPORT:
                Local_Ack2MCU( pgc->rtinfo.local.uart_fd,sn,cmd+1 );
                Rxbuf->type = SetPacketType( Rxbuf->type,LOCAL_DATA_IN,1 );
                ParsePacket( Rxbuf );
                ret = 1;
                break;
            case MCU_CONFIG_WIFI:
                Local_Ack2MCU( pgc->rtinfo.local.uart_fd,sn,cmd+1 );
                configType = localRxbuf[8];
                GAgent_Config( configType,pgc );
                ret = 0;
                break;
            case MCU_RESET_WIFI:
                Local_Ack2MCU( pgc->rtinfo.local.uart_fd,sn,cmd+1 );
                GAgent_Clean_Config(pgc);
                sleep(2);
                GAgent_DevReset();
                ret = 0;
                break;
            case WIFI_PING2MCU_ACK:
                pgc->rtinfo.local.timeoutCnt=0;
                GAgent_Printf( GAGENT_CRITICAL,"Local pong...");
                ret = 0 ;
                break;
            case MCU_CTRL_CMD_ACK:
                ret = 0;
                break;
            case WIFI_TEST:
                Local_Ack2MCU( pgc->rtinfo.local.uart_fd,sn,cmd+1 );
                GAgent_EnterTest( pgc );
                ret = 0;
                break;
            case MCU_ENABLE_BIND:
                Local_Ack2MCU( pgc->rtinfo.local.uart_fd,sn,cmd+1 );
                pgc->mcu.passcodeTimeout = pgc->mcu.passcodeEnableTime;
                GAgent_SetWiFiStatus( pgc,WIFI_MODE_BINDING,1 );	
                ret = 0;
                break;
            case MCU_REQ_GSERVER_TIME:	                                                                                            
                tm = GAgent_GetLocalTimeForm(pgc->rtinfo.clock);
                *(uint16 *)(Rxbuf->ppayload) = htons(tm.year);
                Rxbuf->ppayload[2] = tm.month;
                Rxbuf->ppayload[3] = tm.day;
                Rxbuf->ppayload[4] = tm.hour;
                Rxbuf->ppayload[5] = tm.minute;
                Rxbuf->ppayload[6] = tm.second;
                Rxbuf->pend = (Rxbuf->ppayload) + 7;
                Local_Ack2MCUwithP0( Rxbuf, pgc->rtinfo.local.uart_fd, sn, MCU_REQ_GSERVER_TIME_ACK );
                ret = 0;
                break;
            default:
                ret = 0;
                break;
        }
        //...
    }

  return ret;
}
void GAgent_Local_Handle( pgcontext pgc,ppacket Rxbuf,int32 length )
{
    int32 localDataLen=0;

    localDataLen = GAgent_Local_GetPacket( pgc,Rxbuf );
    if( localDataLen>0 )
    {
        uint32 ret;
        
        ret = GAgent_LocalDataHandle( pgc, pgc->rtinfo.Rxbuf, localDataLen );
        if(ret > 0)
        {
            dealPacket(pgc, pgc->rtinfo.Rxbuf);
        }
        
    }

}
