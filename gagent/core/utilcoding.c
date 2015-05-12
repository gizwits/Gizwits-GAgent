#include "gagent.h"
#include "lan.h"
static uint8 g_SN;

void make_rand(int8* data)
{
    int i;
    srand(GAgent_GetDevTime_MS());
    memset(data,0,10);
	
    for(i=0;i<10;i++)
    {
        data[i] = 65+rand()%(90-65);
    }
    data[10] = '\0';
    GAgent_Printf(GAGENT_INFO,"passcode:%s",data );
}

// 将数字编码为可变长度
// 返回长度
int encodevarlen(int data, u8* buf)
{
    int l = 0, b = 0, ret = 0;
    do
    {
        buf[l] = data % 128;
        data = data / 128;
        if(data > 0)
        {
            buf[l] |= 128;
        }
        l++;
    }
    while(data > 0);
    return l;
}
// 将可变长度解码为数字
// 返回长度
int decodevarlen(u8* buf, int *data)
{
    int len = 0, t = 1;
    u8 c;
    *data = 0;
    do
    {
        c = buf[len++];
        *data += (c & 0x7F) * t;
        t *= 128;
    }
    while((c & 128) != 0);
    return len;
}

void GAgent_String2MAC( uint8* string, uint8* mac )
{

    int j=0,i;

    for(i=0;i<12;i++)
    {
        if (string[i] >= 97)
        {
            string[i] -= 87; /*X -a +10 */
        }
        else if(string[i] >= 65)
        {
            string[i] = string[i] - 55;  /*X -A +10 */
        }
        else
        {
            string[i] = string[i] - 48;
        }

        if( i%2==1 )
        {
            mac[j] = string[i-1]<<4 | string[i];
            j++;
        }
    }

    return;
}
/*********************************************************
*
*       buf     :   need to get checksum buf, need not include the checksum of 
                    received package;
*       bufLen  :   bufLen      
*       return  :   checksum
*                   add by Alex lin 2014-09-02
**********************************************************/
uint8 GAgent_SetCheckSum(  uint8 *buf,int packLen )
{
    uint8 checkSum=0;
    int i;
    
    /*i=2, remove the first two byte*/
    // 如果传入的buf是从第二个字节开始，则可以和下面的合成一个函数
    for( i=2;i<(packLen);i++ )
    {
        checkSum = checkSum+buf[i];
    }

    return checkSum;
}
int8 GAgent_CheckSum( int8 *data,int dataLen )
{
    uint8 checksum=0;
    int i=0;
    for( i=0;i<dataLen;i++ )
      checksum+=data[i];

    return checksum;
}
/************************************************
*
*       buf :   data need to send to mcu
*       return :  the sn number 
*       Add by Alex lin     --2014-09-03
*
*************************************************/
uint8 GAgent_SetSN( uint8 *buf )
{
    if( g_SN>=255 ) g_SN=1;

    buf[MCU_SN_POS]=g_SN;
    g_SN++;
    return buf[MCU_SN_POS];
}
uint8 GAgent_NewSN(void)
{
    return g_SN++;
}
/*************************************************
*
*       FUNCTION : transtion u16 to 2 byte for mqtt remainlen
*       remainLen: src data to transtion
*       return   : varc
*       Add by alex 2014-04-20
***************************************************/
varc Tran2varc(uint32 remainLen)
{
    int i;
	varc Tmp;
	
    memset(&Tmp, 0x0, sizeof(Tmp));

	Tmp.varcbty = 1;
	for(i = 0; i < 4; i++)
	{
		Tmp.var[i] = remainLen % 128;
		remainLen >>= 7;
		if(remainLen)
		{
			Tmp.var[i] |= 0x80;
			Tmp.varcbty++;
		}
		else
		{
			break;
		}
	}
    return Tmp;
}

/***************************************************
FunctionName    :   SetPacketType
Description     :   set addtype to packet type.
currentType     :   current packet msg type
type            :   type
flag            :   0 reset type. 1 set type.
return          :   the type afet add newtype
Add by Alex.lin     --2015-04-07
***************************************************/
int32 SetPacketType( int32 currentType,int32 type,int8 flag )
{
    if( flag==0 )
    {
        currentType &=~ type;
    }
    else
    {
        currentType |=type;
    }
    return currentType;
}

