#ifndef _LOCAL_H_
#define _LOCAL_H_

/*void GAgent_LocalInit( pgcontext pgc );*/
void GAgent_MoveOneByte( uint8 *pData,int32 dataLen,uint8 flag );
uint32 Local_SendData( int32 fd,uint8 *pData, int32 bufferMaxLen );
int32 Local_DataAdapter( /*uint8 cmd,*/ uint8 *pData,int32 dataLen /*,uint8 **pdest*/ );
void Local_HalInit();
void Local_GetInfo( pgcontext pgc );
int Local_Ack2MCU( int32 fd,uint8 sn,uint8 cmd );

#endif