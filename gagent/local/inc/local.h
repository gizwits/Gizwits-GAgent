#ifndef _LOCAL_H_
#define _LOCAL_H_

#define GAGENT_MCU_CHECKSUM_ERROR  0x01
#define GAGENT_MCU_CMD_ERROR       0x02
#define GAGENT_MCU_OTHER_ERROR     0x03

#define DAY_SEC      (24*60*60)
#define Eastern8th   (8*60*60)

typedef struct 
{
    uint16 year;
    uint8 month;
    uint8 day; 
    uint8 hour;
    uint8 minute;
    uint8 second;   
}_tm;

/*void GAgent_LocalInit( pgcontext pgc );*/
void GAgent_MoveOneByte( uint8 *pData,int32 dataLen,uint8 flag );
uint32 Local_SendData( int32 fd,uint8 *pData, int32 bufferMaxLen );
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