#include "gagent.h"
/*
GAGENT_CONFIG_S g_stGAgentConfigData;
XPG_GLOBALVAR g_globalvar;
*/
pgcontext pgContextData=NULL;
void GAgent_NewVar( pgcontext *pgc );

/****************************************************************
Function    :   GAgent_Init
Description :   GAgent init 
pgc         :   global staruc pointer.
return      :   NULL
Add by Alex.lin     --2015-03-27
****************************************************************/
void GAgent_Init( pgcontext *pgc )
{
    GAgent_DevInit( *pgc );
    GAgent_NewVar( pgc );
    GAgent_logevelSet( /*GAGENT_DUMP*/ GAGENT_WARNING );

    GAgent_VarInit( pgc );
    GAgent_LocalInit( *pgc );
    GAgent_LANInit(*pgc);
    GAgent_WiFiInit( *pgc );
    
    GAgent_Printf( GAGENT_CRITICAL,"GAgent Start...");
}
/****************************************************************
Function    :   GAgent_NewVar
Description :   malloc New Var 
pgc         :   global staruc pointer.
return      :   NULL
Add by Alex.lin     --2015-03-27
****************************************************************/
void GAgent_NewVar( pgcontext *pgc )
{
    *pgc = (pgcontext)malloc( sizeof( gcontext ));
    while(NULL == *pgc)
    {
        *pgc = (pgcontext)malloc( sizeof( gcontext ));
        sleep(1);
    }
    memset(*pgc,0,sizeof(gcontext) );

    return ;
}