/* return 1 type set */
int8 isPacketTypeSet( int32 currentType,int32 type )
{
    if( ( currentType & type ) == type )
    {
        return 1;
    }
    else
    { 
        return 0;
    }
}
/***************************************************
        FunctionName    :   resetPacket
        Description     :   let ppacket phead ppayload pend pointer 
                            in satrt status.
        return          :   NULL.
        Add by Alex.lin 2015-03-19
***************************************************/
void resetPacket( ppacket pbuf )
{
    pbuf->phead  = pbuf->allbuf+BUF_HEADLEN;
    pbuf->ppayload = pbuf->allbuf+BUF_HEADLEN;
    pbuf->pend   = pbuf->allbuf+BUF_HEADLEN;
    memset( pbuf->allbuf,0,pbuf->totalcap );
}

/***************************************************
        FunctionName    :   ParsePacket.
        Description     :   set the source phead ppayload
                            pend.
        pbug            :   data source struct.
        return          :   0 ok other fail.
        Add by Alex.lin     --2015-03-21
***************************************************/
uint32 ParsePacket( ppacket pRxBuf )
{
    int32 varlen=0;
    int32 datalen=0;
    uint8* pHead=NULL;
    int32 ret=0;

    uint16 cmd=0;
    uint16 *pcmd=NULL;
    GAgent_Printf(GAGENT_DEBUG,"\r\n");
    GAgent_Printf(GAGENT_DEBUG,"IN %s packet type : %04x",__FUNCTION__ ,pRxBuf->type );
    if( ((pRxBuf->type)&(CLOUD_DATA_IN)) == CLOUD_DATA_IN )
    {

        datalen = mqtt_parse_rem_len( pRxBuf->phead+3 ); 
        varlen = mqtt_num_rem_len_bytes( pRxBuf->phead+3 );
        
        pcmd = (u16*)&(pRxBuf->phead[4+varlen+1]);
        cmd = ntohs( *pcmd );  

        GAgent_Printf( GAGENT_INFO,"CLOUD_DATA_IN cmd : %04X", cmd );
        if( cmd == 0x0090 )
        {
            pRxBuf->ppayload = pRxBuf->phead+4+varlen+1+2;
        }
        if( cmd ==0x0093 )
        {//with sn.
            pRxBuf->ppayload = pRxBuf->phead+4+varlen+1+2+4;          
        }

        pRxBuf->pend   = pRxBuf->phead+4+varlen+datalen;  

        GAgent_Printf( GAGENT_DEBUG," ReSet Data Type : %04X - CLOUD_DATA_IN", pRxBuf->type );
        pRxBuf->type = SetPacketType( pRxBuf->type,CLOUD_DATA_IN,0 );
        pRxBuf->type = SetPacketType( pRxBuf->type,LOCAL_DATA_OUT,1 );
        GAgent_Printf( GAGENT_DEBUG," Set Data Type : %04X - LOCAL_DATA_OUT", pRxBuf->type );
    }
    else if( ((pRxBuf->type)&(LOCAL_DATA_IN)) == LOCAL_DATA_IN )
    {
        /* head(0xffff)| len(2B) | cmd(1B) | sn(1B) | flag(2B) |  payload(xB) | checksum(1B) */
        pRxBuf->ppayload = pRxBuf->phead+8;   /* head + len + cmd + sn + flag */
        datalen = ( (int32)ntohs( *(uint16 *)(pRxBuf->phead + 2) ) ) & 0xffff;
        pRxBuf->pend =  (pRxBuf->phead )+( datalen+4 ); /* datalen + head + len */

        GAgent_Printf( GAGENT_DEBUG," ReSet Data Type : %04X - LOCAL_DATA_IN", pRxBuf->type );
        pRxBuf->type = SetPacketType( pRxBuf->type,LOCAL_DATA_IN,0 );
        pRxBuf->type = SetPacketType( pRxBuf->type,CLOUD_DATA_OUT,1 );
        pRxBuf->type = SetPacketType( pRxBuf->type,LAN_TCP_DATA_OUT,1 );
        GAgent_Printf( GAGENT_DEBUG," Set Data Type : %04X - CLOUD_DATA_OUT & LAN_TCP_DATA_OUT ",pRxBuf->type );
    }
    else if( ((pRxBuf->type)&(LAN_TCP_DATA_IN)) == LAN_TCP_DATA_IN )
    {
        datalen = mqtt_parse_rem_len( pRxBuf->phead+3 ); 
        varlen = mqtt_num_rem_len_bytes( pRxBuf->phead+3 );
        
        pRxBuf->ppayload = pRxBuf->phead + LAN_PROTOCOL_HEAD_LEN + varlen + LAN_PROTOCOL_FLAG_LEN + LAN_PROTOCOL_CMD_LEN;
        pRxBuf->pend   = pRxBuf->phead + LAN_PROTOCOL_HEAD_LEN + varlen + datalen;

        GAgent_Printf( GAGENT_DEBUG," ReSet Data Type : %04X - LAN_TCP_DATA_IN", pRxBuf->type );
        pRxBuf->type   = SetPacketType( pRxBuf->type,LAN_TCP_DATA_IN,0 );
        pRxBuf->type = SetPacketType( pRxBuf->type,LOCAL_DATA_OUT,1 );
        GAgent_Printf( GAGENT_DEBUG," Set Data Type : %04X - LOCAL_DATA_OUT", pRxBuf->type );
    }
    else
    {
        GAgent_Printf( GAGENT_DEBUG,"Data Type error,wite :%04X ", pRxBuf->type );
        return 1;
    }
    GAgent_Printf( GAGENT_DEBUG,"OUT packet type : %04X\r\n",pRxBuf->type );
    return 0;
}
int8 dealPacket( pgcontext pgc, ppacket pTxBuf )
{
    GAgent_Printf(GAGENT_DEBUG,"\r\n");
    GAgent_Printf(GAGENT_DEBUG,"IN %s packet type : %04x",__FUNCTION__ ,pTxBuf->type );
    if( ((pTxBuf->type)&(LOCAL_DATA_OUT)) == LOCAL_DATA_OUT )
    {
        GAgent_Printf( GAGENT_DEBUG,"packet Type : LOCAL_DATA_OUT ");
        GAgent_LocalDataWriteP0( pgc,pgc->rtinfo.local.uart_fd, pTxBuf,MCU_CTRL_CMD );
        GAgent_Printf( GAGENT_DEBUG,"ReSetpacket Type : LOCAL_DATA_OUT ");
        pTxBuf->type = SetPacketType(pTxBuf->type, LOCAL_DATA_OUT, 0);

    }
    if( ((pTxBuf->type)&(CLOUD_DATA_OUT)) == CLOUD_DATA_OUT )
    {
        if( ((pgc->rtinfo.GAgentStatus)&WIFI_CLOUD_CONNECTED)  == WIFI_CLOUD_CONNECTED )
        {
            GAgent_Printf( GAGENT_DEBUG,"packet Type : CLOUD_DATA_OUT ");
            GAgent_Cloud_SendData(  pgc,pTxBuf,(pTxBuf->pend)-(pTxBuf->ppayload) );
            GAgent_Printf( GAGENT_DEBUG,"ReSetpacket Type : CLOUD_DATA_OUT ");
        }
        pTxBuf->type = SetPacketType(pTxBuf->type, CLOUD_DATA_OUT, 0);
    }
    if( ((pTxBuf->type)&(LAN_TCP_DATA_OUT)) == LAN_TCP_DATA_OUT )
    {
        GAgent_Lan_SendTcpData(pgc, pTxBuf);
        pTxBuf->type = SetPacketType(pTxBuf->type, LAN_TCP_DATA_OUT, 0);
    }
    GAgent_Printf( GAGENT_DEBUG,"OUT packet type : %04X\r\n",pTxBuf->type );
    return 0;
}


