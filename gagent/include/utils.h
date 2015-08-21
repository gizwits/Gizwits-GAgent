#ifndef _UTILS_H_
#define _UTILS_H_
#include "gagent.h"

//int8* GAgent_strstr(const int8 *s1, const int8 *s2);
void make_rand(int8* data );
varc Tran2varc(uint32 remainLen);
void resetPacket( ppacket pbuf );
uint32 ParsePacket( ppacket pbuf );
int8 dealPacket( pgcontext pgc, ppacket pTxBuf );
void copyPacket(ppacket psrcPacket, ppacket pdestPacket);
int32 BuildPacket( ppacket pbuf,int32 type );
uint8 GAgent_SetCheckSum(  uint8 *buf,int packLen );
uint8 GAgent_SetSN( uint8 *buf );
int32 SetPacketType( int32 currentType,int32 type,int8 flag );
int8 isPacketTypeSet( int32 currentType,int32 type );
void GAgent_Printf(unsigned int level, char *fmt, ...);
uint8 GAgent_NewSN(void);
void GAgent_DebugPacket(unsigned char *pData, int len);
void clearChannelAttrs(pgcontext pgc);
void setChannelAttrs(pgcontext pgc, stCloudAttrs_t *cloudClient, stLanAttrs_t *lanClient, uint8 isBroadCast);
#endif