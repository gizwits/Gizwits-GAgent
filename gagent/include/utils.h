#ifndef _UTILS_H_
#define _UTILS_H_
#include "gagent.h"

//int8* GAgent_strstr(const int8 *s1, const int8 *s2);
void make_rand(int8* data );
varc Tran2varc(uint32 remainLen);
void resetPacket( ppacket pbuf );
void copyPacket( ppacket src,ppacket dest );
uint32 ParsePacket( ppacket pbuf );
int8 dealPacket( pgcontext pgc, ppacket pTxBuf );
int32 BuildPacket( ppacket pbuf,int32 type );
uint8 GAgent_SetCheckSum(  uint8 *buf,int packLen );
uint8 GAgent_SetSN( uint8 *buf );

#endif