/****************************************************************
Function    :   GAgent_VarInit
Description :   init global var and malloc memory 
pgc         :   global staruc pointer.
return      :   NULL
Add by Alex.lin     --2015-03-27
****************************************************************/
void GAgent_VarInit( pgcontext *pgc )
{
    int totalCap = BUF_LEN + BUF_HEADLEN;
    int bufCap = BUF_LEN;
    (*pgc)->rtinfo.firstStartUp = 1;
    (*pgc)->rtinfo.Txbuf = (ppacket)malloc( sizeof(packet) );
    while( (*pgc)->rtinfo.Txbuf == NULL )
    {
        (*pgc)->rtinfo.Txbuf = (ppacket)malloc( sizeof(packet));
        sleep(1);
    }
    (*pgc)->rtinfo.Txbuf->allbuf = (uint8 *)malloc( totalCap );
    while( (*pgc)->rtinfo.Txbuf->allbuf==NULL )
    {
        (*pgc)->rtinfo.Txbuf->allbuf = (uint8 *)malloc( totalCap );
        sleep(1);
    }
    memset( (*pgc)->rtinfo.Txbuf->allbuf,0,totalCap );
    (*pgc)->rtinfo.Txbuf->totalcap = totalCap;
    (*pgc)->rtinfo.Txbuf->bufcap = bufCap;
    resetPacket( (*pgc)->rtinfo.Txbuf );

    (*pgc)->rtinfo.Rxbuf = (ppacket)malloc( sizeof(packet) );
    (*pgc)->rtinfo.Rxbuf->allbuf = (uint8 *)malloc( totalCap );
    while( (*pgc)->rtinfo.Rxbuf->allbuf==NULL )
    {
        (*pgc)->rtinfo.Rxbuf->allbuf = (uint8 *)malloc( totalCap );
        sleep(1);
    }
    memset( (*pgc)->rtinfo.Rxbuf->allbuf,0,totalCap );
    (*pgc)->rtinfo.Rxbuf->totalcap = totalCap;
    (*pgc)->rtinfo.Rxbuf->bufcap = bufCap;
    resetPacket( (*pgc)->rtinfo.Rxbuf );

    /* get config data form flash */
    GAgent_DevGetConfigData( &(*pgc)->gc );
    (*pgc)->rtinfo.waninfo.CloudStatus=CLOUD_INIT;

    /* get mac address */
    GAgent_DevGetMacAddress((*pgc)->minfo.szmac);
    memcpy( (*pgc)->minfo.ap_name,AP_NAME,strlen(AP_NAME));
    memcpy( (*pgc)->minfo.ap_name+strlen(AP_NAME),(*pgc)->minfo.szmac+8,4);

    (*pgc)->minfo.ap_name[strlen(AP_NAME)+4]= '\0';

    if((*pgc)->gc.magicNumber != GAGENT_MAGIC_NUM)
    {
        memset(&((*pgc)->gc), 0, sizeof(GAGENT_CONFIG_S));
        (*pgc)->gc.magicNumber = GAGENT_MAGIC_NUM;
    }
    else
    {
        if( strlen( (*pgc)->gc.DID )!=(DID_LEN-2) )
            memset( ((*pgc)->gc.DID ),0,DID_LEN );

        if( strlen( (*pgc)->gc.old_did )!=(DID_LEN-2))
            memset( ((*pgc)->gc.old_did ),0,DID_LEN );
        
        if( strlen( ((*pgc)->gc.wifipasscode) ) != PASSCODE_LEN )
        {    
            memset( ((*pgc)->gc.wifipasscode ),0,PASSCODE_MAXLEN + 1);
            make_rand( (*pgc)->gc.wifipasscode );
        }
        if( strlen( ((*pgc)->gc.old_wifipasscode) )!=PASSCODE_LEN || strlen( ((*pgc)->gc.old_did) )!= (DID_LEN-2) )
        {    
            memset( ((*pgc)->gc.old_wifipasscode ),0,PASSCODE_LEN );
            memset( ((*pgc)->gc.old_did ),0,DID_LEN );
        }
        
        if( strlen( ((*pgc)->gc.old_productkey) )!=(PK_LEN) )
            memset( (*pgc)->gc.old_productkey,0,PK_LEN + 1 );

        if( strlen( (*pgc)->gc.m2m_ip)>IP_LEN_MAX || strlen( (*pgc)->gc.m2m_ip)<IP_LEN_MIN )
            memset( (*pgc)->gc.m2m_ip,0,IP_LEN_MAX + 1 );
        
        if( strlen( (*pgc)->gc.GServer_ip)>IP_LEN_MAX || strlen( (*pgc)->gc.GServer_ip)<IP_LEN_MIN )
            memset( (*pgc)->gc.GServer_ip,0,IP_LEN_MAX + 1 );
        
        if( strlen( (*pgc)->gc.cloud3info.cloud3Name )>CLOUD3NAME )
            memset( (*pgc)->gc.cloud3info.cloud3Name,0,CLOUD3NAME );    
    }
    

    (*pgc)->rtinfo.waninfo.ReConnectMqttTime = GAGENT_MQTT_TIMEOUT;
	(*pgc)->rtinfo.waninfo.ReConnectHttpTime = GAGENT_HTTP_TIMEOUT;
    (*pgc)->rtinfo.waninfo.send2HttpLastTime = GAgent_GetDevTime_S();
    (*pgc)->rtinfo.waninfo.firstConnectHttpTime = GAgent_GetDevTime_S();
    (*pgc)->rtinfo.waninfo.httpCloudPingTime = 0;
    GAgent_DevSaveConfigData( &((*pgc)->gc) );
}

void GAgent_dumpInfo( pgcontext pgc )
{
    GAgent_Printf(GAGENT_DEBUG,"Product Soft Version: %s. Hard Version: %s", WIFI_SOFTVAR,WIFI_HARDVER);
    GAgent_Printf(GAGENT_DEBUG,"GAgent Compiled Time: %s, %s.\r\n",__DATE__, __TIME__);
    GAgent_Printf(GAGENT_DEBUG,"GAgent mac :%s",pgc->minfo.szmac );
    GAgent_Printf(GAGENT_DEBUG,"GAgent passcode :%s len=%d",pgc->gc.wifipasscode,strlen( pgc->gc.wifipasscode ) );
    GAgent_Printf(GAGENT_DEBUG,"GAgent did :%s len:%d",pgc->gc.DID,strlen(pgc->gc.DID) );
    GAgent_Printf(GAGENT_DEBUG,"GAgent old did :%s len:%d",pgc->gc.old_did,strlen(pgc->gc.old_did) );
    GAgent_Printf(GAGENT_DEBUG,"GAgent old pk :%s len:%d",pgc->gc.old_productkey,strlen(pgc->gc.old_productkey) );
    GAgent_Printf(GAGENT_DEBUG,"GAgent AP name:%s",pgc->minfo.ap_name );
    GAgent_Printf(GAGENT_DEBUG,"GAgent 3rd cloud :%s",pgc->gc.cloud3info.cloud3Name );
    GAgent_Printf(GAGENT_DEBUG,"GAgent M2M IP :%s",pgc->gc.m2m_ip );
    GAgent_Printf(GAGENT_DEBUG,"GAgent GService IP :%s",pgc->gc.GServer_ip );
    return ;
}
/****************************** running status ******************************/
/*
    flag=1 set GAgentStatus 
    flag=0 reset GAgentStatus
*/
void GAgent_SetWiFiStatus( pgcontext pgc,uint16 GAgentStatus,int8 flag )
{
    if(flag==1)
    {
        pgc->rtinfo.GAgentStatus |= GAgentStatus;
    }
    else
    {
        pgc->rtinfo.GAgentStatus &=~ GAgentStatus;
    }
    return ;
}

