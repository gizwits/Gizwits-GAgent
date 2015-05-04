#include "gagent.h"
#include "cloud.h"
#define SSID "UPGRADE-AP"
#define KEY  "18018888"
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
void  GAgent_WiFiEventTick( pgcontext pgc,uint32 dTime_s )
{
    /*
    #define WIFI_MODE_AP                  (1<<0)
    #define WIFI_MODE_STATION             (1<<1)
    #define WIFI_MODE_ONBOARDING          (1<<2)
    #define WIFI_MODE_BINDING             (1<<3)
    #define WIFI_STATION_CONNECTED        (1<<4)
    #define WIFI_CLOUD_CONNECTED          (1<<5)
    #define WIFI_LEVEL                    (7<<8)
    #define WIFI_CLIENT_ON                (1<<11)
    #define WIFI_MODE_TEST                (1<<12)
    */

    uint16 newStatus=0;
    uint16 gagentWiFiStatus=0;

    gagentWiFiStatus = ( (pgc->rtinfo.GAgentStatus)&(0x03FF) ) ;
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
    GAgent_LocalSendGAgentstatus(pgc,dTime_s);
    return ;
}
