#include "gagent.h"
#include "cloud.h"

void GAgent_WiFiInit( pgcontext pgc )
{
    int8 ret=0;
    //strcpy( pgc->gc.wifi_ssid,SSID );
    //strcpy( pgc->gc.wifi_key,KEY );
    ret = GAgent_DRVGetWiFiMode(pgc);
    if( ((pgc->gc.flag)&XPG_CFG_FLAG_CONNECTED) == XPG_CFG_FLAG_CONNECTED )
    {
        GAgent_Printf( GAGENT_INFO,"In Station mode");
        GAgent_Printf( GAGENT_INFO,"SSID:%s,KEY:%s",pgc->gc.wifi_ssid,pgc->gc.wifi_key );
        pgc->rtinfo.GAgentStatus |=WIFI_MODE_STATION;
        pgc->rtinfo.GAgentStatus |= GAgent_DRVWiFi_StationCustomModeStart( pgc->gc.wifi_ssid,pgc->gc.wifi_key,pgc->rtinfo.GAgentStatus );
    }
    else
    {
        GAgent_Printf( GAGENT_CRITICAL,"In AP mode");
        pgc->rtinfo.GAgentStatus |=WIFI_MODE_AP;
        pgc->rtinfo.GAgentStatus |= GAgent_DRV_WiFi_SoftAPModeStart( pgc->minfo.ap_name,AP_PASSWORD,pgc->rtinfo.GAgentStatus );
        
    }
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
    int8 i=0,apNum=0,ret=0;


    if( pAplist->ApNum <= 0 )
        return 0;
    for( i=0;i<pAplist->ApNum;i++ )
    {
        if( 0==memcmp(pAplist->ApList[i].ssid,GAGENT_TEST_AP1,strlen(GAGENT_TEST_AP1)) )
        {
            apNum=1;
        }
        if( 0==memcmp(pAplist->ApList[i].ssid,GAGENT_TEST_AP2,strlen(GAGENT_TEST_AP2)) )
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
    return ret;
}
void  GAgent_WiFiEventTick( pgcontext pgc,uint32 dTime_s )
{
    uint16 newStatus=0;
    uint16 gagentWiFiStatus=0;

    gagentWiFiStatus = ( (pgc->rtinfo.GAgentStatus)&(LOCAL_GAGENTSTATUS_MASK) ) ;
    newStatus = GAgent_DevCheckWifiStatus( pgc->rtinfo.GAgentStatus );
    if( (gagentWiFiStatus&WIFI_MODE_AP) != (newStatus&WIFI_MODE_AP) )
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
        GAgent_SetCloudServerStatus( pgc,MQTT_STATUS_START );
        GAgent_SetWiFiStatus( pgc,WIFI_CLOUD_CONNECTED,0 );
    }
    if( (gagentWiFiStatus&WIFI_MODE_STATION) != (newStatus&WIFI_MODE_STATION) )
    {
        if( newStatus&WIFI_MODE_STATION )
        {
            //WIFI_MODE_STATION UP
            GAgent_SetWiFiStatus( pgc,WIFI_MODE_STATION,1 );
        }
        else
        {
            //WIFI_MODE_STATION DOWN
            GAgent_SetWiFiStatus( pgc,WIFI_MODE_STATION,0 );
            GAgent_SetCloudServerStatus( pgc,MQTT_STATUS_START );
            GAgent_SetWiFiStatus( pgc,WIFI_CLOUD_CONNECTED,0 );
        }
        pgc->rtinfo.waninfo.wanclient_num=0;
        pgc->ls.tcpClientNums=0;
    }
    if( (gagentWiFiStatus&WIFI_MODE_ONBOARDING) != (newStatus&WIFI_MODE_ONBOARDING) )
    {
        if( newStatus&WIFI_MODE_ONBOARDING )
        {
            //WIFI_MODE_ONBOARDING UP
            GAgent_SetWiFiStatus( pgc,WIFI_MODE_ONBOARDING,1 );
        }
        else
        {
            //WIFI_MODE_ONBOARDING DOWN
            GAgent_SetWiFiStatus( pgc,WIFI_MODE_ONBOARDING,0 );
        }
        pgc->rtinfo.waninfo.wanclient_num=0;
        pgc->ls.tcpClientNums=0;
        GAgent_SetCloudServerStatus( pgc,MQTT_STATUS_START );
        GAgent_SetWiFiStatus( pgc,WIFI_CLOUD_CONNECTED,0 );
    }
    if( (gagentWiFiStatus&WIFI_STATION_CONNECTED) != (newStatus & WIFI_STATION_CONNECTED) )
    {
        if( newStatus&WIFI_STATION_CONNECTED )
        {
            //WIFI_STATION_CONNECTED UP
            GAgent_SetWiFiStatus( pgc,WIFI_STATION_CONNECTED,1 );
        }
        else
        {
            //WIFI_STATION_CONNECTED DOWN
            GAgent_SetWiFiStatus( pgc,WIFI_STATION_CONNECTED,0 );
            GAgent_SetCloudServerStatus( pgc,MQTT_STATUS_START );
            GAgent_SetWiFiStatus( pgc,WIFI_CLOUD_CONNECTED,0 );
            GAgent_SetWiFiStatus( pgc,WIFI_CLIENT_ON,0 );
        }
        pgc->rtinfo.waninfo.wanclient_num=0;
        pgc->ls.tcpClientNums=0;
    }
    if( (gagentWiFiStatus&WIFI_CLOUD_CONNECTED) != (newStatus & WIFI_CLOUD_CONNECTED) )
    {
        if( newStatus&WIFI_CLOUD_CONNECTED )
        {
            //WIFI_CLOUD_CONNECTED UP
            GAgent_SetWiFiStatus( pgc,WIFI_CLOUD_CONNECTED,1 );
        }
        else
        {
            //WIFI_CLOUD_CONNECTED DOWN
            pgc->rtinfo.waninfo.wanclient_num=0;
            GAgent_SetCloudServerStatus( pgc,MQTT_STATUS_START );
            GAgent_SetWiFiStatus( pgc,WIFI_CLOUD_CONNECTED,0 );
        }
    }
    if( gagentWiFiStatus&WIFI_MODE_TEST )//test mode
    {
        static int8 cnt=0;
        int8 ret =0;
        NetHostList_str *aplist;
        pgc->rtinfo.testLastTimeStamp+=dTime_s;

        if( cnt>=18 && 0 == pgc->rtinfo.scanWifiFlag )
        {
            cnt=0;
            GAgent_SetWiFiStatus( pgc,WIFI_MODE_TEST,0 );
            GAgent_DRVWiFiStopScan( );
        }

        if( pgc->rtinfo.testLastTimeStamp >= 10  && 0 == pgc->rtinfo.scanWifiFlag )
        {
            cnt++;
            pgc->rtinfo.testLastTimeStamp = 0;
            GAgent_DRVWiFiStartScan();
        }

        aplist = GAgentDRVWiFiScanResult( aplist );
        ret = GAgentFindTestApHost( aplist );
        if( ret>0 )
        {
             pgc->rtinfo.scanWifiFlag = 1;
             cnt=0;
             if( 1==ret )
              pgc->rtinfo.GAgentStatus |= GAgent_DRVWiFi_StationCustomModeStart( GAGENT_TEST_AP1,GAGENT_TEST_AP_PASS,pgc->rtinfo.GAgentStatus );
             else
              pgc->rtinfo.GAgentStatus |= GAgent_DRVWiFi_StationCustomModeStart( GAGENT_TEST_AP2,GAGENT_TEST_AP_PASS,pgc->rtinfo.GAgentStatus );
        }
    }
    GAgent_LocalSendGAgentstatus(pgc,dTime_s);
    GAgentSetLedStatus( gagentWiFiStatus );
    return ;
}