void GAgent_SetCloudConfigStatus( pgcontext pgc,int16 cloudstauts )
{
    pgc->rtinfo.waninfo.CloudStatus = cloudstauts;
    /*g_globalvar.waninfo.CloudStatus = cloudstauts;*/
    return ;
}

void GAgent_SetCloudServerStatus( pgcontext pgc,int16 serverstatus )
{
    pgc->rtinfo.waninfo.mqttstatus = serverstatus;
    /*g_globalvar.waninfo.mqttstatus = serverstatus;*/
    return ;
}
void  GAgent_AddSelectFD( pgcontext pgc )
{
    int32 i=0;
    FD_ZERO( &(pgc->rtinfo.readfd) );

    if( pgc->rtinfo.waninfo.http_socketid>0 )
        FD_SET( pgc->rtinfo.waninfo.http_socketid,&(pgc->rtinfo.readfd) );

    if( pgc->rtinfo.waninfo.m2m_socketid>0 )
        FD_SET( pgc->rtinfo.waninfo.m2m_socketid,&(pgc->rtinfo.readfd) );

    if( pgc->rtinfo.local.uart_fd>0 )
        FD_SET( pgc->rtinfo.local.uart_fd,&(pgc->rtinfo.readfd));

    if( pgc->ls.tcpServerFd > 0 )
        FD_SET( pgc->ls.tcpServerFd, &(pgc->rtinfo.readfd) );

    if( pgc->ls.udpServerFd > 0 )
        FD_SET( pgc->ls.udpServerFd, &(pgc->rtinfo.readfd) );

    if( pgc->ls.tcpWebConfigFd > 0 )
        FD_SET( pgc->ls.tcpWebConfigFd, &(pgc->rtinfo.readfd) );
    
    for(i = 0; i < LAN_TCPCLIENT_MAX; i++)
    {
        if( pgc->ls.tcpClient[i].fd > 0 )
        {
            FD_SET( pgc->ls.tcpClient[i].fd, &(pgc->rtinfo.readfd) );
        }
    }
}

int32 GAgent_MaxFd( pgcontext pgc  ) 
{
    int i;
    int32 maxfd =0;

    if( maxfd<=pgc->rtinfo.waninfo.http_socketid )
        maxfd = pgc->rtinfo.waninfo.http_socketid;

    if( maxfd<pgc->rtinfo.waninfo.m2m_socketid )
        maxfd = pgc->rtinfo.waninfo.m2m_socketid;

    if( maxfd<=pgc->rtinfo.local.uart_fd )
        maxfd = pgc->rtinfo.local.uart_fd;

    if( maxfd<pgc->ls.tcpServerFd )
        maxfd = pgc->ls.tcpServerFd;

    if( maxfd<pgc->ls.udpServerFd )
        maxfd = pgc->ls.udpServerFd;  

    if( maxfd<pgc->ls.tcpWebConfigFd )
        maxfd = pgc->ls.tcpWebConfigFd;  

    for(i = 0; i < LAN_TCPCLIENT_MAX; i++)
    {
        if(pgc->ls.tcpClient[i].fd >0 && maxfd< pgc->ls.tcpClient[i].fd )
        {
            maxfd = pgc->ls.tcpClient[i].fd;
        }
    }

    return maxfd;
}


