#include "gagent.h"
#include "utils.h"
#include "lan.h"
#include "local.h"
#include "hal_receive.h"
#include "cloud.h"
/****************************************************************
FunctionName    :   GAgent_LocalReceData
Description     :   receive data form local io.
pgc             :   gagent global struct. 
return          :   one packe local data length.
Add by Alex.lin     --2015-04-07
****************************************************************/
int32 GAgent_Local_GetPacket( pgcontext pgc, ppacket Rxbuf )
{
    return 0;  
}
/****************************************************************
FunctionName    :   GAgent_LocalDataWriteP0
Description     :   send p0 to local io and add 0x55 after 0xff
                    auto.
cmd             :   MCU_CTRL_CMD or WIFI_STATUS2MCU
return          :   0-ok other -error
Add by Alex.lin     --2015-04-07
****************************************************************/
int32 GAgent_LocalDataWriteP0( pgcontext pgc,int32 fd,ppacket pTxBuf,uint8 cmd )
{
    return 0;
}
/****************************************************************
FunctionName  :     GAgent_LocalGetInfo
Description   :     get localinfo like pk.
return        :     return 
Add by Alex.lin         --2015-04-18
****************************************************************/
void Local_GetInfo( pgcontext pgc )
{
    int8 *mcu_protocol_ver = "00000001";
    int8 *mcu_p0_ver = "00000002";
    int8 *mcu_hard_ver = "00000003";
    int8 *mcu_soft_ver = "00000004";
    int8 *mcu_product_key = "6f3074fe43894547a4f1314bd7e3ae0b";
    uint16 mcu_passcodeEnableTime = 0;

    strcpy((char *)pgc->mcu.hard_ver,mcu_hard_ver);
    strcpy((char *)pgc->mcu.soft_ver,mcu_soft_ver);
    strcpy((char *)pgc->mcu.p0_ver,mcu_p0_ver);
    strcpy((char *)pgc->mcu.protocol_ver,mcu_protocol_ver);
    strcpy((char *)pgc->mcu.product_key,mcu_product_key);
    pgc->mcu.passcodeEnableTime = mcu_passcodeEnableTime;
    pgc->mcu.passcodeTimeout = pgc->mcu.passcodeEnableTime;
    GAgent_Printf(GAGENT_INFO,"GAgent_get hard_ver: %s.",pgc->mcu.hard_ver);
    GAgent_Printf(GAGENT_INFO,"GAgent_get soft_ver: %s.",pgc->mcu.soft_ver);
    GAgent_Printf(GAGENT_INFO,"GAgent_get p0_ver: %s.",pgc->mcu.p0_ver);
    GAgent_Printf(GAGENT_INFO,"GAgent_get protocal_ver: %s.",pgc->mcu.protocol_ver);
    GAgent_Printf(GAGENT_INFO,"GAgent_get product_key: %s.",pgc->mcu.product_key);
}
/****************************************************************
FunctionName    :   GAgent_Reset
Description     :   update old info and send disable device to 
                    cloud,then reboot(clean the config data,unsafe).
pgc             :   global staruc 
return          :   NULL
Add by Alex.lin     --2015-04-18
****************************************************************/
/* Use this function carefully!!!!!!!!!!!!!!!!!!!! */
void GAgent_Reset( pgcontext pgc )
{
    GAgent_Clean_Config(pgc); 
    sleep(2);    
    GAgent_DevReset();
}
/****************************************************************
FunctionName    :   GAgent_Clean_Config
Description     :   GAgent clean the device config                  
pgc             :   global staruc 
return          :   NULL
Add by Frank Liu     --2015-05-08
****************************************************************/
void GAgent_Clean_Config( pgcontext pgc )
{
    memset( pgc->gc.old_did,0,DID_LEN);
    memset( pgc->gc.old_wifipasscode,0,PASSCODE_MAXLEN + 1);
  
    memcpy( pgc->gc.old_did,pgc->gc.DID,DID_LEN );
    memcpy( pgc->gc.old_wifipasscode,pgc->gc.wifipasscode,PASSCODE_MAXLEN + 1 );
    GAgent_Printf(GAGENT_INFO,"Reset GAgent and goto Disable Device !");  
    Cloud_ReqDisable( pgc );
    GAgent_SetCloudConfigStatus( pgc,CLOUD_RES_DISABLE_DID );

    memset( pgc->gc.wifipasscode,0,PASSCODE_MAXLEN + 1);
    memset( pgc->gc.wifi_ssid,0,SSID_LEN_MAX + 1 );
    memset( pgc->gc.wifi_key,0, WIFIKEY_LEN_MAX + 1 );
    memset( pgc->gc.DID,0,DID_LEN);
    
    memset( (uint8*)&(pgc->gc.cloud3info),0,sizeof( GAgent3Cloud ) );
    
    memset( pgc->gc.GServer_ip,0,IP_LEN_MAX + 1);
    memset( pgc->gc.m2m_ip,0,IP_LEN_MAX + 1);
    make_rand(pgc->gc.wifipasscode);

    pgc->gc.flag &=~XPG_CFG_FLAG_CONNECTED;
    GAgent_DevSaveConfigData( &(pgc->gc) );
}
/****************************************************************
        FunctionName        :   GAgent_LocalSendGAgentstatus.
        Description         :   check Gagent's status whether it is update.
        Add by Nik.chen     --2015-04-18
****************************************************************/
void GAgent_LocalSendGAgentstatus(pgcontext pgc,uint32 dTime_s )
{
    return;
}
void GAgent_LocalInit( pgcontext pgc )
{
    Local_GetInfo( pgc );
}
void GAgent_LocalTick( pgcontext pgc,uint32 dTime_s )
{
    return;
}
void GAgent_Local_Handle( pgcontext pgc,ppacket Rxbuf,int32 length )
{
    return;
}



