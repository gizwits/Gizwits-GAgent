#ifndef _LOCAL_H_
#define _LOCAL_H_

#define GAGENT_MCU_CHECKSUM_ERROR  0x01
#define GAGENT_MCU_CMD_ERROR       0x02
#define GAGENT_MCU_OTHER_ERROR     0x03

/*void GAgent_LocalInit( pgcontext pgc );*/
void GAgent_MoveOneByte( uint8 *pData,int32 dataLen,uint8 flag );
uint32 Local_SendData( int32 fd,uint8 *pData, int32 bufferMaxLen );
int32 Local_DataAdapter( /*uint8 cmd,*/ uint8 *pData,int32 dataLen /*,uint8 **pdest*/ );
void Local_HalInit();
void Local_GetInfo( pgcontext pgc );
void Local_Ack2MCU( int32 fd,uint8 sn,uint8 cmd );
void Local_Ack2MCU_Illegal( int32 fd,uint8 sn,uint8 flag );

#endif