/****************************************************************
*       functionName    :   GAgent_SetGServerIP
*       description     :   set the  Gserver ip into configdata
*       Input           :   gserver ip string like "192.168.1.1"
*       return          :   =0 set Gserver ip ok
*                       :   other fail 
*       add by Alex.lin     --2015-03-02
****************************************************************/
int8 GAgent_SetGServerIP( pgcontext pgc,int8 *szIP )
{
    /*strcpy( g_stGAgentConfigData.GServer_ip,szIP );*/
    strcpy( pgc->gc.GServer_ip,szIP );
    GAgent_DevSaveConfigData( &(pgc->gc) );
    return 0;
}
/****************************************************************
*       functionName    :   GAgent_SetGServerSocket
*       description     :   set the  Gserver socket val
*       pgc             :   global struct.
*       socketid        :   the socketid will set into GServer socket
*       return          :   =0 set ok
*                       :   other fail 
*       add by Alex.lin     --2015-03-02
****************************************************************/
int8 GAgent_SetGServerSocket( pgcontext pgc,int32 socketid )
{
    pgc->rtinfo.waninfo.http_socketid = socketid;
    /*g_globalvar.waninfo.http_socketid = socketid;*/
    return 0;
}
/****************************** config status ******************************/
uint8 GAgent_SetDeviceID( pgcontext pgc,int8 *p_szDeviceID )
{
    //int8 len=0;
    //len = strlen( p_szDeviceID );
    if( p_szDeviceID != NULL )
    {
        strcpy( pgc->gc.DID,p_szDeviceID );
    }
    else
    {
        memset( pgc->gc.DID,0,DID_LEN );
    }
    GAgent_DevSaveConfigData( &(pgc->gc) );
    return 0;
}
/****************************************************************
*       FunctionName      :     GAgent_SetOldDeviceID
*       Description       :     reset the old did and passcode
*       flag              :     0 reset to NULL
*                               1 set the new did and passcode 
*                                 to old info.
*      Add by Alex.lin      --2015-03-02
****************************************************************/
int8 GAgent_SetOldDeviceID( pgcontext pgc,int8* p_szDeviceID,int8* p_szPassCode,int8 flag )
{
    /*
    memset( g_stGAgentConfigData.old_did,0,24 );
    memset( g_stGAgentConfigData.old_wifipasscode,0,16 );
    */
    memset( pgc->gc.old_did,0,DID_LEN );
    memset( pgc->gc.old_wifipasscode,0,PASSCODE_LEN );
    if( 1 == flag )
    {
        strcpy( pgc->gc.old_did,p_szDeviceID );
        strcpy( pgc->gc.old_wifipasscode,p_szPassCode );
    }
    GAgent_DevSaveConfigData( &(pgc->gc) );
    return 0;
}

