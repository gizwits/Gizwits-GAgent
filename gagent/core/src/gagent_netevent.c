#include "gagent.h"
#include "cloud.h"
#include "platform.h"

void GAgent_WiFiInit( pgcontext pgc )
{
    uint16 tempWiFiStatus=0;
    tempWiFiStatus = pgc->rtinfo.GAgentStatus;
   if( ((pgc)->gc.flag & XPG_CFG_FLAG_CONFIG_AP) == XPG_CFG_FLAG_CONFIG_AP )
    {
        GAgent_Printf( GAGENT_DEBUG," GAgent XPG_CFG_FLAG_CONFIG_AP." );
        GAgent_DevCheckWifiStatus( WIFI_MODE_ONBOARDING,1 );
    }
    if( GAgent_DRVBootConfigWiFiMode() != GAgent_DRVGetWiFiMode(pgc) )
    {
        //前配置与GAgent启动模式不一样，
        //以GAgent启动模式为准.
        if( ((pgc->gc.flag)&XPG_CFG_FLAG_CONNECTED) == XPG_CFG_FLAG_CONNECTED )
        {
            GAgent_Printf( GAGENT_INFO,"In Station mode");
            GAgent_Printf( GAGENT_INFO,"SSID:%s,KEY:%s",pgc->gc.wifi_ssid,pgc->gc.wifi_key );
            tempWiFiStatus |=WIFI_MODE_STATION;
            tempWiFiStatus |= GAgent_DRVWiFi_StationCustomModeStart( pgc->gc.wifi_ssid,pgc->gc.wifi_key, tempWiFiStatus );
        }
        else
        {
            GAgent_Printf( GAGENT_CRITICAL,"In AP mode");
            tempWiFiStatus |=WIFI_MODE_AP;
            tempWiFiStatus |= GAgent_DRV_WiFi_SoftAPModeStart( pgc->minfo.ap_name,AP_PASSWORD,tempWiFiStatus );
        }
    }
}
/****************************************************************
Function    :   GAgent_DevCheckWifiStatus
Description :   check the wifi status and will set the wifi status
                and return it.
wifistatus  :   =0XFFFF get the wifistatus.
                !=0XFFFF & flag=1 set the wifistatus to the wifistatus.
                !=0xFFFF & flag=0 reset the wifistatus.
eg.
set the "WIFI_STATION_CONNECTED" into wifistatus,like:
    GAgent_DevCheckWifiStatus( WIFI_STATION_CONNECTED,1  );
reset the "WIFI_STATION_CONNECTED" from wifistatus,like:
    GAgent_DevCheckWifiStatus( WIFI_STATION_CONNECTED,0  );
return      :   the new wifi status.
Add by Alex.lin     --2015-04-17.
****************************************************************/
uint16 GAgent_DevCheckWifiStatus( uint16 wifistatus,int8 flag  )
{
    static uint16 halWiFiStatus=0;

    if( 0xFFFF==wifistatus )
    {
        GAgent_Printf( GAGENT_DEBUG," GAgent Get Hal wifiStatus :%04X ",halWiFiStatus );
       return halWiFiStatus;
    }
    else
    {
        if( 1==flag )
        {
            //对应位置1
            halWiFiStatus |=wifistatus;
            GAgent_Printf( GAGENT_DEBUG,"GAgent Hal Set wifiStatus%04X",wifistatus);
        }
        else
        {   //对应位清零
            uint16 tempstatus=0;
            tempstatus = ( 0xFFFF - wifistatus );
            halWiFiStatus &=tempstatus;
            GAgent_Printf( GAGENT_DEBUG,"GAgent Hal ReSet wifiStatus%04X",wifistatus);
        }

    }
    GAgent_Printf( GAGENT_DEBUG," GAgent Hal wifiStatus :%04X ",halWiFiStatus );
    return halWiFiStatus;
}
void GAgentSetLedStatus( uint16 gagentWiFiStatus )
{
    static uint8  Router_Connect_flag = 0;
    static uint32 Router_Connect_count = 0;
    static uint8  Airlink_flag = 0;
    static uint32 Airlink_count = 0;
    static uint8  Cloud_Connect_flag = 0;
    static uint32 Cloud_Connect_count = 0;

    switch( gagentWiFiStatus&0x35 )
    {
        case WIFI_MODE_AP: 
            GAgent_DevLED_Red( 1 );
            GAgent_DevLED_Green( 0 ); 
            break;
        case WIFI_MODE_ONBOARDING:
            if( (gagentWiFiStatus&WIFI_MODE_AP) == 0 )
            {
                Airlink_count++;
                if( Airlink_count%4 == 0)
                {
                    Airlink_flag = !Airlink_flag;
                    Airlink_count = 0;
                }
                GAgent_DevLED_Red( 0 );
                GAgent_DevLED_Green( Airlink_flag ); 
            }
            else
            {
                GAgent_DevLED_Red( 1 );
                GAgent_DevLED_Green( 0 ); 
            }
            break;
        case WIFI_STATION_CONNECTED:          
            GAgent_DevLED_Red( 1 );
            GAgent_DevLED_Green( 1 );      
            break;
        case ( WIFI_CLOUD_CONNECTED | WIFI_STATION_CONNECTED ):
            if( Cloud_Connect_count < ONE_MINUTE*10 )
            {
                Cloud_Connect_count++;
                if( Cloud_Connect_count%4 == 0)
                {
                    Cloud_Connect_flag = !Cloud_Connect_flag;
                }
                GAgent_DevLED_Red( Cloud_Connect_flag );
                GAgent_DevLED_Green( !Cloud_Connect_flag );
            }
            else
            {
                GAgent_DevLED_Red( 0 );
                GAgent_DevLED_Green( 0 );  
            }
            break;
        default: //don't connect router           
            Router_Connect_count++;            
            if( Router_Connect_count%4 == 0)
            {
                Router_Connect_flag = !Router_Connect_flag;
                Router_Connect_count = 0;
            }
            GAgent_DevLED_Red( 1 );
            GAgent_DevLED_Green( Router_Connect_flag );
            break;
    }
   
}

