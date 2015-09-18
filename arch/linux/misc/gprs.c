#include "gprs.h"

/****************************************************************
        FunctionName  :  GAgent_getGprsInfo.
        Description      :  get GRPS module info,this function need return immediately.      
        return             :  0 successful other fail.
****************************************************************/
uint32 GAgent_getGprsInfo(GprsInfo_t **GprsInfo)
{
    return RET_SUCCESS;
}
uint32 GAgent_sendGprsInfo( pgcontext pgc )
{   
    int32 i,pos = 0;
    GprsInfo_t GprsInfo;
    pGprsInfo_t pgprsInfo;
    pgprsInfo = &GprsInfo;
    
    memset(&GprsInfo,0,sizeof(GprsInfo_t));
    if(0 != GAgent_getGprsInfo(&pgprsInfo))
    {
        GAgent_Printf(GAGENT_WARNING, "gagent get GPRS info failed!");
        return RET_FAILED;
    }
    
    /* ModuleType */
    pgc->rtinfo.Txbuf->ppayload[0] = 0x02;
    pos += 1;
    
    /* MCU_PROTOCOLVER */
    memcpy( pgc->rtinfo.Txbuf->ppayload+pos, pgc->mcu.protocol_ver, MCU_PROTOCOLVER_LEN );
    pos += MCU_PROTOCOLVER_LEN;
    
    /* HARDVER */
    memcpy( pgc->rtinfo.Txbuf->ppayload+pos, WIFI_HARDVER, 8 );
    pos += 8;
    
    /* SOFTVAR */
    memcpy( pgc->rtinfo.Txbuf->ppayload+pos, WIFI_SOFTVAR, 8 );
    pos += 8;

    /* MCU_MCUATTR */
    memcpy( pgc->rtinfo.Txbuf->ppayload+pos, pgc->mcu.mcu_attr, MCU_MCUATTR_LEN);
    pos += MCU_MCUATTR_LEN;

    /* IMEI */
    memcpy( pgc->rtinfo.Txbuf->ppayload+pos, GprsInfo.IMSI, 16 );
    pos += 16;

    /* IMSI */
    memcpy( pgc->rtinfo.Txbuf->ppayload+pos, GprsInfo.IMSI, 16 );
    pos += 16;

    /* MCC */
    memset(pgc->rtinfo.Txbuf->ppayload+pos, 0 , 8);
    memcpy( pgc->rtinfo.Txbuf->ppayload+pos, GprsInfo.MCC, strlen(GprsInfo.MCC) );
    pos += 8;
    
    /* MNC */
    memset(pgc->rtinfo.Txbuf->ppayload+pos, 0 , 8);
    memcpy( pgc->rtinfo.Txbuf->ppayload+pos, GprsInfo.MNC, strlen(GprsInfo.MNC) );
    pos += 8;
    
    /* CellNum */
    pgc->rtinfo.Txbuf->ppayload[pos] = GprsInfo.CellNum;
    pos += 1;

    /* CellInfoLen */
    pgc->rtinfo.Txbuf->ppayload[pos]  = GprsInfo.CellInfoLen;
    pos += 1;

    /* CellInfo*/
    for(i=0; i<GprsInfo.CellNum; i++)
    {
        *(uint16 *)(pgc->rtinfo.Txbuf->ppayload+pos) = htons(GprsInfo.CellInfo[i].LACID);
        pos += 2;
        *(uint16 *)(pgc->rtinfo.Txbuf->ppayload+pos) = htons(GprsInfo.CellInfo[i].CellID);
        pos += 2;
        pgc->rtinfo.Txbuf->ppayload[pos]  = GprsInfo.CellInfo[i].RSSI;
        pos += 1;
    }
    
    pgc->rtinfo.Txbuf->pend = pgc->rtinfo.Txbuf->ppayload + pos;
    return RET_SUCCESS;
}