/*
    return 0 : don't need to disable did.
           1 : need to disable did
*/           
int8 GAgent_IsNeedDisableDID( pgcontext pgc )
{
    uint32 didLen=0,passcodeLen=0;
    didLen = strlen( pgc->gc.old_did );
    passcodeLen = strlen( pgc->gc.old_wifipasscode );
    if( (0==didLen)|| ( 22<didLen) || (passcodeLen==0) || (passcodeLen>16) ) /* invalid did length or passcode length */
    {
        memset( pgc->gc.old_did,0,DID_LEN );
        memset( pgc->gc.old_wifipasscode,0,PASSCODE_LEN );
        GAgent_DevSaveConfigData( &(pgc->gc) );
        return 0;
    }
    return 1;
}
void GAgent_logevelSet( uint16 level )
{
    pgContextData->rtinfo.loglevel = level;
}
int8 GAgent_loglevelenable( uint16 level )
{
    if( level > pgContextData->rtinfo.loglevel )
        return 1;
    else 
        return 0;
}
/****************************************************************
*       FunctionName      :     GAgent_RefreshIPTick
*       Description       :     update ip tick,if gethostbyname 
                                ok will set time to one hour 
*      Add by Alex.lin      --2015-04-23
****************************************************************/
void GAgent_RefreshIPTick( pgcontext pgc,uint32 dTime_s )
{
    uint32 cTime=0,dTime=0;
    int8 tmpip[32] = {0},failed=0,ret=0,flag=0;

    if( (pgc->rtinfo.GAgentStatus)&WIFI_MODE_TEST == WIFI_MODE_TEST )
    {
        return ;
    }
    pgc->rtinfo.waninfo.RefreshIPLastTime+=dTime_s;
    if( (pgc->rtinfo.waninfo.RefreshIPLastTime) >= (pgc->rtinfo.waninfo.RefreshIPTime) )
    {
        GAgent_Printf( GAGENT_DEBUG,"GAgentStatus:%04x",(pgc->rtinfo.GAgentStatus) );
        GAgent_Printf( GAGENT_DEBUG,"RefreshIPTime=%d ms,lsst:%d,ctimd %d",(pgc->rtinfo.waninfo.RefreshIPTime),(pgc->rtinfo.waninfo.RefreshIPLastTime) ,cTime);
        GAgent_Printf( GAGENT_DEBUG,"RefreshIPTime=%d s",(pgc->rtinfo.waninfo.RefreshIPTime) );
        pgc->rtinfo.waninfo.RefreshIPLastTime = 0;

        if( ((pgc->rtinfo.GAgentStatus)&WIFI_STATION_CONNECTED) != WIFI_STATION_CONNECTED )
        {
            GAgent_Printf( GAGENT_DEBUG,"line %d",__LINE__ );
            pgc->rtinfo.waninfo.RefreshIPTime =  1;

       
        }
        ret = GAgent_GetHostByName(HTTP_SERVER, tmpip);
        if( ret!=0 )
        {
            GAgent_Printf( GAGENT_ERROR,"line %d",__LINE__ );
            failed=1;
        }
        else
        {
            GAgent_Printf( GAGENT_DEBUG," %s : %s",HTTP_SERVER,tmpip);
            if(strcmp( pgc->gc.GServer_ip, tmpip) != 0)
            {
                GAgent_Printf( GAGENT_DEBUG,"Save GService ip into flash!");
                strcpy(pgc->gc.GServer_ip, tmpip );
                GAgent_DevSaveConfigData( &(pgc->gc) );
            }
        }
        
        if(strlen(pgc->minfo.m2m_SERVER) == 0)
        {
           GAgent_Printf(GAGENT_DEBUG," m2m server is NULL!");
            //not yet m2m host
            failed = 1;
        }
        else
        {
            ret = GAgent_GetHostByName( pgc->minfo.m2m_SERVER,tmpip );
            if( ret!=0)
            {
                GAgent_Printf( GAGENT_DEBUG,"line %d",__LINE__ );
                failed = 1;
            }
            else
            {
                GAgent_Printf( GAGENT_DEBUG,"got %s : %s",pgc->minfo.m2m_SERVER,tmpip);
                if( 0!=strcmp( pgc->gc.m2m_ip,tmpip) )
                {
                    strcpy( pgc->gc.m2m_ip,tmpip );
                    GAgent_DevSaveConfigData( &(pgc->gc) );
                    GAgent_DevGetConfigData( &(pgc->gc) );
                }
            }
            
        }
        if( 1==failed )
        {
            if( ((pgc->rtinfo.GAgentStatus)&WIFI_MODE_TEST) == WIFI_MODE_TEST )
            {
                pgc->rtinfo.waninfo.RefreshIPTime =  1*10;
            }
            else
            {
                pgc->rtinfo.waninfo.RefreshIPTime = 1;
            }
        }
        else
        {
            pgc->rtinfo.waninfo.RefreshIPTime = ONE_HOUR;
        }
    }
}

/******************************************************
 *      FUNCTION        :   update info
 *      new_pk          :   new productkey
 *   Add by Alex lin  --2014-12-19
 *
 ********************************************************/
