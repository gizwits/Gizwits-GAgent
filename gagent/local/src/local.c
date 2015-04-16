#include "gagent.h"
#include "utils.h"

#define ACKBUF_LEN 1024
ppacket pLocalAckbuf=NULL;

pfMasterMCU_ReciveData PF_ReceiveDataformMCU = NULL;
pfMasertMCU_SendData   PF_SendData2MCU = NULL;


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
    uint8 *p_start=NULL,*p_end=NULL;

    len = MCU_LEN_NO_PIECE;
    len = dataLen;

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
    pLocalAckbuf->allbuf = malloc( totalCap );
    while( pLocalAckbuf->allbuf==NULL )
    {
        pLocalAckbuf->allbuf = malloc( totalCap );
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
        PF_SendData2MCU( fd,pData,bufferMaxLen );
    }
    return 0;
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
    if(FD_ISSET( pgc->rtinfo.uart_fd,&(pgc->rtinfo.readfd)) )
    {
         resetPacket( Rxbuf ); 
         dataLen = hal_ReceivepOnePack( pgc->rtinfo.uart_fd,Rxbuf->phead );
    }
    return dataLen;  
}
int32 GAgent_LocalDataWriteP0( pgcontext pgc,int32 fd,ppacket pTxBuf )
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
    pTxBuf->phead[4] = MCU_CTRL_CMD;
    pTxBuf->phead[5] = GAgent_NewSN();
    *(uint16 *)(pTxBuf->phead + 6) = htons(flag);
    *(pTxBuf->pend) = GAgent_SetCheckSum(pTxBuf->phead, pTxBuf->pend - pTxBuf->phead);
    pTxBuf->pend += 1;  /* add 1 Byte of checksum */

    sendLen = (pTxBuf->pend) - (pTxBuf->phead);

    //sendLen = Local_DataAdapter( pTxBuf->ppayload,( (pTxBuf->pend)-1 ) - (pTxBuf->ppayload) );
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
FunctionName  :  GAgent_LocalGetInfo
Description   :   get localinfo like pk..

****************************************************************/
void Local_GetInfo( pgcontext pgc )
{
    int8 i=0;
    int32 pos=0;
    uint16 *pTime=NULL;
    uint8 get_Mcu_InfoBuf[9]=
    {
        0xff,0xff,0x00,0x05,0x01,0x01,0x00,0x00,0x07
    }; 
    GAgent_DevLED_Green(0);
    get_Mcu_InfoBuf[8]  = GAgent_SetCheckSum( get_Mcu_InfoBuf, 8);
    Local_SendData( pgc->rtinfo.uart_fd,get_Mcu_InfoBuf, 9 );

    for( i=0;i<20;i++ )
    {
        if(GAgent_CheckAck( pgc->rtinfo.uart_fd,pgc,get_Mcu_InfoBuf,9,pgc->rtinfo.Rxbuf,GAgent_GetDevTime_MS())==0)
        {
            int8 * Rxbuf=NULL;
            Rxbuf = pgc->rtinfo.Rxbuf->phead;
            pos+=8;
            memcpy( pgc->mcu.protocol_ver,Rxbuf+pos,8);
            pgc->mcu.protocol_ver[8] = '\0';
            pos+=8;

            memcpy( pgc->mcu.p0_ver,Rxbuf+pos,8);
            pgc->mcu.p0_ver[8] = '\0';
            pos+=8;

            memcpy( pgc->mcu.hard_ver,Rxbuf+pos,8);
            pgc->mcu.hard_ver[8] = '\0';
            pos+=8;

            memcpy( pgc->mcu.soft_ver,Rxbuf+pos,8);
            pgc->mcu.soft_ver[8] = '\0';
            pos+=8;

            memcpy( pgc->mcu.product_key,Rxbuf+pos,32);
            pgc->mcu.product_key[32] = '\0';
            pos+=32;

            pTime = (u16*)&Rxbuf[pos];
            pgc->mcu.passcodeEnableTime = ntohs(*pTime);

            GAgent_Printf( GAGENT_INFO,"GAgent get local info ok.");
            GAgent_Printf( GAGENT_INFO,"MCU Protocol Vertion:%s.",pgc->mcu.protocol_ver);
            GAgent_Printf( GAGENT_INFO,"MCU P0 Vertion:%s.",pgc->mcu.p0_ver);
            GAgent_Printf( GAGENT_INFO,"MCU Hard Vertion:%s.",pgc->mcu.hard_ver);
            GAgent_Printf( GAGENT_INFO,"MCU Soft Vertion:%s.",pgc->mcu.soft_ver);
            GAgent_Printf( GAGENT_INFO,"MCU old product_key:%s.",pgc->gc.old_productkey);
            GAgent_Printf( GAGENT_INFO,"MCU product_key:%s.",pgc->mcu.product_key);
            GAgent_Printf( GAGENT_INFO,"MCU passcodeEnableTime:%d s.\r\n",pgc->mcu.passcodeEnableTime);
            if( strcmp( pgc->mcu.product_key,pgc->gc.old_productkey )!=0 )
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
void GAgent_LocalInit( pgcontext pgc )
{
    GAgent_LocalDataIOInit( pgc );
    Local_HalInit();
    GAgent_RegisterReceiveDataHook( GAgent_Local_GetPacket );
    GAgent_RegisterSendDataHook( serial_write );
    Local_GetInfo( pgc );
}
uint32 GAgent_LocalDataHandle( pgcontext pgc,ppacket Rxbuf,int32 RxLen /*,ppacket Txbuf*/ )
{
    int32 i=0;
    int8 cmd=0;
    uint8 sn=0,checksum=0;
    uint8 *localRxbuf=NULL;
    uint32 ret = 0;
  
    if( RxLen>0 )
    {
        localRxbuf = Rxbuf->phead;
        
        //TODO 
        cmd = localRxbuf[4];
        sn  = localRxbuf[5];
        checksum = GAgent_SetCheckSum( localRxbuf,RxLen-1 );
        if( checksum!=localRxbuf[RxLen-1] )
        {
            GAgent_Printf( GAGENT_ERROR,"local data checksum error !");
            //  Todo tell mcu 
            return 1;
        }
        switch( cmd )
        {
            case MCU_REPORT:
                Rxbuf->type = SetPacketType( Rxbuf->type,LOCAL_DATA_IN,1 );
                ParsePacket( Rxbuf );
                Rxbuf->type = SetPacketType( Rxbuf->type,CLOUD_DATA_OUT,1 );
                Rxbuf->type = SetPacketType( Rxbuf->type,LAN_TCP_DATA_OUT,1 );
                ret = 1;
                break;
            case MCU_CONFIG_WIFI:
                ret = 0;
                break;
            case MCU_RESET_WIFI:
                ret = 0;
                break;
            case WIFI_PING2MCU_ACK:
               ret = 0 ;
                break;
            case MCU_CTRL_CMD_ACK:
                ret = 0;
                 break;
            case WIFI_TEST:
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
