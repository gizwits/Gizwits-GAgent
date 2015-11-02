#ifndef _LOCAL_H_
#define _LOCAL_H_

#define GAGENT_MCU_CHECKSUM_ERROR  0x01
#define GAGENT_MCU_CMD_ERROR       0x02
#define GAGENT_MCU_OTHER_ERROR     0x03
#define GAGENT_MCU_FILETPYE_ERROR  0x04

#define DAY_SEC      (24*60*60)
#define Eastern8th   (8*60*60)

#define MCU_MD5_UNMATCH           -1
#define MCU_FIRMWARE_TYPE_UNMATCH -2
#define MCU_FIRMWARE_TYPE_HEX      1
#define MCU_FIRMWARE_TYPE_BIN      2

typedef struct 
{
    uint16 year;
    uint8 month;
    uint8 day; 
    uint8 hour;
    uint8 minute;
    uint8 second;  
    uint32 ntp;
}_tm;

/*void GAgent_LocalInit( pgcontext pgc );*/
void GAgent_MoveOneByte( uint8 *pData,int32 dataLen,uint8 flag );
uint32 Local_SendData( int32 fd,uint8 *pData, int32 bufferMaxLen );
int32 GAgent_LocalSendUpgrade( pgcontext pgc, int32 fd, ppacket pTxBuf, int16 piecelen, uint8 cmd );
int32 Local_DataAdapter( /*uint8 cmd,*/ uint8 *pData,int32 dataLen /*,uint8 **pdest*/ );
void Local_HalInit( pgcontext pgc );
void Local_GetInfo( pgcontext pgc );
void Local_Ack2MCU( int32 fd,uint8 sn,uint8 cmd );
void Local_Ack2MCUwithP0( ppacket pbuf,int32 fd,uint8 sn,uint8 cmd );
int32 GAgent_Local_ExtractOnePacket(uint8 *buf);
int32 trans_sendotadownloadresult(pgcontext pgc, u8 result);
int32 trans_startmcuota(pgcontext pgc, u8 *url);
int32 trans_sendfirmwareinfo(pgcontext pgc, u8 *sv, u8 *url);
int32 trans_checkmcuota(pgcontext pgc, u8 *sv, u8 *url);
#endif