void GAgent_UpdateInfo( pgcontext pgc,int8 *new_pk )
{
    GAgent_Printf(GAGENT_DEBUG,"a new productkey is :%s.",new_pk);
    /*the necessary information to disable devices*/
    memset( pgc->gc.old_did,0,DID_LEN);
    memset( pgc->gc.old_wifipasscode,0,PASSCODE_MAXLEN + 1);
    /*存到old字段用于注销设备*/
    memcpy( pgc->gc.old_did,pgc->gc.DID,DID_LEN);
    memcpy( pgc->gc.old_wifipasscode,pgc->gc.wifipasscode,PASSCODE_MAXLEN + 1);
    
    memset( pgc->gc.old_productkey,0,PK_LEN + 1);
    memcpy( pgc->gc.old_productkey,new_pk,PK_LEN + 1);
    pgc->gc.old_productkey[PK_LEN] = '\0';
    
    /*neet to reset info */
    memset( pgc->gc.FirmwareVer,0,FIRMWARE_LEN_MAX + 1);
    memset( pgc->gc.FirmwareVerLen,0,2);
    memset( &(pgc->gc.cloud3info),0,sizeof(pgc->gc.cloud3info));
    memset( pgc->gc.DID,0,DID_LEN );
    
   /*生成新的wifipasscode*/
    make_rand(pgc->gc.wifipasscode);

    GAgent_DevSaveConfigData( &(pgc->gc) );
}
/******************************************************
 *      FUNCTION        :   uGAgent_Config
 *      new_pk          :   new productkey
 *   Add by Alex lin  --2014-12-19
 *
 ********************************************************/
void GAgent_Config( uint8 typed,pgcontext pgc )
{
    switch( typed )
    {
        //AP MODE
        case 1:
            GAgent_DRV_WiFi_SoftAPModeStart( AP_NAME, AP_PASSWORD, WIFI_MODE_AP );
        break;
        
        //Airlink
        case 2:
        {
            int8 timeout ;
            timeout = 10;
            GAgent_OpenAirlink( timeout/* timeout */ );
            while( timeout )
            {
                timeout--;
                sleep(1);
                if( pgc->gc.flag & XPG_CFG_FLAG_CONFIG ==XPG_CFG_FLAG_CONFIG )
                {    
                    GAgent_DRVSetWiFiStartMode( pgc,WIFI_MODE_STATION );
                    GAgent_DevSaveConfigData( &(pgc->gc) );  
                    break;
                }
            }      
        break;
        }
        default :
        break;
    }
}
uint8 GAgent_EnterTest( pgcontext pgc )
{
    pgc->rtinfo.scanWifiFlag = 0;
    memset( pgc->gc.GServer_ip,0,IP_LEN_MAX+1);
    memset( pgc->gc.m2m_ip,0,IP_LEN_MAX+1);

    GAgent_DevSaveConfigData( &(pgc->gc) );
    GAgent_SetWiFiStatus( pgc,WIFI_MODE_TEST,1 );
    GAgent_DRVWiFiStartScan();
    return 0;
}
uint8 GAgent_ExitTest( pgcontext pgc )
{
    int16 ret=0;
    pgc->rtinfo.scanWifiFlag = 0;
    ret = GAgent_DRVWiFi_StationDisconnect();
    GAgent_SetWiFiStatus( pgc,WIFI_MODE_TEST,0 );
    GAgent_DRVWiFiStopScan( );
    return 0;
}
/****************************************************************
*       FunctionName      :     GAgent_BaseTick
*       Description       :     the function will return 1 second at least
*       return            :     return 1 second at least. 
*      Add by Alex.lin      --2015-04-23
****************************************************************/
uint32 GAgent_BaseTick()
{
    static uint32 preTime = 0;
    uint32 cTime=0,dTime=0;

    if(0 == preTime)
        preTime = GAgent_GetDevTime_S();//run in first time;

    cTime = GAgent_GetDevTime_S();

    if(cTime >= preTime)
    {
        dTime = cTime - preTime;
    }
    else
    {
        //memory overflow
        dTime = cTime + ( (~preTime)+1 );
    }
    preTime = cTime;
    
    if(dTime < 1)
    {
        return 0;
    }
    return dTime;
}

/****************************************************************
*       FunctionName      :     GAgent_Tick
*       Description       :     GAgent runing Tick.
*       return            :     return NULL. 
*      Add by Alex.lin      --2015-04-23
****************************************************************/
void GAgent_Tick( pgcontext pgc )
{
    uint32 dTime=0;

    dTime = GAgent_BaseTick();
    if( dTime<1 )
        return ;

    GAgent_DevTick();
    GAgent_CloudTick( pgc,dTime );
    GAgent_LocalTick( pgc,dTime );
    GAgent_LanTick( pgc,dTime );
    GAgent_WiFiEventTick( pgc,dTime );
    GAgent_RefreshIPTick( pgc,dTime );

}
