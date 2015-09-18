/****************************************************************
 *  GPRS模组回复MCU请求通讯模组信息 
 ****************************************************************/
#ifndef __GPRS_H_
#define __GPRS_H_
#include "gagent_typedef.h"
#include "gagent.h"

#define CellNumMax 7
typedef struct
{
    uint16 LACID;
    uint16 CellID;
    uint8 RSSI;
}CellInfo_t;

typedef struct
{
   uint8 IMEI[16];
   uint8 IMSI[16];
   uint8 MCC[8];
   uint8 MNC[8];
   uint8 CellNum;
   uint8 CellInfoLen;  
   CellInfo_t CellInfo[CellNumMax];
}GprsInfo_t,*pGprsInfo_t;

uint32 GAgent_getGprsInfo(GprsInfo_t **GprsInfo);
uint32 GAgent_sendGprsInfo( pgcontext pgc );

#endif