/****************************************************************
FunctionName    :   GAgentFindTestApHost.
Description     :   find the test ap host,like:GIZWITS_TEST_*
NetHostList_str :   GAgent wifi scan result .
return          :   1-GIZWITS_TEST_1
                    2-GIZWITS_TEST_2
                    fail :0.
Add by Alex.lin         --2015-05-06
****************************************************************/
uint8 GAgentFindTestApHost( NetHostList_str *pAplist )
{
    int16 i=0;
    int8 apNum=0,ret=0;

    for( i=0;i<pAplist->ApNum;i++ )
    {
        if( 0==memcmp(pAplist->ApList[i].ssid,(int8 *)GAGENT_TEST_AP1,strlen(GAGENT_TEST_AP1)) )
        {
            apNum=1;
        }
        if( 0==memcmp(pAplist->ApList[i].ssid,(int8 *)GAGENT_TEST_AP2,strlen(GAGENT_TEST_AP2)) )
        {
            /* 两个热点都能找到 */
            if( 1==apNum)
              apNum=3;
            else
              apNum=2;
        }
    }
    switch( apNum )
    {
        /* only the GIZWITS_TEST_1 */
        case 1:
            ret=1;
        break;
        /* only the GIZWITS_TEST_2 */
        case 2:
            ret=2;
        break;
        /* both of the test ap */
        case 3:
           srand(GAgent_GetDevTime_MS());
           ret = rand()%100;
           ret = (ret%2)+1; 
        break;
        default:
        ret =0;
        break;
    }

    if(NULL !=  pAplist->ApList)
            free( pAplist->ApList); 
            
    return ret;
}
void  GAgent_WiFiEventTick( pgcontext pgc,uint32 dTime_s )
{
    uint16 newStatus=0;
    uint16 gagentWiFiStatus=0;
    static uint32 gagentOnboardingTime=0;

    gagentWiFiStatus = ( (pgc->rtinfo.GAgentStatus)&(LOCAL_GAGENTSTATUS_MASK) ) ;
    newStatus = GAgent_DevCheckWifiStatus( 0xffff,1 );
    GAgent_Printf( GAGENT_INFO,"wifiStatus : %04x new:%04x", gagentWiFiStatus,newStatus );
    if( (gagentWiFiStatus&WIFI_MODE_AP) != ( newStatus&WIFI_MODE_AP) )
    {
        if( newStatus&WIFI_MODE_AP )
        {
            //WIFI_MODE_AP UP
            GAgent_SetWiFiStatus( pgc,WIFI_MODE_AP,1 );
        }
        else
        {
            //WIFI_MODE_AP DOWN
            GAgent_SetWiFiStatus( pgc,WIFI_MODE_AP,0 );
        }
        pgc->rtinfo.waninfo.wanclient_num=0;
        pgc->ls.tcpClientNums=0;
        if( pgc->rtinfo.waninfo.CloudStatus == CLOUD_CONFIG_OK )
        {
            GAgent_SetCloudServerStatus( pgc,MQTT_STATUS_START );
        }
        else
        {
            GAgent_SetCloudConfigStatus( pgc,CLOUD_INIT );
        }
        GAgent_DevCheckWifiStatus( WIFI_CLOUD_CONNECTED,0 );
        //GAgent_SetWiFiStatus( pgc,WIFI_CLOUD_CONNECTED,0 );
    }
    if( (gagentWiFiStatus&WIFI_MODE_STATION) != ( newStatus&WIFI_MODE_STATION) )
    {
        if( newStatus&WIFI_MODE_STATION )
        {
            //WIFI_MODE_STATION UP
            GAgent_Printf( GAGENT_INFO,"WIFI_MODE_STATION UP." );
            GAgent_SetWiFiStatus( pgc,WIFI_MODE_STATION,1 );
        }
        else
        {
            //WIFI_MODE_STATION DOWN
            GAgent_Printf( GAGENT_INFO,"WIFI_MODE_STATION Down." );
            GAgent_SetWiFiStatus( pgc,WIFI_MODE_STATION,0 );
            GAgent_DevCheckWifiStatus( WIFI_CLOUD_CONNECTED,0 );
            if( pgc->rtinfo.waninfo.CloudStatus == CLOUD_CONFIG_OK )
            {
                GAgent_SetCloudServerStatus( pgc, MQTT_STATUS_START );
            }
            else
            {
                GAgent_SetCloudConfigStatus( pgc,CLOUD_INIT );
            }
        }
        pgc->rtinfo.waninfo.wanclient_num=0;
        pgc->ls.tcpClientNums=0;
    }
    if( (gagentWiFiStatus&WIFI_MODE_ONBOARDING) != ( newStatus&WIFI_MODE_ONBOARDING) )
    {
        if( newStatus&WIFI_MODE_ONBOARDING )
        {
            uint16 tempWiFiStatus=0;
            //WIFI_MODE_ONBOARDING UP
            pgc->gc.flag |= XPG_CFG_FLAG_CONFIG_AP;
            GAgent_DevSaveConfigData( &(pgc->gc) );
            GAgent_Printf( GAGENT_INFO,"WIFI_MODE_ONBOARDING UP." );
            
            if( (newStatus&WIFI_STATION_CONNECTED) == WIFI_STATION_CONNECTED )
            {
                tempWiFiStatus = GAgent_DRVWiFi_StationDisconnect();
            }
            if( !(newStatus&WIFI_MODE_AP) )
            {
                tempWiFiStatus |= GAgent_DRV_WiFi_SoftAPModeStart( pgc->minfo.ap_name,AP_PASSWORD,tempWiFiStatus );
            }
            GAgent_SetWiFiStatus( pgc,WIFI_MODE_ONBOARDING,1 );
            gagentOnboardingTime = GAGENT_ONBOARDING_TIME;
        }
        else
        {
            //WIFI_MODE_ONBOARDING DOWN
            pgc->gc.flag &=~ XPG_CFG_FLAG_CONFIG_AP;
            GAgent_DevSaveConfigData( &(pgc->gc) );
            GAgent_Printf( GAGENT_INFO,"WIFI_MODE_ONBOARDING DOWN." );
            if( gagentOnboardingTime>0 )
            { /* 在规定时间内接收到配置包 */
              GAgent_Printf( GAGENT_INFO,"Receive OnBoarding data.");
              pgc->ls.onboardingBroadCastTime = SEND_UDP_DATA_TIMES;
              pgc->gc.flag |=XPG_CFG_FLAG_CONFIG;
              GAgent_DevSaveConfigData( &(pgc->gc) );
            }
            GAgent_SetWiFiStatus( pgc,WIFI_MODE_ONBOARDING,0 );
            gagentOnboardingTime = 0;
            GAgent_WiFiInit( pgc );
        }
        if( pgc->rtinfo.waninfo.CloudStatus == CLOUD_CONFIG_OK )
        {
            GAgent_SetCloudServerStatus( pgc, MQTT_STATUS_START );
        }
        else
        {
            GAgent_SetCloudConfigStatus( pgc,CLOUD_INIT );
        }
        pgc->rtinfo.waninfo.wanclient_num=0;
        pgc->ls.tcpClientNums=0;
        //GAgent_SetWiFiStatus( pgc,WIFI_CLOUD_CONNECTED,0 );
        GAgent_DevCheckWifiStatus( WIFI_CLOUD_CONNECTED,0 );
    }
    if( (gagentWiFiStatus&WIFI_STATION_CONNECTED) != ( newStatus & WIFI_STATION_CONNECTED) )
    {
        if( newStatus&WIFI_STATION_CONNECTED )
        {
            GAgent_Printf( GAGENT_INFO," WIFI_STATION_CONNECTED UP" );
            /* 重置重新连接云端时间 */
            pgc->rtinfo.waninfo.ReConnectMqttTime = GAGENT_MQTT_TIMEOUT;
            pgc->rtinfo.waninfo.ReConnectHttpTime = GAGENT_HTTP_TIMEOUT;
            if( !(newStatus&WIFI_MODE_ONBOARDING) )
            {
                if( (newStatus&WIFI_MODE_AP)==WIFI_MODE_AP )
                {
                    GAgent_DRVWiFi_APModeStop( pgc );
                }
            }
            //WIFI_STATION_CONNECTED UP
            GAgent_DRVWiFiPowerScan( pgc );
            GAgent_SetWiFiStatus( pgc,WIFI_STATION_CONNECTED,1 );
        }
        else
        {
            //WIFI_STATION_CONNECTED DOWN
            GAgent_Printf( GAGENT_INFO," WIFI_STATION_CONNECTED Down" );
            GAgent_SetWiFiStatus( pgc,WIFI_STATION_CONNECTED,0 );
            GAgent_SetWiFiStatus( pgc,WIFI_CLIENT_ON,0 );
            GAgent_SetCloudServerStatus( pgc,MQTT_STATUS_START );

            GAgent_DevCheckWifiStatus( WIFI_CLOUD_CONNECTED,0 );
        }
        pgc->rtinfo.waninfo.wanclient_num=0;
        pgc->ls.tcpClientNums=0;
    }
    if( (gagentWiFiStatus&WIFI_CLOUD_CONNECTED) != ( newStatus & WIFI_CLOUD_CONNECTED) )
    {
        if( newStatus&WIFI_CLOUD_CONNECTED )
        {
            //WIFI_CLOUD_CONNECTED UP
            GAgent_Printf( GAGENT_INFO," WIFI_CLOUD_CONNECTED Up." );
            GAgent_SetWiFiStatus( pgc,WIFI_CLOUD_CONNECTED,1 );
        }
        else
        {
            //WIFI_CLOUD_CONNECTED DOWN
            GAgent_Printf( GAGENT_INFO," WIFI_CLOUD_CONNECTED Down." );
            pgc->rtinfo.waninfo.wanclient_num=0;
            GAgent_SetCloudServerStatus( pgc,MQTT_STATUS_START );
            GAgent_SetWiFiStatus( pgc,WIFI_CLOUD_CONNECTED,0 );
            
        }
    }
    if( gagentWiFiStatus&WIFI_MODE_TEST )//test mode
    {
        static int8 cnt=0;
        int8 ret =0;
        NetHostList_str *aplist=NULL;
              
        if( 0 == pgc->rtinfo.scanWifiFlag )
        {
        
            pgc->rtinfo.testLastTimeStamp+=dTime_s;

            if( cnt>=18 )
            {
                cnt=0;
                GAgent_SetWiFiStatus( pgc,WIFI_MODE_TEST,0 );
                GAgent_DRVWiFiStopScan( );
                GAgent_Printf( GAGENT_INFO,"Exit Test Mode...");
            }

            if( pgc->rtinfo.testLastTimeStamp >= 10 )
            {
                cnt++;
                pgc->rtinfo.testLastTimeStamp = 0;
                GAgent_DRVWiFiStartScan();
                GAgent_Printf( GAGENT_INFO,"IN TEST MODE...");
            }
            aplist = GAgentDRVWiFiScanResult( aplist );
            if( NULL==aplist )
            {
                ret = 0;
            }
            else
            {
                if( aplist->ApNum <= 0 )
                {
                    ret = 0;
                }
                else
                {
                    ret = GAgentFindTestApHost( aplist );
                }
            }
        }
        if( ret>0 )
        {
             uint16 tempWiFiStatus=0;
             pgc->rtinfo.scanWifiFlag = 1;
             cnt=0;
             GAgent_DRVWiFiStopScan( );
             if( 1==ret )
             {
                tempWiFiStatus |= GAgent_DRVWiFi_StationCustomModeStart( GAGENT_TEST_AP1,GAGENT_TEST_AP_PASS,pgc->rtinfo.GAgentStatus );
             }
             else
             {
                tempWiFiStatus |= GAgent_DRVWiFi_StationCustomModeStart( GAGENT_TEST_AP2,GAGENT_TEST_AP_PASS,pgc->rtinfo.GAgentStatus );
             }
        }
    }
    pgc->rtinfo.wifiLastScanTime+=dTime_s;
    if( gagentWiFiStatus&WIFI_STATION_CONNECTED )
    {
        static int8 tempwifiRSSI=0;
        int8 wifiRSSI=0;
        uint16 wifiLevel=0;
        if( pgc->rtinfo.wifiLastScanTime >= GAGENT_STA_SCANTIME )
        {
            pgc->rtinfo.wifiLastScanTime=0;
            GAgent_DRVWiFiPowerScan( pgc );
            GAgent_Printf( GAGENT_INFO,"start to scan wifi ...");
        }
        wifiRSSI = GAgent_DRVWiFiPowerScanResult( pgc );
        GAgent_Printf( GAGENT_INFO,"wifiRSSI=%d",wifiRSSI);
        if( abs( wifiRSSI-tempwifiRSSI )>=10 )
        {
            tempwifiRSSI = wifiRSSI;
            wifiLevel = GAgent_GetStaWiFiLevel( wifiRSSI );
            gagentWiFiStatus =gagentWiFiStatus|(wifiLevel<<8);
            pgc->rtinfo.GAgentStatus = gagentWiFiStatus;
            GAgent_Printf( GAGENT_INFO,"SSID power:%d level:%d wifistatus:%04x",wifiRSSI,wifiLevel,gagentWiFiStatus );
        }
    }
    if( gagentWiFiStatus&WIFI_MODE_AP )
    {
        NetHostList_str *pAplist=NULL;
        int32 i=0;
        
        if( pgc->rtinfo.wifiLastScanTime >=GAGENT_AP_SCANTIME )
        {
            pgc->rtinfo.wifiLastScanTime=0;
           GAgent_DRVWiFiStartScan( );
        }
       
        pAplist = GAgentDRVWiFiScanResult( pAplist );
        if( NULL == pAplist )
        {
           GAgent_Printf( GAGENT_WARNING,"pAplist is NULL!");
        }
        else
        {
            if( (pgc->rtinfo.aplist.ApList)!=NULL )
            {
                GAgent_Printf( GAGENT_CRITICAL,"free xpg aplist...");
                free( (pgc->rtinfo.aplist.ApList) );
            }
            if( pAplist->ApNum>0 )
            {
                pgc->rtinfo.aplist.ApNum = pAplist->ApNum;
                (pgc->rtinfo.aplist.ApList) = (ApHostList_str *)malloc( (pAplist->ApNum)*sizeof(ApHostList_str) );
                if( (pgc->rtinfo.aplist.ApList)!=NULL )
                {
                    for( i=0;i<pAplist->ApNum;i++ )
                    {
                        strcpy( (char *)pgc->rtinfo.aplist.ApList[i].ssid, (char *)pAplist->ApList[i].ssid);
                        pgc->rtinfo.aplist.ApList[i].ApPower = pAplist->ApList[i].ApPower;
    //                    GAgent_Printf( GAGENT_CRITICAL,"AP Scan SSID = %s power = %d", 
    //                                                            pgc->rtinfo.aplist.ApList[i].ssid,
    //                                                            pgc->rtinfo.aplist.ApList[i].ApPower );
                    }
                }
            }
        }
    }
    if( (gagentWiFiStatus&WIFI_MODE_ONBOARDING)==WIFI_MODE_ONBOARDING )
    {
        uint16 TempWiFiStatus = 0;
        TempWiFiStatus = gagentWiFiStatus;

        if( gagentOnboardingTime>0 )
        {
            gagentOnboardingTime--;
        }
        else
        {
            GAgent_DevCheckWifiStatus( WIFI_MODE_ONBOARDING,0 );
            GAgent_Printf( GAGENT_DEBUG,"file:%s function:%s line:%d ",__FILE__,__FUNCTION__,__LINE__ );
        }
    }
    /*
    uint16 temp=0;
    gagentWiFiStatus = pgc->rtinfo.GAgentStatus;
    //获取状态
    GAgent_Printf( GAGENT_DEBUG,"file:%s function:%s line:%d ",__FILE__,__FUNCTION__,__LINE__ );
    temp = GAgent_DevCheckWifiStatus( 0xFFFF );
    temp = ( temp&0x13 );
    gagentWiFiStatus = ( gagentWiFiStatus&0xFFEC );
    gagentWiFiStatus = ( gagentWiFiStatus|temp );
    */
    //GAgent_Printf( GAGENT_INFO,"temp = %04X gagentWiFiStatus: %04X",temp,gagentWiFiStatus );
    GAgent_LocalSendGAgentstatus(pgc,dTime_s);
    GAgentSetLedStatus( gagentWiFiStatus );

    //newStatus = gagentWiFiStatus;
    //GAgent_Printf( GAGENT_DEBUG,"file:%s function:%s line:%d ",__FILE__,__FUNCTION__,__LINE__ );
    //newStatus = GAgent_DevCheckWifiStatus( newStatus );
    return ;
}
